#include "support/folia_test.hpp"
import folia.math.output.common;
import folia.math.output.svg.render_session;
import folia.math.util.context_property_bindings;
import folia.math.util.map_bindings;
import folia.math.util.option_functions;
import folia.math.util.runtime_modules;

namespace {
using namespace boost::ut;
namespace opts=folia::math::options;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

struct FactoryMethodFixture {
    opts::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::ContextPropertyBindings properties{context,functions};
    folia::math::util::RuntimeModules modules{context};
    std::shared_ptr<opts::MapContextBindings> maps=modules.getOrCreate<opts::MapContextBindings>(
        u"ECMAScript/Map",[&]{return std::make_shared<opts::MapContextBindings>(context);});
    common::CommonFactoryContextBindings bindings{context};
    std::shared_ptr<std::atomic_bool> active=std::make_shared<std::atomic_bool>(true);
    opts::ObjectRef constructor;
    opts::ObjectRef prototype;

    FactoryMethodFixture(){
        bindings.bindFactoryMethods(modules,properties,*maps,
            [](opts::ObjectRef)->folia::math::mml::MmlNode&{
                throw opts::Exception("TypeError",u"unexpected node resolution");
            },
            [](folia::math::mml::MmlNode&)->opts::ObjectRef{
                throw opts::Exception("TypeError",u"unexpected node identity");
            },active);
        constructor=bindings.registerConstructor();
        prototype=std::get<opts::ObjectRef>(context.get(
            opts::Value(constructor),opts::Key(u"prototype")).data);
    }
};

// Insert this helper and suite inside the existing anonymous namespace,
// after `FactoryMethodFixture` and before the existing test suite. No new
// imports are required by this fragment.
struct LayoutMapRetirementItem final:folia::math::core::AbstractMathItem {
    LayoutMapRetirementItem() : AbstractMathItem(folia::math::Property(u"x"),
        folia::math::core::MathInputJaxReference{folia::math::Missing{}}) {}
};

suite common_factory_layout_map_retirement=[] {
"private_renderer_retires_only_the_closed_jobs_exact_wrapper_map"_test=[] {
    opts::Context context;
    opts::MapContextBindings maps{context};
    folia::math::OptionFunctionBindings functions{context};
    folia::math::ContextPropertyBindings properties{context,functions};
    folia::math::util::RuntimeModules modules{context};
    common::CommonFactoryContextBindings bindings{context};
    auto active=std::make_shared<std::atomic_bool>(true);
    bindings.bindFactoryMethods(modules,properties,maps,
        [](opts::ObjectRef)->folia::math::mml::MmlNode&{
            throw opts::Exception("TypeError",u"unexpected node resolution");
        },
        [&](folia::math::mml::MmlNode&)->opts::ObjectRef{return context.object();},active);

    LayoutMapRetirementItem item;
    auto makeJob=[&]{
        auto job=std::make_shared<common::LayoutJob>(item,folia::math::core::MathDocumentReference{folia::math::Missing{}},folia::math::DOMNodeRef{});
        job->nodeMap->initialize(context,maps,
            [&](folia::math::mml::MmlNode&)->opts::ObjectRef{return context.object();},
            [&](common::CommonWrapper&)->opts::ObjectRef{return context.object();},
            [&](opts::ObjectRef)->common::CommonWrapper&{
                throw opts::Exception("TypeError",u"unexpected wrapper resolution");
            });
        return job;
    };
    const auto baseline=maps.canonicalStorageCountForDiagnostics();
    auto first=makeJob();
    auto second=makeJob();
    const auto firstObject=first->nodeMap->contextObject();
    const auto secondObject=second->nodeMap->contextObject();
    auto firstStorage=first->nodeMap->storage();
    const std::weak_ptr<opts::MapStorage> firstWeak=firstStorage;
    firstStorage.reset();
    expect(maps.canonicalStorageCountForDiagnostics()==baseline+2u);
    auto wrongStorage=maps.createStorage();
    context.setPayload(firstObject,wrongStorage);
    expect(throws<opts::Exception>([&]{(void)bindings.retireRendererLayoutJob(first);}));
    expect(context.kind(firstObject)==opts::ObjectKind::instance);
    expect(maps.canonicalStorageCountForDiagnostics()==baseline+2u)
        << "a wrong payload leaves the canonical Map registered for retry";
    context.setPayload(firstObject,first->nodeMap->storage());

    expect(bindings.retireRendererLayoutJob(first)==0u);
    expect(throws<opts::Exception>([&]{(void)context.kind(firstObject);}));
    expect(context.kind(secondObject)==opts::ObjectKind::instance);
    expect(maps.canonicalStorageCountForDiagnostics()==baseline+1u);
    first.reset();
    expect(firstWeak.expired());
    expect(bindings.retireRendererLayoutJob(second)==0u);
    expect(throws<opts::Exception>([&]{(void)context.kind(secondObject);}));
    second.reset();
    expect(maps.canonicalStorageCountForDiagnostics()==baseline);
};
};
// Insert this suite after `common_factory_layout_map_retirement` in the same
// anonymous namespace of tests/math/common_factory_context_methods.cpp.
suite common_factory_layout_job_detachment_retry=[] {
"private_renderer_detaches_source_node_map_before_retiring_active_or_completed_job"_test=[] {
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.warn=[](const std::any&) {};
    svg::NativeRenderSession session{std::move(host)};
    auto& context=session.context();
    auto& maps=session.services().mapBindings();
    auto jax=session.construct();
    opts::Value published=opts::Undefined{};
    bool failClear=false;
    jax->bindNodeMap(
        [&]{return std::make_shared<common::WrapperNodeMap>(context,maps,
            [&](folia::math::mml::MmlNode&)->opts::ObjectRef{return context.object();},
            [&](common::CommonWrapper&)->opts::ObjectRef{return context.object();},
            [&](opts::ObjectRef)->common::CommonWrapper&{
                throw opts::Exception("TypeError",u"unexpected wrapper resolution");
            });},
        [&](opts::Value value){
            if(failClear&&std::holds_alternative<opts::Null>(value.data))
                throw opts::Exception("TypeError",u"injected nodeMap detach failure");
            published=std::move(value);
        });

    LayoutMapRetirementItem item;
    const auto startJob=[&]{return jax->beginLayoutJob(item,
        folia::math::core::MathDocumentReference{folia::math::Missing{}},
        folia::math::DOMNodeRef{});};
    const auto checkPublished=[&]{
        const auto* object=std::get_if<opts::ObjectRef>(&published.data);
        expect(object!=nullptr);
        return object?*object:opts::ObjectRef{};
    };

    // Active-job path: failed detach leaves both the source Map and active
    // LayoutJob intact; retry returns that same job before Map retirement.
    auto activeJob=startJob();
    const auto activeMap=checkPublished();
    const auto baseline=maps.canonicalStorageCountForDiagnostics();
    failClear=true;
    expect(throws<opts::Exception>([&]{(void)jax->takeCurrentLayoutJobForPrivateRendererRetirement();}));
    expect(jax->activeLayoutJob()==activeJob);
    expect(context.kind(activeMap)==opts::ObjectKind::instance);
    expect(checkPublished()==activeMap);
    failClear=false;
    auto takenActive=jax->takeCurrentLayoutJobForPrivateRendererRetirement();
    expect(takenActive==activeJob);
    expect(std::holds_alternative<opts::Null>(published.data));
    expect(context.kind(activeMap)==opts::ObjectKind::instance)
        << "the Source Map remains live until the caller retires the returned job";
    (void)session.services().commonFactories().retireRendererLayoutJob(takenActive);
    expect(throws<opts::Exception>([&]{(void)context.kind(activeMap);}));
    takenActive.reset();activeJob.reset();
    expect(maps.canonicalStorageCountForDiagnostics()==baseline-1u);

    // Completed-job path: finish moves ownership to completedJob_ before its
    // publication callback; a failed take must preserve that weak-token path.
    auto completedJob=startJob();
    const auto completedMap=checkPublished();
    failClear=true;
    expect(throws<opts::Exception>([&]{jax->finishLayoutJob();}));
    expect(throws<opts::Exception>([&]{(void)jax->takeCurrentLayoutJobForPrivateRendererRetirement();}));
    expect(checkPublished()==completedMap);
    expect(context.kind(completedMap)==opts::ObjectKind::instance);
    failClear=false;
    auto takenCompleted=jax->takeCurrentLayoutJobForPrivateRendererRetirement();
    expect(takenCompleted==completedJob);
    expect(std::holds_alternative<opts::Null>(published.data));
    (void)session.services().commonFactories().retireRendererLayoutJob(takenCompleted);
    expect(throws<opts::Exception>([&]{(void)context.kind(completedMap);}));
};
};

suite common_factory_context_methods=[] {
    "native_render_session_initializes_generic_factory_before_common_constructors"_test=[] {
        common::FontLoadHost host;
        host.asyncIsSynchronous=[] {return true;};
        host.warn=[](const std::any&) {};
        svg::NativeRenderSession session(std::move(host));
        auto& context=session.context();
        auto& services=session.services();
        const auto commonConstructor=services.commonFactories().registerConstructor();
        const auto commonPrototype=std::get<opts::ObjectRef>(context.get(
            opts::Value(commonConstructor),opts::Key(u"prototype")).data);
        const auto wrapperConstructor=context.prototype(commonConstructor);
        const auto wrapperPrototype=context.prototype(commonPrototype);
        expect(wrapperConstructor.has_value());
        expect(wrapperPrototype.has_value());
        if(!wrapperConstructor||!wrapperPrototype)return;
        const auto abstractWrapperPrototypeValue=context.get(
            opts::Value(*wrapperConstructor),opts::Key(u"prototype"));
        const auto* abstractWrapperPrototype=std::get_if<opts::ObjectRef>(
            &abstractWrapperPrototypeValue.data);
        expect(abstractWrapperPrototype!=nullptr);
        if(!abstractWrapperPrototype)return;
        expect(context.get(opts::Value(*wrapperConstructor),opts::Key(u"name"))==
            opts::Value(u"AbstractWrapperFactory"));
        expect(*wrapperPrototype==*abstractWrapperPrototype);
        const auto abstractFactoryPrototype=context.prototype(*abstractWrapperPrototype);
        const auto abstractFactoryConstructor=context.prototype(*wrapperConstructor);
        expect(abstractFactoryPrototype.has_value());
        expect(abstractFactoryConstructor.has_value());
        if(!abstractFactoryPrototype||!abstractFactoryConstructor)return;
        auto& parser=services.parser();
        expect(*abstractFactoryPrototype==parser.mmlFactoryContextMethods.basePrototype());
        const auto mmlBaseConstructor=context.prototype(parser.mmlFactoryContextMethods.constructor());
        expect(mmlBaseConstructor.has_value());
        if(!mmlBaseConstructor)return;
        expect(*abstractFactoryConstructor==*mmlBaseConstructor);
        expect(*abstractFactoryConstructor==parser.factoryContextBaseModule->abstractFactoryConstructor());
    };

    "common_factory_wrap_inherits_and_calls_current_create_with_source_arguments"_test=[] {
        FactoryMethodFixture f;
        const auto wrapperPrototype=f.context.prototype(f.prototype);
        expect(wrapperPrototype.has_value());
        if(!wrapperPrototype)return;
        const auto abstractFactoryPrototype=f.context.prototype(*wrapperPrototype);
        expect(abstractFactoryPrototype.has_value());
        if(!abstractFactoryPrototype)return;
        const auto abstractWrapperCtor=f.context.get(opts::Value(*wrapperPrototype),opts::Key(u"constructor"));
        const auto abstractFactoryCtor=f.context.get(opts::Value(*abstractFactoryPrototype),opts::Key(u"constructor"));
        expect(f.context.get(abstractWrapperCtor,opts::Key(u"name"))==opts::Value(u"AbstractWrapperFactory"));
        expect(f.context.get(abstractWrapperCtor,opts::Key(u"length"))==opts::Value(0));
        expect(f.context.get(abstractFactoryCtor,opts::Key(u"name"))==opts::Value(u"AbstractFactory"));
        expect(f.context.get(abstractFactoryCtor,opts::Key(u"length"))==opts::Value(1));
        bool abstractConstructionSupported=false;
        try{
            const auto abstractInstance=f.context.construct(std::get<opts::ObjectRef>(abstractFactoryCtor.data));
            const auto own=f.context.ownKeys(opts::Value(abstractInstance));
            abstractConstructionSupported=own.size()==3&&own[0]==opts::Key(u"defaultKind")&&
                own[1]==opts::Key(u"nodeMap")&&own[2]==opts::Key(u"node");
        }catch(const opts::Exception&){ }
        expect(abstractConstructionSupported);
        const auto wrap=f.context.get(opts::Value(f.prototype),opts::Key(u"wrap"));
        const auto create=f.context.get(opts::Value(f.prototype),opts::Key(u"create"));
        expect(f.context.kind(std::get<opts::ObjectRef>(wrap.data))==opts::ObjectKind::function);
        expect(f.context.kind(std::get<opts::ObjectRef>(create.data))==opts::ObjectKind::function);
        expect(f.context.get(wrap,opts::Key(u"name"))==opts::Value(u""));
        expect(f.context.get(wrap,opts::Key(u"length"))==opts::Value(1));
        expect(f.context.get(create,opts::Key(u"name"))==opts::Value(u""));
        expect(f.context.get(create,opts::Key(u"length"))==opts::Value(1));

        auto receiver=f.context.object();
        f.context.setPrototype(receiver,f.prototype);
        auto node=f.context.object();
        f.context.set(node,opts::Key(u"kind"),opts::Value(u"math"));
        const auto extra=f.context.object();
        bool called=false;
        std::vector<opts::Value> seen;
        opts::Value seenReceiver;
        const auto replacement=f.context.function(u"create",
            [&](opts::Context&,opts::Value thisValue,std::span<const opts::Value> args){
                called=true;seen.assign(args.begin(),args.end());seenReceiver=std::move(thisValue);
                return opts::Value(extra);
            });
        f.context.set(receiver,opts::Key(u"create"),opts::Value(replacement));
        const std::array<opts::Value,2> args{opts::Value(node),opts::Value(extra)};
        const auto result=f.context.call(std::get<opts::ObjectRef>(wrap.data),
            opts::Value(receiver),args);
        expect(called);
        expect(result==opts::Value(extra));
        expect(seenReceiver==opts::Value(receiver));
        expect(seen.size()==3);
        if(seen.size()==3){
            expect(seen[0]==opts::Value(u"math"));
            expect(seen[1]==opts::Value(node));
            expect(seen[2]==opts::Value(extra));
        }
    };

    "common_factory_wrap_preserves_source_property_and_apply_evaluation_order"_test=[] {
        FactoryMethodFixture f;
        const auto wrap=f.context.get(opts::Value(f.prototype),opts::Key(u"wrap"));
        auto receiver=f.context.object();
        f.context.setPrototype(receiver,f.prototype);
        auto node=f.context.object();
        std::vector<std::u16string> events;
        const auto kindGetter=f.context.function(u"get kind",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                events.push_back(u"kind");return opts::Value(u"mi");
            });
        f.context.define(node,opts::Key(u"kind"),opts::Descriptor::access(kindGetter,std::nullopt));
        const auto create=f.context.function(u"create",
            [](opts::Context&,opts::Value,std::span<const opts::Value>){
                return opts::Value(opts::Undefined{});
            });
        const auto apply=f.context.function(u"apply",
            [&](opts::Context& cx,opts::Value thisValue,std::span<const opts::Value> args){
                events.push_back(u"call apply");
                expect(thisValue==opts::Value(create));
                expect(args.size()==2);
                if(args.size()==2){
                    expect(args[0]==opts::Value(receiver));
                    const auto* array=std::get_if<opts::ObjectRef>(&args[1].data);
                    expect(array!=nullptr);
                    if(array){
                        const auto values=cx.arrayValues(*array);
                        expect(values.size()==3);
                        if(values.size()==3){
                            expect(values[0]==opts::Value(u"mi"));
                            expect(values[1]==opts::Value(node));
                            expect(values[2]==opts::Value(u"extra"));
                        }
                    }
                }
                return opts::Value(u"applied");
            });
        const auto applyGetter=f.context.function(u"get apply",
            [&](opts::Context&,opts::Value thisValue,std::span<const opts::Value>){
                events.push_back(u"apply getter");
                expect(thisValue==opts::Value(create));
                return opts::Value(apply);
            });
        f.context.define(create,opts::Key(u"apply"),
            opts::Descriptor::access(applyGetter,std::nullopt));
        const auto createGetter=f.context.function(u"get create",
            [&](opts::Context&,opts::Value thisValue,std::span<const opts::Value>){
                events.push_back(u"create getter");
                expect(thisValue==opts::Value(receiver));
                return opts::Value(create);
            });
        f.context.define(receiver,opts::Key(u"create"),
            opts::Descriptor::access(createGetter,std::nullopt));
        const std::array<opts::Value,2> args{opts::Value(node),opts::Value(u"extra")};
        const auto result=f.context.call(std::get<opts::ObjectRef>(wrap.data),
            opts::Value(receiver),args);
        expect(result==opts::Value(u"applied"));
        const bool sourceOrder=events==std::vector<std::u16string>{u"create getter",u"apply getter",u"kind",u"call apply"};
        expect(sourceOrder);
    };

