#include "support/folia_test.hpp"
import folia.math.core.tree.factory_context_bindings;
import folia.math.util.map_bindings;
import folia.math.util.option_functions;
import folia.math.util.native_identities;
import folia.math.util.context_property_bindings;
import folia.math.util.option_types;
import folia.math.util.runtime_modules;
import folia.math.input.tex.parser;
import folia.math.util.regexp_binding;
import folia.math.util.unicode_normalization;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;

namespace {
using namespace boost::ut;
namespace o=folia::math::options;
namespace t=folia::math::tree;
struct ProbeProduct {};
struct ProbeNode { std::string kind()const{return "probe";} };

struct Fixture {
    o::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::ContextPropertyBindings properties{context,functions};
    folia::math::util::RuntimeModules modules{context};
    std::shared_ptr<o::MapContextBindings> maps=modules.getOrCreate<o::MapContextBindings>(
        u"ECMAScript/Map",[&]{return std::make_shared<o::MapContextBindings>(context);});
    std::shared_ptr<t::FactoryContextBaseModule> sharedBase=modules.getOrCreate<t::FactoryContextBaseModule>(
        u"core/Tree/Factory",[&]{return std::make_shared<t::FactoryContextBaseModule>(modules);});
    std::shared_ptr<std::atomic_bool> lease=std::make_shared<std::atomic_bool>(true);
    using Owner=std::shared_ptr<t::AbstractFactory<ProbeProduct>>;
    std::vector<std::pair<o::ObjectRef,Owner>> owners;
    t::FactoryContextBindings<ProbeProduct,ProbeNode> bindings;

