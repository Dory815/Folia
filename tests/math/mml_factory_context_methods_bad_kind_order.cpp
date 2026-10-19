// MathJax 4.1.3 NodeFactory.create argument-consumption order.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory_context_methods;
import folia.math.util.context_property_bindings;
import folia.math.util.native_identities;
import folia.math.util.map_bindings;
import folia.math.util.runtime_modules;
import folia.math.util.option_functions;

namespace {
using namespace boost::ut;
namespace opts=folia::math::options;
namespace math=folia::math;
namespace mml=folia::math::mml;
namespace tree=folia::math::tree;

struct Fixture {
    opts::Context context;
    math::OptionFunctionBindings functions{context};
    math::ContextPropertyBindings properties{context,functions};
    math::util::RuntimeModules modules{context};
    std::shared_ptr<opts::MapContextBindings> maps=modules.getOrCreate<opts::MapContextBindings>(
        u"ECMAScript/Map",[&]{return std::make_shared<opts::MapContextBindings>(context);});
    std::shared_ptr<tree::FactoryContextBaseModule> factoryBase=modules.getOrCreate<tree::FactoryContextBaseModule>(
        u"core/Tree/Factory",[&]{return std::make_shared<tree::FactoryContextBaseModule>(modules);});
    math::NativeIdentityBindings<mml::MmlNode*> nodes{context};
    math::NativeIdentityBindings<mml::MmlFactory*> factories{context};
    std::shared_ptr<std::atomic_bool> active=std::make_shared<std::atomic_bool>(true);
    std::shared_ptr<mml::MmlFactory> factory=std::make_shared<mml::MmlFactory>();
    opts::ObjectRef factoryObject=context.object();
    opts::ObjectRef nodeObject=context.object();
    mml::MmlFactoryContextMethods methods;
    Fixture()
        :methods(context,properties,nodes,factoryBase,active,
            [this](opts::ObjectRef object){
                auto* native=factories.resolve(object);
                const auto* payload=std::any_cast<math::NativeBindingPayload<mml::MmlFactory*>>(
                    &context.payload(object));
                if(!payload||!payload->lifetime)
                    throw opts::Exception("TypeError",u"MmlFactory has no retained owner");
                return std::shared_ptr<mml::MmlFactory>(payload->lifetime,native);
            },
            [this](mml::MmlNode& node){return nodes.find(&node);}) {
        factories.bind(factory.get(),factoryObject,std::static_pointer_cast<void>(factory));
        factory->setIdentityBinder(this,[this](mml::MmlNode& node,
                                               std::shared_ptr<mml::MmlFactory> owner){
            if(!nodes.find(&node))
                nodes.bind(&node,context.object(),std::static_pointer_cast<void>(owner));
        });
        context.set(opts::Value(factoryObject),opts::Key(u"node"),opts::Value(nodeObject));
        methods.bind(factoryObject,factory);
    }
};

suite g190_factory_create_diagnostic=[] {
    "abstract_node_factory_constructor_uses_shared_base_and_returns_call_receiver"_test=[] {
        Fixture f;
        const auto constructor=f.methods.constructor();
        const auto constructorLength=f.context.get(opts::Value(constructor),opts::Key(u"length"));
        const auto prototypeValue=f.context.get(opts::Value(constructor),opts::Key(u"prototype"));
        const auto* prototype=std::get_if<opts::ObjectRef>(&prototypeValue.data);
        const auto parentConstructor=f.context.prototype(constructor);
        expect(constructorLength==opts::Value(0.0));
        expect(prototype!=nullptr);
        expect(parentConstructor.has_value());
        if(prototype&&parentConstructor){
            expect(f.context.get(opts::Value(*parentConstructor),opts::Key(u"prototype"))==
                   opts::Value(f.methods.basePrototype()));
            expect(f.context.prototype(*prototype)==std::optional<opts::ObjectRef>(f.methods.basePrototype()));
            const auto instance=f.context.construct(constructor);
            expect(f.context.prototype(instance)==std::optional<opts::ObjectRef>(*prototype));
            const auto getKinds=f.context.get(opts::Value(instance),opts::Key(u"getKinds"));
            const auto kinds=f.context.callMember(getKinds,opts::Value(instance));
            const auto* kindsArray=std::get_if<opts::ObjectRef>(&kinds.data);
            expect(kindsArray!=nullptr);
            if(kindsArray)expect(f.context.arrayLength(*kindsArray)==0u);

            const auto existing=f.context.objectWithPrototype(*prototype);
            const auto baseConstructor=*parentConstructor;
            std::vector<opts::Value> observed;
            const auto replacementApply=f.context.function(u"replacement apply",
                [&observed,baseConstructor](opts::Context&,opts::Value thisValue,
                    std::span<const opts::Value> args){
                    expect(thisValue==opts::Value(baseConstructor));
                    observed.assign(args.begin(),args.end());
                    return opts::Value(opts::Undefined{});
                });
            f.context.define(baseConstructor,opts::Key(u"apply"),
                opts::Descriptor::data(opts::Value(replacementApply)));
            const std::array<opts::Value,1> arguments{opts::Value(u"sentinel")};
            const auto result=f.context.call(constructor,opts::Value(existing),arguments);
            expect(result==opts::Value(existing));
            expect(observed.size()==2);
            if(observed.size()==2){
                expect(observed[0]==opts::Value(existing));
                const auto* list=std::get_if<opts::ObjectRef>(&observed[1].data);
                expect(list!=nullptr);
                if(list)expect(std::ranges::equal(f.context.arrayValues(*list),std::array<opts::Value,1>{opts::Value(u"sentinel")}));
            }
        }
    };

    "mml_factory_missing_kind_does_not_observe_children_before_type_error"_test=[] {
        Fixture f;
        std::string trace;
        auto observe=[&](std::string_view event){if(!trace.empty())trace+="/";trace+=event;};
        const auto children=f.context.object();
        const auto lengthGetter=f.context.function(u"get length",
            [&observe](auto&,auto,std::span<const opts::Value>){observe("children.length");return opts::Value(1);});
        f.context.define(children,opts::Key(u"length"),opts::Descriptor::access(lengthGetter,{}));
        const auto iterator=f.context.object();
        auto done=f.context.object();
        f.context.set(done,opts::Key(u"done"),opts::Value(true));
        const auto next=f.context.function(u"next",
            [&observe,done](auto&,auto,std::span<const opts::Value>){observe("iterator.next");return opts::Value(done);});
        f.context.set(iterator,opts::Key(u"next"),opts::Value(next));
        const auto iteratorMethod=f.context.function(u"iterator",
            [&observe,iterator](auto&,auto,std::span<const opts::Value>){observe("children.[Symbol.iterator] call");return opts::Value(iterator);});
        const auto iteratorGetter=f.context.function(u"get [Symbol.iterator]",
            [&observe,iteratorMethod](auto&,auto,std::span<const opts::Value>){observe("children.[Symbol.iterator] get");return opts::Value(iteratorMethod);});
        f.context.define(children,opts::Key(f.context.intrinsics().symbolIterator),
            opts::Descriptor::access(iteratorGetter,{}));
        const auto create=f.context.get(f.factoryObject,opts::Key(u"create"));
        const std::array<opts::Value,3> args{
            opts::Value(u"missing-kind"),opts::Value(f.context.object()),opts::Value(children)};
        std::string name="returned";
        std::u16string message;
        try{(void)f.context.callMember(create,f.factoryObject,args);}
        catch(const opts::Exception& error){name=error.name;message=error.message;}
        expect(name=="TypeError");
        expect(message==u"this.node[kind] is not a function");
        expect(trace.empty())
            <<"Source CJS locks and expected no child observations in g190-factory-create-diagnostic/source-witness.cjs";
    };
    "mml_factory_non_callable_kind_does_not_observe_children_before_type_error"_test=[] {
        Fixture f;
        f.context.set(opts::Value(f.nodeObject),opts::Key(u"non-callable-kind"),
                      opts::Value(0.0));
        std::string trace;
        const auto children=f.context.object();
        const auto lengthGetter=f.context.function(u"get length",
            [&trace](auto&,auto,std::span<const opts::Value>){trace="children.length";return opts::Value(1.0);});
        f.context.define(children,opts::Key(u"length"),opts::Descriptor::access(lengthGetter,{}));
        const auto create=f.context.get(f.factoryObject,opts::Key(u"create"));
        const std::array<opts::Value,3> args{
            opts::Value(u"non-callable-kind"),opts::Value(f.context.object()),opts::Value(children)};
        std::string name="returned";
        std::u16string message;
        try{(void)f.context.callMember(create,f.factoryObject,args);}
        catch(const opts::Exception& error){name=error.name;message=error.message;}
        expect(name=="TypeError");
        expect(message==u"this.node[kind] is not a function");
        expect(trace.empty())<<"a selected non-callable property is not entered as a creator";
    };
    "mml_factory_callable_kind_enters_creator_after_selection"_test=[] {
        Fixture f;
        std::string trace;
        const auto children=f.context.object();
        const auto lengthGetter=f.context.function(u"get length",
            [&trace](auto&,auto,std::span<const opts::Value>){trace="children.length";return opts::Value(0.0);});
        f.context.define(children,opts::Key(u"length"),opts::Descriptor::access(lengthGetter,{}));
        const auto creator=f.context.function(u"mi creator",
            [](opts::Context& context,opts::Value,std::span<const opts::Value> args){
                (void)context.get(args[1],opts::Key(u"length"));
                return opts::Value(opts::Undefined{});
            });
        f.context.set(opts::Value(f.nodeObject),opts::Key(u"mi"),opts::Value(creator));
        const auto create=f.context.get(f.factoryObject,opts::Key(u"create"));
        const std::array<opts::Value,3> args{
            opts::Value(u"mi"),opts::Value(f.context.object()),opts::Value(children)};
        (void)f.context.callMember(create,f.factoryObject,args);
        expect(trace=="children.length")<<"a registered callable creator consumes children in its constructor";
    };
};
}