    "function_apply_reads_array_like_slots_and_preserves_holes"_test=[] {
        opts::Context context;
        auto target=context.function(u"target",
            [](opts::Context&,opts::Value receiver,std::span<const opts::Value> args){
                expect(receiver==opts::Value(u"receiver"));
                expect(args.size()==3);
                if(args.size()==3){
                    expect(args[0]==opts::Value(11));
                    expect(std::holds_alternative<opts::Undefined>(args[1].data));
                    expect(args[2]==opts::Value(33));
                }
                return opts::Value(true);
            });
        const auto apply=context.get(opts::Value(target),opts::Key(u"apply"));
        const auto sparse=context.array(3);
        context.set(opts::Value(sparse),opts::Key(u"0"),opts::Value(11));
        context.set(opts::Value(sparse),opts::Key(u"2"),opts::Value(33));
        const std::array<opts::Value,2> callArgs{opts::Value(u"receiver"),opts::Value(sparse)};
        const auto result=context.callMember(apply,opts::Value(target),callArgs);
        expect(result==opts::Value(true));
    };

    "function_apply_rejects_primitive_argument_lists_before_length_lookup"_test=[] {
        opts::Context context;
        const auto target=context.function(u"target",
            [](opts::Context&,opts::Value,std::span<const opts::Value>){return opts::Value(true);});
        const auto apply=context.get(opts::Value(target),opts::Key(u"apply"));
        auto arrayLike=context.object();
        bool lengthRead=false;
        const auto lengthGetter=context.function(u"get length",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                lengthRead=true;return opts::Value(0);
            });
        context.define(arrayLike,opts::Key(u"length"),opts::Descriptor::access(lengthGetter,std::nullopt));
        const std::array<opts::Value,2> validArgs{opts::Value(opts::Undefined{}),opts::Value(arrayLike)};
        bool called=false;
        try{(void)context.callMember(apply,opts::Value(context.object()),validArgs);}
        catch(const opts::Exception& error){called=error.name=="TypeError";}
        expect(called);
        expect(!lengthRead);