    static typename t::FactoryContextBindings<ProbeProduct,ProbeNode>::Hooks hooks(Fixture& self) {
        using Hooks=typename t::FactoryContextBindings<ProbeProduct,ProbeNode>::Hooks;
        Hooks result;
        result.executionLease=self.lease;
        result.resolveBase=[&self](o::ObjectRef object)->std::optional<Owner>{
            const auto& payload=std::as_const(self.context).payload(object);
            if(!payload.has_value())return {};
            const auto* native=std::any_cast<folia::math::NativeBindingPayload<t::AbstractFactory<ProbeProduct>*>>(&payload);
            if(!native||!native->identity||!native->lifetime)
                throw o::Exception("TypeError",u"incompatible native receiver payload");
            return std::static_pointer_cast<t::AbstractFactory<ProbeProduct>>(native->lifetime);
        };
        result.bindBase=[&self](const Owner& owner,o::ObjectRef object){
            if(std::as_const(self.context).payload(object).has_value())
                throw o::Exception("TypeError",u"receiver already has a native owner");
            self.context.setPayload(object,folia::math::NativeBindingPayload<t::AbstractFactory<ProbeProduct>*>{
                owner.get(),std::static_pointer_cast<void>(owner)});
            self.owners.emplace_back(object,owner);
        };
        result.allocateBase=[](o::ObjectRef){return std::make_shared<t::AbstractFactory<ProbeProduct>>();};
        result.allocateWrapper=[](o::ObjectRef){
            return std::static_pointer_cast<t::AbstractFactory<ProbeProduct>>(
                std::make_shared<t::AbstractWrapperFactory<ProbeProduct,ProbeNode>>());
        };
        result.initializeFields=[&self](const Owner& owner,o::ObjectRef receiver){
            // Test adapter for the actual owner field seam: initialize the
            // Source fields once and install the same hooks used by the real
            // AbstractFactory::createSourceValue path.
            self.context.set(o::Value(receiver),o::Key(u"defaultKind"),o::Value(u"unknown"));
            const auto map=self.maps->create();
            self.context.set(o::Value(receiver),o::Key(u"nodeMap"),o::Value(map));
            const auto node=self.context.object();
            self.context.set(o::Value(receiver),o::Key(u"node"),o::Value(node));
            using CreateHooks=t::AbstractFactory<ProbeProduct>::SourceCreateHooks;
            CreateHooks create;
            create.readNode=[&self,receiver]{
                return self.context.get(o::Value(receiver),o::Key(u"node"));
            };
            create.readDefaultKind=[&self,receiver]{
                return self.context.get(o::Value(receiver),o::Key(u"defaultKind"));
            };
            create.toKey=[](const o::Value& value){
                if(const auto* symbol=std::get_if<o::SymbolRef>(&value.data))return o::Key(*symbol);
                if(const auto* text=std::get_if<std::u16string>(&value.data))return o::Key(*text);
                throw o::Exception("TypeError",u"test factory key must be a string or Symbol");
            };
            create.lookup=[&self](const o::Value& target,const o::Key& key){
                return self.context.get(target,key);
            };
            create.readApply=[&self](const o::Value& selected){
                return self.context.get(selected,o::Key(u"apply"));
            };
            create.argumentValue=[](const std::any& value){return std::any_cast<o::Value>(value);};
            create.invoke=[&self](const o::Value& selected,const o::Value& apply,
                                  std::span<const o::Value> args){
                const auto array=self.context.array(static_cast<std::uint32_t>(args.size()));
                for(std::size_t index=0;index<args.size();++index){
                    const auto ascii=std::to_string(index);
                    self.context.set(o::Value(array),o::Key(std::u16string(ascii.begin(),ascii.end())),args[index]);
                }
                const std::array<o::Value,2> forwarded{o::Value(o::Undefined{}),o::Value(array)};
                return self.context.callMember(apply,selected,forwarded,u"create.apply");
            };
            create.resolveProduct=[](const o::Value&)->ProbeProduct*{return nullptr;};
            owner->sourceCreateHooks(std::move(create));
        };
        result.instanceOf=[](o::Context&,const o::Value&,const o::Value&){return false;};
        return result;
    }
    Fixture():bindings(context,sharedBase,hooks(*this)){}
};

suite factoryContextBindingsCandidate=[] {
    "shared_factory_module_remains_usable_after_first_parser_runtime_dies"_test=[] {
        o::Context context;
        folia::math::OptionFunctionBindings functions(context);
        folia::math::regexp_host::RegExpBindings regexp(context);
        folia::math::Unicode16Normalization unicode;
        folia::math::core::InputJaxModule input(context);
        folia::math::core::FindMathModule findMath(context);
        folia::math::tex::FindTeXModule findTeX(context,findMath,regexp);
        folia::math::util::RuntimeModules modules(context);
        std::shared_ptr<t::FactoryContextBaseModule> shared;
        {
            folia::math::tex::ParserRuntime parser(context,functions,regexp,unicode,input,findTeX,&modules);
            shared=parser.factoryContextBaseModule;
        }
        const auto constructor=shared->abstractFactoryConstructor();
        const auto instance=context.construct(constructor);
        const auto prototype=shared->abstractFactoryPrototype();
        const auto getKinds=context.get(o::Value(prototype),o::Key(u"getKinds"));
        const auto kinds=context.callMember(getKinds,o::Value(instance));
        const auto* kindsObject=std::get_if<o::ObjectRef>(&kinds.data);
        expect(kindsObject!=nullptr);
        if(kindsObject)expect(context.arrayLength(*kindsObject)==0u);
    };

    "source_base_constructor_fields_and_wrapper_adapter_owner"_test=[] {
        Fixture f;
        const auto base=f.bindings.abstractFactoryConstructor();
        const auto basePrototype=f.bindings.abstractFactoryPrototype();
        const auto wrapper=f.bindings.abstractWrapperFactoryConstructor();
        const auto wrapperPrototype=f.bindings.abstractWrapperFactoryPrototype();
        const auto baseLength=f.context.get(o::Value(base),o::Key(u"length"));
        const auto wrapperLength=f.context.get(o::Value(wrapper),o::Key(u"length"));
        const bool metadata=baseLength==o::Value(1.0)&&wrapperLength==o::Value(0.0)&&
            f.context.get(o::Value(base),o::Key(u"name"))==o::Value(u"AbstractFactory")&&
            f.context.get(o::Value(wrapper),o::Key(u"name"))==o::Value(u"AbstractWrapperFactory");
        const bool inheritance=f.context.prototype(wrapper)==std::optional<o::ObjectRef>(base)&&
            f.context.prototype(wrapperPrototype)==std::optional<o::ObjectRef>(basePrototype);
        const auto methodMatches=[&](o::ObjectRef prototype,std::u16string_view name,double length){
            const auto descriptor=f.context.descriptor(o::Value(prototype),o::Key(std::u16string(name)));
            if(!descriptor||descriptor->accessor||!descriptor->writable||
               !descriptor->enumerable||!descriptor->configurable)return false;
            const auto* function=std::get_if<o::ObjectRef>(&descriptor->value.data);
            if(!function||f.context.kind(*function)!=o::ObjectKind::function)return false;
            const auto ownName=f.context.descriptor(o::Value(*function),o::Key(u"name"));
            const auto ownLength=f.context.descriptor(o::Value(*function),o::Key(u"length"));
            return ownName&&ownLength&&!ownName->enumerable&&!ownLength->enumerable&&
                ownName->configurable&&ownLength->configurable&&
                f.context.get(o::Value(*function),o::Key(u"name"))==o::Value(u"")&&
                f.context.get(o::Value(*function),o::Key(u"length"))==o::Value(length);
        };
        const bool methods=methodMatches(basePrototype,u"create",1)&&
            methodMatches(basePrototype,u"setNodeClass",2)&&
            methodMatches(basePrototype,u"getNodeClass",1)&&
            methodMatches(basePrototype,u"deleteNodeClass",1)&&
            methodMatches(basePrototype,u"nodeIsKind",2)&&
            methodMatches(basePrototype,u"getKinds",0)&&
            methodMatches(wrapperPrototype,u"wrap",1);
        const auto defaults=f.context.get(o::Value(base),o::Key(u"defaultNodes"));
        const auto defaultDescriptor=f.context.descriptor(o::Value(base),o::Key(u"defaultNodes"));
        const bool actualDefault=std::holds_alternative<o::ObjectRef>(defaults.data)&&
            f.context.get(o::Value(wrapper),o::Key(u"defaultNodes"))==defaults&&defaultDescriptor&&
            !defaultDescriptor->accessor&&defaultDescriptor->writable&&defaultDescriptor->enumerable&&
            defaultDescriptor->configurable;

        const auto empty=f.context.object();
        const std::array<o::Value,1> args{o::Value(empty)};
        const auto instance=f.context.construct(base,args);
        const auto keys=f.context.ownKeys(o::Value(instance));
        const bool ownFields=keys.size()==3&&keys[0]==o::Key(u"defaultKind")&&
            keys[1]==o::Key(u"nodeMap")&&keys[2]==o::Key(u"node");
        const bool sourceFieldValues=f.context.get(o::Value(instance),o::Key(u"defaultKind"))==o::Value(u"unknown")&&
            std::holds_alternative<o::ObjectRef>(f.context.get(o::Value(instance),o::Key(u"nodeMap")).data)&&
            std::holds_alternative<o::ObjectRef>(f.context.get(o::Value(instance),o::Key(u"node")).data);
        const bool baseHasNoProductPayload=!std::as_const(f.context).payload(instance).has_value();

        const auto existing=f.context.objectWithPrototype(basePrototype);
        f.context.set(o::Value(existing),o::Key(u"constructor"),o::Value(base));
        const auto constructorBeforeCall=f.context.get(o::Value(existing),o::Key(u"constructor"));
        const auto callResult=f.context.call(base,o::Value(existing),args);
        const auto callFields=f.context.ownKeys(o::Value(existing));
        const bool callInitializesSourceFields=callFields.size()==4&&
            callFields[0]==o::Key(u"constructor")&&callFields[1]==o::Key(u"defaultKind")&&
            callFields[2]==o::Key(u"nodeMap")&&callFields[3]==o::Key(u"node");
        const bool sourceCallPreservesConstructor=constructorBeforeCall==o::Value(base)&&
            f.context.get(o::Value(existing),o::Key(u"constructor"))==constructorBeforeCall;

        const auto wrapperCall=f.context.call(wrapper,o::Value(existing),args);
        const auto wrapperOwner=f.bindings.resolveBase(existing);
        expect(metadata)<<"factory-constructor-metadata";
        expect(inheritance)<<"factory-constructor-and-prototype-inheritance";
        expect(methods)<<"factory-prototype-method-descriptors";
        expect(actualDefault)<<"factory-inherited-defaultNodes";
        expect(ownFields&&sourceFieldValues)<<"factory-source-construction-initializes-source-fields";
        expect(baseHasNoProductPayload)<<"factory-source-construction-has-no-template-product-owner";
        expect(callInitializesSourceFields)<<"factory-source-call-initializes-the-existing-receiver";
        expect(sourceCallPreservesConstructor)<<"factory-source-call-preserves-own-constructor";
        expect(std::holds_alternative<o::Undefined>(callResult.data))<<"factory-base-call-return";
        expect(std::holds_alternative<o::ObjectRef>(wrapperCall.data)&&std::get<o::ObjectRef>(wrapperCall.data)==existing)<<"factory-wrapper-call-returns-this";
        expect(static_cast<bool>(wrapperOwner))<<"factory-wrapper-adapter-allocates-its-product-owner";
    };

    "explicit_nodes_do_not_read_constructor_getter_for_base_or_wrapper_call"_test=[] {
        Fixture f;
        const auto base=f.bindings.abstractFactoryConstructor();
        const auto wrapper=f.bindings.abstractWrapperFactoryConstructor();
        const auto nodes=f.context.object();
        const auto nodeClass=f.context.nativeConstructor(u"ExplicitClass",
            [](o::Context& cx,std::span<const o::Value>)->o::ObjectRef{return cx.object();});
        f.context.set(o::Value(nodes),o::Key(u"explicit"),o::Value(nodeClass));

        int baseReads=0,wrapperReads=0;
        const auto baseGetter=f.context.function(u"get constructor",
            [&baseReads](o::Context&,o::Value,std::span<const o::Value>)->o::Value {
                ++baseReads;
                throw o::Exception("TypeError",u"constructor getter must not run");
            });
        const auto wrapperGetter=f.context.function(u"get constructor",
            [&wrapperReads](o::Context&,o::Value,std::span<const o::Value>)->o::Value {
                ++wrapperReads;
                throw o::Exception("TypeError",u"constructor getter must not run");
            });

        const auto baseReceiver=f.context.objectWithPrototype(f.bindings.abstractFactoryPrototype());
        const auto wrapperReceiver=f.context.objectWithPrototype(f.bindings.abstractWrapperFactoryPrototype());
        f.context.define(baseReceiver,o::Key(u"constructor"),o::Descriptor::access(baseGetter,std::nullopt,true,true));
        f.context.define(wrapperReceiver,o::Key(u"constructor"),o::Descriptor::access(wrapperGetter,std::nullopt,true,true));

        const auto baseResult=f.context.call(base,o::Value(baseReceiver),std::array<o::Value,1>{o::Value(nodes)});
        const auto wrapperResult=f.context.call(wrapper,o::Value(wrapperReceiver),std::array<o::Value,1>{o::Value(nodes)});
        const auto wrapperOwner=f.bindings.resolveBase(wrapperReceiver);
        const auto baseNode=f.context.get(o::Value(baseReceiver),o::Key(u"node"));
        const auto wrapperNode=f.context.get(o::Value(wrapperReceiver),o::Key(u"node"));
        const auto baseMap=f.context.get(o::Value(baseReceiver),o::Key(u"nodeMap"));
        const auto wrapperMap=f.context.get(o::Value(wrapperReceiver),o::Key(u"nodeMap"));
        const auto baseMapGet=f.context.get(baseMap,o::Key(u"get"));
        const auto wrapperMapGet=f.context.get(wrapperMap,o::Key(u"get"));
        const std::array<o::Value,1> explicitKey{o::Value(u"explicit")};
        const auto baseMappedClass=f.context.callMember(baseMapGet,baseMap,explicitKey);
        const auto wrapperMappedClass=f.context.callMember(wrapperMapGet,wrapperMap,explicitKey);
        const auto baseCreator=f.context.get(baseNode,o::Key(u"explicit"));
        const auto wrapperCreator=f.context.get(wrapperNode,o::Key(u"explicit"));
        const auto* baseCreatorObject=std::get_if<o::ObjectRef>(&baseCreator.data);
        const auto* wrapperCreatorObject=std::get_if<o::ObjectRef>(&wrapperCreator.data);
        expect(baseReads==0)<<"factory-explicit-base-skips-constructor-getter";
        expect(wrapperReads==0)<<"factory-explicit-wrapper-skips-constructor-getter";
        expect(std::holds_alternative<o::Undefined>(baseResult.data))<<"factory-explicit-base-call-return";
        expect(std::holds_alternative<o::ObjectRef>(wrapperResult.data)&&std::get<o::ObjectRef>(wrapperResult.data)==wrapperReceiver)<<"factory-explicit-wrapper-call-return";
        expect(!std::as_const(f.context).payload(baseReceiver).has_value())<<"factory-source-base-call-has-no-template-owner";
        expect(static_cast<bool>(wrapperOwner))<<"factory-wrapper-adapter-call-has-product-owner";
        expect(baseMappedClass==o::Value(nodeClass)&&wrapperMappedClass==o::Value(nodeClass))<<"factory-explicit-class-is-in-nodeMap";
        expect(baseCreatorObject&&f.context.kind(*baseCreatorObject)==o::ObjectKind::function)<<"factory-explicit-base-node-entry-is-creator";
        expect(wrapperCreatorObject&&f.context.kind(*wrapperCreatorObject)==o::ObjectKind::function)<<"factory-explicit-wrapper-node-entry-is-creator";
    };
    "expired_factory_constructor_call_reports_type_error"_test=[] {
        Fixture f;
        const auto base=f.bindings.abstractFactoryConstructor();
        f.lease->store(false,std::memory_order_release);
        bool typeError=false;
        try{(void)f.context.call(base,o::Value(f.context.object()),{});}
        catch(const o::Exception& error){typeError=error.name=="TypeError";}
        expect(typeError);
    };

    "class_map_methods_and_wrapper_wrap_use_live_context_owners"_test=[] {
        Fixture f;
        const auto base=f.bindings.abstractFactoryConstructor();
        const auto wrapper=f.bindings.abstractWrapperFactoryConstructor();
        const auto nodeClass=f.context.nativeConstructor(u"ProbeClass",
            [](o::Context& cx,std::span<const o::Value>)->o::ObjectRef{return cx.object();});
        const auto nodes=f.context.object();
        f.context.set(o::Value(nodes),o::Key(u"probe"),o::Value(nodeClass));
        const auto factory=f.context.construct(base,
            std::array<o::Value,1>{o::Value(nodes)});
        const auto proto=f.bindings.abstractFactoryPrototype();
        const auto getNodeClass=f.context.get(o::Value(proto),o::Key(u"getNodeClass"));
        const std::array<o::Value,1> probeKey{o::Value(u"probe")};
        const auto got=f.context.callMember(getNodeClass,o::Value(factory),probeKey,u"getNodeClass");
        const auto getKinds=f.context.get(o::Value(proto),o::Key(u"getKinds"));
        const auto kindsValue=f.context.callMember(getKinds,o::Value(factory),{},u"getKinds");
        const auto kinds=std::get<o::ObjectRef>(kindsValue.data);
        const auto firstKind=f.context.get(o::Value(kinds),o::Key(u"0"));
        const auto nodeIsKind=f.context.get(o::Value(proto),o::Key(u"nodeIsKind"));
        const std::array<o::Value,2> kindArgs{o::Value(f.context.object()),o::Value(u"probe")};
        const auto isKind=f.context.callMember(nodeIsKind,o::Value(factory),kindArgs,u"nodeIsKind");
        const auto deleteNodeClass=f.context.get(o::Value(proto),o::Key(u"deleteNodeClass"));
        (void)f.context.callMember(deleteNodeClass,o::Value(factory),probeKey,u"deleteNodeClass");
        const auto afterDelete=f.context.callMember(getKinds,o::Value(factory),{},u"getKinds");
        const auto emptyKinds=std::get<o::ObjectRef>(afterDelete.data);

        const auto wrapperInstance=f.context.construct(wrapper,
            std::array<o::Value,1>{o::Value(f.context.object())});
        std::vector<o::Value> wrappedArgs;
        o::Value wrappedReceiver=o::Undefined{};
        const auto create=f.context.function(u"",[&](o::Context&,o::Value receiver,
                std::span<const o::Value> args)->o::Value{
            wrappedReceiver=std::move(receiver);
            wrappedArgs.assign(args.begin(),args.end());
            return o::Value(u"wrapped");
        });
        f.context.set(o::Value(wrapperInstance),o::Key(u"create"),o::Value(create));
        const auto wrap=f.context.get(o::Value(f.bindings.abstractWrapperFactoryPrototype()),o::Key(u"wrap"));
        const auto node=f.context.object();
        f.context.set(o::Value(node),o::Key(u"kind"),o::Value(u"leaf"));
        const std::array<o::Value,2> wrapArgs{o::Value(node),o::Value(u"extra")};
        const auto wrapResult=f.context.callMember(wrap,o::Value(wrapperInstance),wrapArgs,u"wrap");
        const bool pass=got==o::Value(nodeClass)&&firstKind==o::Value(u"probe")&&
            std::holds_alternative<bool>(isKind.data)&&!std::get<bool>(isKind.data)&&
            f.context.get(o::Value(emptyKinds),o::Key(u"length"))==o::Value(0.0)&&
            wrapResult==o::Value(u"wrapped")&&wrappedReceiver==o::Value(wrapperInstance)&&
            wrappedArgs.size()==3&&wrappedArgs[0]==o::Value(u"leaf")&&
            wrappedArgs[1]==o::Value(node)&&wrappedArgs[2]==o::Value(u"extra");
        expect(pass)<<"factory-map-methods-and-wrapper-live-dispatch";
    };

    "create_delegates_to_the_real_owner_source_selection_algorithm"_test=[] {
        Fixture f;
        const auto base=f.bindings.abstractFactoryConstructor();
        const auto instance=f.context.construct(base,
            std::array<o::Value,1>{o::Value(f.context.object())});
        const auto node=std::get<o::ObjectRef>(
            f.context.get(o::Value(instance),o::Key(u"node")).data);
        std::vector<o::Value> observed;
        o::Value observedReceiver=o::Undefined{};
        const auto creator=f.context.function(u"",[&](o::Context&,o::Value receiver,
                std::span<const o::Value> args)->o::Value{
            observedReceiver=std::move(receiver);
            observed.assign(args.begin(),args.end());
            return o::Value(u"created");
        });
        f.context.set(o::Value(node),o::Key(u"custom"),o::Value(creator));
        const auto method=f.context.get(o::Value(f.bindings.abstractFactoryPrototype()),o::Key(u"create"));
        const std::array<o::Value,3> args{o::Value(u"custom"),o::Value(5.0),o::Value(6.0)};
        const auto result=f.context.callMember(method,o::Value(instance),args,u"create");
        const bool pass=result==o::Value(u"created")&&
            std::holds_alternative<o::Undefined>(observedReceiver.data)&&observed.size()==2&&
            observed[0]==o::Value(5.0)&&observed[1]==o::Value(6.0);
        expect(pass)<<"factory-create-uses-actual-owner-selection-algorithm";
    };

    "create_reads_and_calls_selected_creator_apply_at_the_source_boundary"_test=[] {
        Fixture f;
        const auto factory=f.context.construct(f.bindings.abstractFactoryConstructor());
        const auto node=std::get<o::ObjectRef>(f.context.get(o::Value(factory),o::Key(u"node")).data);
        std::vector<std::u16string> order;
        const auto creator=f.context.object();
        const auto applyGetter=f.context.function(u"get apply",[&](o::Context&,o::Value,
            std::span<const o::Value>){order.push_back(u"apply get");return o::Value(f.context.function(u"apply",
                [&](o::Context& cx,o::Value thisValue,std::span<const o::Value> args){
                    order.push_back(u"apply call");
                    expect(thisValue==o::Value(creator));
                    expect(args.size()==2);
                    if(args.size()==2){
                        expect(std::holds_alternative<o::Undefined>(args[0].data));
                        const auto* array=std::get_if<o::ObjectRef>(&args[1].data);
                        expect(array!=nullptr);
                        if(array)expect(std::ranges::equal(cx.arrayValues(*array),std::array<o::Value,2>{o::Value(17.0),o::Value(23.0)}));
                    }
                    return o::Value(u"applied creator");
                }));});
        f.context.define(creator,o::Key(u"apply"),o::Descriptor::access(applyGetter,std::nullopt,true,true));
        f.context.set(o::Value(node),o::Key(u"custom"),o::Value(creator));
        const auto create=f.context.get(o::Value(f.bindings.abstractFactoryPrototype()),o::Key(u"create"));
        const std::array<o::Value,3> args{o::Value(u"custom"),o::Value(17.0),o::Value(23.0)};
        const auto result=f.context.callMember(create,o::Value(factory),args,u"create");
        expect(result==o::Value(u"applied creator"));
        expect(order==std::vector<std::u16string>{u"apply get",u"apply call"});
    };

    "create_fallback_rereads_node_before_default_kind"_test=[] {
        Fixture f;
        const auto factory=f.context.construct(f.bindings.abstractFactoryConstructor());
        const auto firstNode=f.context.object();
        const auto fallbackNode=f.context.object();
        const auto creator=f.context.object();
        std::vector<std::u16string> order;
        const auto applyGetter=f.context.function(u"get apply",[&](o::Context&,o::Value,
            std::span<const o::Value>){
            order.push_back(u"apply get");
            return o::Value(f.context.function(u"apply",[&](o::Context&,o::Value,
                std::span<const o::Value>){order.push_back(u"apply call");return o::Value(u"fallback result");}));
        });
        f.context.define(creator,o::Key(u"apply"),o::Descriptor::access(applyGetter,std::nullopt,true,true));
        f.context.set(o::Value(fallbackNode),o::Key(u"unknown"),o::Value(creator));
        int nodeReads=0;
        const auto nodeGetter=f.context.function(u"get node",[&](o::Context&,o::Value,
            std::span<const o::Value>){
            order.push_back(nodeReads++==0?u"node first":u"node fallback");
            return o::Value(nodeReads==1?firstNode:fallbackNode);
        });
        const auto defaultKindGetter=f.context.function(u"get defaultKind",[&](o::Context&,o::Value,
            std::span<const o::Value>){order.push_back(u"defaultKind");return o::Value(u"unknown");});
        f.context.define(factory,o::Key(u"node"),o::Descriptor::access(nodeGetter,std::nullopt,true,true));
        f.context.define(factory,o::Key(u"defaultKind"),o::Descriptor::access(defaultKindGetter,std::nullopt,true,true));
        const auto create=f.context.get(o::Value(f.bindings.abstractFactoryPrototype()),o::Key(u"create"));
        const auto result=f.context.callMember(create,o::Value(factory),std::array<o::Value,1>{o::Value(u"missing")});
        expect(result==o::Value(u"fallback result"));
        expect(order==std::vector<std::u16string>{u"node first",u"node fallback",u"defaultKind",u"apply get",u"apply call"});
    };

    "wrapper_apply_noncallable_throws_source_type_error_after_kind_get"_test=[] {
        Fixture f;
        const auto wrapper=f.context.construct(f.bindings.abstractWrapperFactoryConstructor());
        std::vector<std::u16string> order;
        const auto node=f.context.object();
        const auto kindGetter=f.context.function(u"get kind",
            [&order](o::Context&,o::Value,std::span<const o::Value>){
                order.push_back(u"kind get");return o::Value(u"leaf");
            });
        f.context.define(node,o::Key(u"kind"),o::Descriptor::access(kindGetter,std::nullopt,true,true));
        const auto createObject=f.context.object();
        const auto applyGetter=f.context.function(u"get apply",
            [&order](o::Context&,o::Value,std::span<const o::Value>){
                order.push_back(u"apply get");return o::Value(0.0);
            });
        f.context.define(createObject,o::Key(u"apply"),o::Descriptor::access(applyGetter,std::nullopt,true,true));
        const auto createGetter=f.context.function(u"get create",
            [&order,createObject](o::Context&,o::Value,std::span<const o::Value>){
                order.push_back(u"create get");return o::Value(createObject);
            });
        f.context.define(wrapper,o::Key(u"create"),o::Descriptor::access(createGetter,std::nullopt,true,true));
        const auto wrap=f.context.get(o::Value(f.bindings.abstractWrapperFactoryPrototype()),o::Key(u"wrap"));
        bool typeError=false;
        try{(void)f.context.callMember(wrap,o::Value(wrapper),std::array<o::Value,1>{o::Value(node)},u"wrap");}
        catch(const o::Exception& error){typeError=error.name=="TypeError";}
        expect(typeError);
        expect(order==std::vector<std::u16string>{u"create get",u"apply get",u"kind get"});
    };

    "source_base_call_initializes_fields_and_preserves_unrelated_payload"_test=[] {
        Fixture f;
        const auto base=f.bindings.abstractFactoryConstructor();
        const auto receiver=f.context.objectWithPrototype(f.bindings.abstractFactoryPrototype());
        f.context.set(o::Value(receiver),o::Key(u"constructor"),o::Value(base));
        f.context.setPayload(receiver,std::string("owned by another subsystem"));
        const auto result=f.context.call(base,o::Value(receiver),
            std::array<o::Value,1>{o::Value(o::Null{})});
        const auto& payload=std::as_const(f.context).payload(receiver);
        const auto* retained=std::any_cast<std::string>(&payload);
        const bool sourceFields=f.context.get(o::Value(receiver),o::Key(u"defaultKind"))==o::Value(u"unknown")&&
            std::holds_alternative<o::ObjectRef>(f.context.get(o::Value(receiver),o::Key(u"nodeMap")).data)&&
            std::holds_alternative<o::ObjectRef>(f.context.get(o::Value(receiver),o::Key(u"node")).data);
        expect(std::holds_alternative<o::Undefined>(result.data)&&sourceFields&&retained&&
            *retained=="owned by another subsystem")
            <<"factory-source-call-initializes-fields-and-preserves-independent-payload";
    };

    "constructors_use_current_default_nodes_and_dynamic_setNodeClass"_test=[] {
        Fixture f;
        const auto base=f.bindings.abstractFactoryConstructor();
        const auto wrapper=f.bindings.abstractWrapperFactoryConstructor();
        const auto nodeClass=f.context.nativeConstructor(u"ProbeClass",
            [](o::Context& cx,std::span<const o::Value>)->o::ObjectRef{return cx.object();});
        const auto defaults=f.context.object();
        f.context.set(o::Value(defaults),o::Key(u"alpha"),o::Value(nodeClass));
        f.context.set(o::Value(base),o::Key(u"defaultNodes"),o::Value(defaults));
        const auto baseInstance=f.context.construct(base,
            std::array<o::Value,1>{o::Value(o::Null{})});
        const auto baseNode=std::get<o::ObjectRef>(
            f.context.get(o::Value(baseInstance),o::Key(u"node")).data);
        const auto baseCreator=f.context.get(o::Value(baseNode),o::Key(u"alpha"));

        const auto wrapperDefaults=f.context.object();
        f.context.set(o::Value(wrapperDefaults),o::Key(u"beta"),o::Value(nodeClass));
        f.context.set(o::Value(wrapper),o::Key(u"defaultNodes"),o::Value(wrapperDefaults));
        const auto existing=f.context.objectWithPrototype(
            f.bindings.abstractWrapperFactoryPrototype());
        f.context.set(o::Value(existing),o::Key(u"constructor"),o::Value(wrapper));
        const auto callResult=f.context.call(wrapper,o::Value(existing),
            std::array<o::Value,1>{o::Value(o::Null{})});
        const auto wrapperNode=std::get<o::ObjectRef>(
            f.context.get(o::Value(existing),o::Key(u"node")).data);
        const auto wrapperCreator=f.context.get(o::Value(wrapperNode),o::Key(u"beta"));
        const bool pass=std::holds_alternative<o::ObjectRef>(baseCreator.data)&&
            std::holds_alternative<o::ObjectRef>(wrapperCreator.data)&&
            std::get<o::ObjectRef>(callResult.data)==existing&&
            f.context.get(o::Value(wrapper),o::Key(u"defaultNodes"))==o::Value(wrapperDefaults);
        expect(pass)<<"factory-current-default-nodes-and-live-constructor-dispatch";
    };

    "set_node_class_keeps_two_map_reads_and_defers_constructor_call"_test=[] {
        Fixture f;
        const auto base=f.bindings.abstractFactoryConstructor();
        const auto instance=f.context.construct(base,std::array<o::Value,1>{o::Value(f.context.object())});
        std::vector<std::string> order;
        const auto secondMap=f.context.object();
        f.context.set(o::Value(secondMap),o::Key(u"get"),o::Value(f.context.function(u"",[&](o::Context&,o::Value,
                std::span<const o::Value> getArgs)->o::Value {
            order.push_back("second.get");
            return getArgs.empty()?o::Value(o::Undefined{}):getArgs[0];
        })));
        const auto firstMap=f.context.object();
        auto current=firstMap;
        const auto getter=f.context.function(u"get nodeMap",[&](o::Context&,o::Value,
                std::span<const o::Value>)->o::Value {
            order.push_back("nodeMap.get");
            return o::Value(current);
        });
        f.context.define(instance,o::Key(u"nodeMap"),o::Descriptor::access(getter,std::nullopt,true,true));
        // The setter callback changes the getter's live result after .set.
        const auto setMap=f.context.function(u"",[&](o::Context& cx,o::Value receiver,
                std::span<const o::Value> args)->o::Value {
            order.push_back("first.set");
            current=secondMap;
            return receiver;
        });
        f.context.set(o::Value(firstMap),o::Key(u"set"),o::Value(setMap));
        const auto probeClass=f.context.nativeConstructor(u"ProbeClass",
            [](o::Context& cx,std::span<const o::Value>)->o::ObjectRef{return cx.object();});
        const std::array<o::Value,2> setterArgs{o::Value(u"late"),o::Value(probeClass)};
        const auto setter=f.context.get(o::Value(instance),o::Key(u"setNodeClass"));
        (void)f.context.callMember(setter,o::Value(instance),setterArgs,u"setNodeClass");
        const auto node=std::get<o::ObjectRef>(f.context.get(o::Value(instance),o::Key(u"node")).data);
        const auto creator=std::get<o::ObjectRef>(f.context.get(o::Value(node),o::Key(u"late")).data);
        const bool sequence=order==std::vector<std::string>{"nodeMap.get","first.set","nodeMap.get","second.get"};
        const bool metadata=f.context.get(o::Value(creator),o::Key(u"name"))==o::Value(u"")&&
            f.context.get(o::Value(creator),o::Key(u"length"))==o::Value(0.0);
        expect(sequence&&metadata)<<"factory-setNodeClass-live-reads-and-deferred-creator";
    };
};
}