        const std::array<opts::Value,2> primitiveArgs{opts::Value(opts::Undefined{}),opts::Value(u"abc")};
        bool primitiveRejected=false;
        try{(void)context.callMember(apply,opts::Value(target),primitiveArgs);}
        catch(const opts::Exception& error){primitiveRejected=error.name=="TypeError";}
        expect(primitiveRejected);
    };

    "common_factory_source_methods_reject_released_runtime_before_native_access"_test=[] {
        FactoryMethodFixture f;
        const auto wrap=f.context.get(opts::Value(f.prototype),opts::Key(u"wrap"));
        f.active->store(false,std::memory_order_release);
        bool threw=false;
        try{
            (void)f.context.call(std::get<opts::ObjectRef>(wrap.data),
                opts::Value(f.context.object()),{});
        }catch(const opts::Exception& error){threw=error.name=="TypeError";}
        expect(threw);
    };
    "common_exported_base_uses_context_instanceof_for_generic_classes"_test=[] {
        FactoryMethodFixture f;
        const auto wrapperProto=f.context.prototype(f.prototype);
        const auto baseProto=wrapperProto?f.context.prototype(*wrapperProto):std::nullopt;
        expect(baseProto.has_value());
        if(!baseProto)return;
        const auto baseCtorValue=f.context.get(opts::Value(*baseProto),opts::Key(u"constructor"));
        const auto* baseCtor=std::get_if<opts::ObjectRef>(&baseCtorValue.data);
        expect(baseCtor!=nullptr);
        if(!baseCtor)return;

        opts::ObjectRef productCtor{};
        productCtor=f.context.nativeConstructor(u"Product",
            [&productCtor](opts::Context& cx,std::span<const opts::Value>)->opts::ObjectRef {
                return cx.instance(productCtor);
            });
        const auto nodes=f.context.object();
        f.context.set(opts::Value(nodes),opts::Key(u"product"),opts::Value(productCtor));
        f.context.set(opts::Value(nodes),opts::Key(u"invalid"),opts::Value(f.context.object()));
        const auto receiver=f.context.objectWithPrototype(*baseProto);
        const auto initialized=f.context.call(*baseCtor,opts::Value(receiver),
            std::array<opts::Value,1>{opts::Value(nodes)});
        expect(std::holds_alternative<opts::Undefined>(initialized.data));

        const auto product=f.context.construct(productCtor);
        const auto nodeIsKind=f.context.get(opts::Value(receiver),opts::Key(u"nodeIsKind"));
        const std::array<opts::Value,2> productArgs{opts::Value(product),opts::Value(u"product")};
        const std::array<opts::Value,2> plainArgs{opts::Value(f.context.object()),opts::Value(u"product")};
        const std::array<opts::Value,2> invalidArgs{opts::Value(product),opts::Value(u"invalid")};
        const auto productResult=f.context.callMember(nodeIsKind,opts::Value(receiver),productArgs,u"nodeIsKind");
        const auto plainResult=f.context.callMember(nodeIsKind,opts::Value(receiver),plainArgs,u"nodeIsKind");
        bool invalidRhsTypeError=false;
        try{(void)f.context.callMember(nodeIsKind,opts::Value(receiver),invalidArgs,u"nodeIsKind");}
        catch(const opts::Exception& error){
            invalidRhsTypeError=error.name=="TypeError"&&
                error.message==u"Right-hand side of 'instanceof' is not callable";
        }
        expect(productResult==opts::Value(true));
        expect(plainResult==opts::Value(false));
        expect(invalidRhsTypeError);
    };
};
}
