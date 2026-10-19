#include "support/folia_test.hpp"
import folia.math.output.common;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.tree.factory_class_map_owner;
import folia.math.util.context_property_bindings;
import folia.math.util.map_bindings;
import folia.math.util.option_functions;
import folia.math.util.runtime_modules;

namespace {
using namespace boost::ut;
namespace opts=folia::math::options;
namespace common=folia::math::output::common;

suite common_factory_live_fields=[] {
    "common_creator_record_enumerability_reads_only_live_descriptor_metadata"_test=[] {
        opts::Context context;
        folia::math::OptionFunctionBindings functions{context};
        folia::math::ContextPropertyBindings properties{context,functions};
        folia::math::util::RuntimeModules modules{context};
        auto maps=modules.getOrCreate<opts::MapContextBindings>(u"ECMAScript/Map",
            [&]{return std::make_shared<opts::MapContextBindings>(context);});
        common::CommonFactoryContextBindings bindings{context};
        auto active=std::make_shared<std::atomic_bool>(true);
        bindings.bindFactoryMethods(modules,properties,*maps,
            [](opts::ObjectRef)->folia::math::mml::MmlNode&{
                throw opts::Exception("TypeError",u"unexpected node resolution");
            },
            [&](folia::math::mml::MmlNode&)->opts::ObjectRef{return context.object();},active);
        (void)bindings.registerConstructor();
        const auto factory=std::make_shared<common::CommonWrapperFactory>();
        const auto factoryObject=bindings.bind(factory);
        const auto recordValue=context.get(opts::Value(factoryObject),opts::Key(u"node"));
        const auto* record=std::get_if<opts::ObjectRef>(&recordValue.data);
        expect(record!=nullptr);
        if(!record)return;

        std::size_t getterCalls=0;
        const auto getter=context.function(u"getProbe",[&](opts::Context&,opts::Value,
            std::span<const opts::Value>){++getterCalls;return opts::Value(37.0);});
        const auto enumerableKey=opts::Key(u"metadata-enumerable");
        const auto hiddenKey=opts::Key(u"metadata-hidden");
        context.define(*record,enumerableKey,opts::Descriptor::access(getter,std::nullopt,true,true));
        context.define(*record,hiddenKey,opts::Descriptor::access(getter,std::nullopt,false,true));

        expect(context.ownPropertyEnumerable(recordValue,enumerableKey));
        expect(!context.ownPropertyEnumerable(recordValue,hiddenKey));
        expect(getterCalls==0u);
        expect(context.get(recordValue,enumerableKey)==opts::Value(37.0));
        expect(getterCalls==1u);
    };

    "common_factory_live_fields_do_not_retain_bindings_state_through_resolvers"_test=[] {
        opts::Context context;
        folia::math::OptionFunctionBindings functions{context};
        folia::math::ContextPropertyBindings properties{context,functions};
        folia::math::util::RuntimeModules modules{context};
        auto maps=modules.getOrCreate<opts::MapContextBindings>(u"ECMAScript/Map",
            [&]{return std::make_shared<opts::MapContextBindings>(context);});
        auto active=std::make_shared<std::atomic_bool>(true);
        std::weak_ptr<void> bindingsLifetime;
        {
            common::CommonFactoryContextBindings bindings{context};
            auto lifetime=bindings.lifetimeToken();
            bindingsLifetime=lifetime;
            lifetime.reset();
            bindings.bindFactoryMethods(modules,properties,*maps,
                [](opts::ObjectRef)->folia::math::mml::MmlNode&{
                    throw opts::Exception("TypeError",u"unexpected node resolution");
                },
                [&](folia::math::mml::MmlNode&)->opts::ObjectRef{return context.object();},active);
            (void)bindings.registerConstructor();
            (void)bindings.bind(std::make_shared<common::CommonWrapperFactory>());
        }
        expect(bindingsLifetime.expired());
    };
    "common_factory_node_prototype_and_own_key_order_are_live"_test=[] {
        opts::Context context;
        folia::math::OptionFunctionBindings functions{context};
        folia::math::ContextPropertyBindings properties{context,functions};
        folia::math::util::RuntimeModules modules{context};
        auto maps=modules.getOrCreate<opts::MapContextBindings>(u"ECMAScript/Map",
            [&]{return std::make_shared<opts::MapContextBindings>(context);});
        common::CommonFactoryContextBindings bindings{context};
        auto active=std::make_shared<std::atomic_bool>(true);
        const auto typedMmlIdentity=context.object();
        int typedMmlIdentityReads=0;
        bindings.bindFactoryMethods(modules,properties,*maps,
            [](opts::ObjectRef)->folia::math::mml::MmlNode&{
                throw opts::Exception("TypeError",u"unexpected node resolution");
            },
            [&](folia::math::mml::MmlNode&)->opts::ObjectRef{
                ++typedMmlIdentityReads;
                return typedMmlIdentity;
            },active);
        (void)bindings.registerConstructor();
        const auto factory=std::make_shared<common::CommonWrapperFactory>();
        const auto factoryObject=bindings.bind(factory);
        const auto secondFactory=std::make_shared<common::CommonWrapperFactory>();
        const auto secondFactoryObject=bindings.bind(secondFactory);
        const auto bindData=[&](std::u16string_view key,opts::Value value){
            context.set(opts::Value(factoryObject),opts::Key(std::u16string(key)),std::move(value));
        };
        const auto readData=[&](std::u16string_view key){
            return context.get(opts::Value(factoryObject),opts::Key(std::u16string(key)));
        };
        const opts::Value originalNodeValue=readData(u"node");

        // Assignments preserve their exact Source values. Failures happen at
        // the later operation which consumes the field.
        const auto mutableDefault=context.object();
        int conversions=0;
        std::u16string currentDefaultKey=u"late";
        context.define(mutableDefault,opts::Key(u"toString"),opts::Descriptor::data(opts::Value(
            context.function(u"toString",[&](opts::Context&,opts::Value,
                std::span<const opts::Value>){++conversions;return opts::Value(currentDefaultKey);}))));
        bindData(u"defaultKind",opts::Value(mutableDefault));
        expect(readData(u"defaultKind")==opts::Value(mutableDefault));
        bool defaultConsumed=false;
        try{(void)factory->createWrapper("unmapped");}
        catch(const opts::Exception&){defaultConsumed=true;}
        expect(defaultConsumed);
        expect(conversions==1);
        currentDefaultKey=u"mutated-later";
        try{(void)factory->createWrapper("unmapped");}
        catch(const opts::Exception&){}
        expect(conversions==2);

        auto externalStorage=maps->createStorage();
        const auto externalMap=maps->bind(externalStorage);
        bindData(u"nodeMap",opts::Value(externalMap));
        // Retain the actual Map adapter installed while the native data
        // field is active. This is the owner identity/lifetime observation;
        // later Source descriptor reads are tested independently above.
        const auto retainedExternalMapAccess=factory->nodeMapAccess();
        expect(retainedExternalMapAccess->identity()==externalStorage->identity());
        context.set(opts::Value(secondFactoryObject),opts::Key(u"nodeMap"),opts::Value(externalMap));
        expect(factory->nodeMapAccess()->identity()==externalStorage->identity());
        expect(secondFactory->nodeMapAccess()->identity()==externalStorage->identity());
        expect(readData(u"nodeMap")==opts::Value(externalMap));
        const auto rawConstructor=context.function(u"unregistered",[](opts::Context&,opts::Value,
            std::span<const opts::Value>){return opts::Value(u"raw");});
        factory->nodeMapAccess()->set(opts::Value(u"raw"),
            folia::math::tree::FactoryClassMapValue<common::CommonWrapper>::source(
                opts::Value(rawConstructor)));
        expect(externalStorage->get(opts::Value(u"raw"))==opts::Value(rawConstructor));
        expect(secondFactory->nodeMapAccess()->get(opts::Value(u"raw"))->sourceValue()!=nullptr);
        secondFactory->nodeMapAccess()->set(opts::Value(u"second-write"),
            folia::math::tree::FactoryClassMapValue<common::CommonWrapper>::source(
                opts::Value(rawConstructor)));
        expect(externalStorage->get(opts::Value(u"second-write"))==opts::Value(rawConstructor));
        const auto getNodeClass=context.get(opts::Value(factoryObject),opts::Key(u"getNodeClass"));
        expect(context.get(getNodeClass,opts::Key(u"name"))==opts::Value(u""));
        expect(context.get(getNodeClass,opts::Key(u"length"))==opts::Value(1));
        const std::array<opts::Value,1> rawKey{opts::Value(u"raw")};
        expect(context.callMember(getNodeClass,opts::Value(factoryObject),rawKey)==opts::Value(rawConstructor));
        const auto originalMapGet=context.get(opts::Value(externalMap),opts::Key(u"get"));
        opts::Value observedMapReceiver=opts::Undefined{};
        opts::Value observedMapKey=opts::Undefined{};
        const auto overriddenGet=context.function(u"",[&](opts::Context&,opts::Value receiver,
            std::span<const opts::Value> args){
            observedMapReceiver=receiver;
            observedMapKey=args.empty()?opts::Value(opts::Undefined{}):args.front();
            return opts::Value(rawConstructor);
        });
        context.set(opts::Value(externalMap),opts::Key(u"get"),opts::Value(overriddenGet));
        const std::array<opts::Value,1> overriddenKey{opts::Value(u"overridden")};
        expect(context.callMember(getNodeClass,opts::Value(factoryObject),overriddenKey)==opts::Value(rawConstructor));
        expect(observedMapReceiver==opts::Value(externalMap));
        expect(observedMapKey==overriddenKey.front());
        context.set(opts::Value(externalMap),opts::Key(u"get"),originalMapGet);
        const bool kindsValuesMatch=factory->getKindsValues()==std::vector<opts::Value>{opts::Value(u"raw"),opts::Value(u"second-write")};
        expect(kindsValuesMatch);

        int mapGetterCalls=0;
        const auto mapGetter=context.function(u"get",[&](opts::Context&,opts::Value,
            std::span<const opts::Value>){++mapGetterCalls;return opts::Value(externalMap);});
        context.define(factoryObject,opts::Key(u"nodeMap"),opts::Descriptor::access(mapGetter,{}));
        expect(context.callMember(getNodeClass,opts::Value(factoryObject),rawKey)==opts::Value(rawConstructor));
        expect(mapGetterCalls==1);
        const auto accessorKinds=factory->getKindsValues();
        expect(accessorKinds==std::vector<opts::Value>{opts::Value(u"raw"),opts::Value(u"second-write")});
        expect(mapGetterCalls==2);
        context.erase(factoryObject,opts::Key(u"nodeMap"));
        expect(!context.hasOwn(opts::Value(factoryObject),opts::Key(u"nodeMap")));
        bool deletedMapObserved=false;
        try{(void)context.callMember(getNodeClass,opts::Value(factoryObject),rawKey);}
        catch(const opts::Exception&){deletedMapObserved=true;}
        expect(deletedMapObserved);
        bool deletedMapTypedGetObserved=false;
        try{(void)factory->getNodeClass("raw");}
        catch(const opts::Exception&){deletedMapTypedGetObserved=true;}
        expect(deletedMapTypedGetObserved);
        bool deletedMapTypedKeysObserved=false;
        try{(void)factory->getKindsValues();}
        catch(const opts::Exception&){deletedMapTypedKeysObserved=true;}
        expect(deletedMapTypedKeysObserved);
        bool deletedMapNativeWriteObserved=false;
        try{factory->setWrapperClass("after-delete",{});}
        catch(const opts::Exception&){deletedMapNativeWriteObserved=true;}
        expect(deletedMapNativeWriteObserved);
        bindData(u"nodeMap",opts::Value(externalMap));

        bindData(u"nodeMap",opts::Value(opts::Null{}));
        expect(readData(u"nodeMap")==opts::Value(opts::Null{}));
        bool mapFailureDeferred=false;
        try{(void)factory->getNodeClass("unmapped");}
        catch(const opts::Exception&){mapFailureDeferred=true;}
        expect(mapFailureDeferred);
        bindData(u"nodeMap",opts::Value(externalMap));

        // setWrapperClass must store the Source constructor returned by the
        // current Map owner, then resolve that captured value only when the
        // generated creator is invoked.
        const auto delayedConstructor=context.object();
        auto delayedClass=std::make_shared<folia::math::tree::FactoryClass<common::CommonWrapper>>();
        delayedClass->staticData=common::CommonWrapperClassData{.kind="delayed"};
        int delayedConstructionCalls=0;
        delayedClass->construct=[&](folia::math::tree::FactoryServices<common::CommonWrapper>&,
                                    folia::math::tree::FactoryArgs)->common::CommonWrapper*{
            ++delayedConstructionCalls;
            throw opts::Exception("TypeError",u"delayed constructor reached");
        };
        bindings.bindWrapperClass(delayedClass,delayedConstructor);
        factory->setWrapperClass("generated",delayedClass);
        expect(externalStorage->get(opts::Value(u"generated"))==opts::Value(delayedConstructor));
        expect(delayedConstructionCalls==0);
        bool delayedConstructionReached=false;
        try{(void)factory->createWrapper("generated");}
        catch(const opts::Exception& error){
            delayedConstructionReached=error.name=="TypeError"&&error.message==u"delayed constructor reached";
        }
        expect(delayedConstructionReached);
        expect(delayedConstructionCalls==1);

        bindData(u"node",opts::Value(opts::Null{}));
        expect(readData(u"node")==opts::Value(opts::Null{}));
        bool nodeFailureDeferred=false;
        try{(void)factory->createWrapper("unmapped");}
        catch(const opts::Exception&){nodeFailureDeferred=true;}
        expect(nodeFailureDeferred);
        bindData(u"node",originalNodeValue);

        const auto nodeValue=context.get(opts::Value(factoryObject),opts::Key(u"node"));
        const auto* node=std::get_if<opts::ObjectRef>(&nodeValue.data);
        expect(node!=nullptr);
        if(!node)return;

        expect(context.prototype(*node)==
            std::optional<opts::ObjectRef>(context.intrinsics().objectPrototype));
        const auto prototype=context.object();
        context.setPrototype(*node,prototype);
        expect(context.prototype(*node)==std::optional<opts::ObjectRef>(prototype));
        context.setPrototype(*node,std::nullopt);
        expect(!context.prototype(*node).has_value());

        const auto symbol=context.symbol(u"last");
        context.define(*node,opts::Key(symbol),opts::Descriptor::data(opts::Value(1)));
        context.define(*node,opts::Key(u"zz-late"),opts::Descriptor::data(opts::Value(2)));
        context.define(*node,opts::Key(u"4294967295"),opts::Descriptor::data(opts::Value(5)));
        context.define(*node,opts::Key(u"3"),opts::Descriptor::data(opts::Value(3)));
        context.define(*node,opts::Key(u"1"),opts::Descriptor::data(opts::Value(4)));
        context.define(*node,opts::Key(u"4294967294"),opts::Descriptor::data(opts::Value(6)));
        const auto keys=context.ownKeys(opts::Value(*node));
        expect(keys.size()>=6);
        if(keys.size()>=6){
            expect(keys[0]==opts::Key(u"1"));
            expect(keys[1]==opts::Key(u"3"));
            expect(keys[2]==opts::Key(u"4294967294"));
            expect(keys[keys.size()-3]==opts::Key(u"zz-late"));
            expect(keys[keys.size()-2]==opts::Key(u"4294967295"));
            expect(keys.back()==opts::Key(symbol));
        }

        // Source create must consume current property descriptors. Accessor
        // replacement changes both lookups; deleting the accessor leaves no
        // stale native `node` cell available to the method.
        const auto firstNode=context.object();
        const auto liveNode=context.object();
        opts::Value creatorReceiver=opts::Undefined{};
        opts::Value creatorArgument=opts::Undefined{};
        opts::Value creatorSecondArgument=opts::Undefined{};
        const auto liveCreator=context.function(u"",[&](opts::Context&,opts::Value receiver,
            std::span<const opts::Value> args){
            creatorReceiver=receiver;
            creatorArgument=args.empty()?opts::Value(opts::Undefined{}):args.front();
            creatorSecondArgument=args.size()<2?opts::Value(opts::Undefined{}):args[1];
            return opts::Value(u"live-create");
        });
        context.set(opts::Value(liveNode),opts::Key(u"late"),opts::Value(liveCreator));
        int nodeGetterCalls=0,defaultGetterCalls=0,defaultKeyConversions=0;
        const auto nodeGetter=context.function(u"get",[&](opts::Context&,opts::Value,
            std::span<const opts::Value>){
            ++nodeGetterCalls;
            return opts::Value(nodeGetterCalls==1?firstNode:liveNode);
        });
        const auto defaultString=context.function(u"toString",[&](opts::Context&,opts::Value,
            std::span<const opts::Value>){++defaultKeyConversions;return opts::Value(u"late");});
        const auto defaultObject=context.object();
        context.set(opts::Value(defaultObject),opts::Key(u"toString"),opts::Value(defaultString));
        const auto defaultGetter=context.function(u"get",[&](opts::Context&,opts::Value,
            std::span<const opts::Value>){++defaultGetterCalls;return opts::Value(defaultObject);});
        context.define(factoryObject,opts::Key(u"node"),opts::Descriptor::access(nodeGetter,{}));
        context.define(factoryObject,opts::Key(u"defaultKind"),opts::Descriptor::access(defaultGetter,{}));

        // Exercise the same Source accessors through the real typed
        // CommonWrapperFactory::createWrapper -> createWithArguments path.
        // The first node misses; the second getter result supplies the
        // fallback creator. Its non-wrapper return then fails only at the
        // final native identity conversion, after the Source call occurred.
        nodeGetterCalls=defaultGetterCalls=defaultKeyConversions=0;
        bool typedResultRejected=false;
        const std::vector<std::any> typedArguments{std::any(opts::Value(23))};
        try{(void)factory->createWrapper("missing",typedArguments);}
        catch(const opts::Exception&){typedResultRejected=true;}
        expect(typedResultRejected);
        expect(nodeGetterCalls==2);
        expect(defaultGetterCalls==1);
        expect(defaultKeyConversions==1);
        expect(creatorReceiver==opts::Value(opts::Undefined{}));
        expect(creatorArgument==opts::Value(23));

        // A real typed MML node takes AbstractWrapperFactory::wrap and then
        // the same createWithArguments field-reader path.
        folia::math::mml::MmlFactory nativeMmlFactory;
        auto* typedMmlNode=nativeMmlFactory.create("mi",folia::math::tree::FactoryArgs{});
        const std::vector<std::any> typedWrapArguments{std::any(opts::Value(23))};
        nodeGetterCalls=defaultGetterCalls=defaultKeyConversions=typedMmlIdentityReads=0;
        bool typedWrapRejected=false;
        try{(void)factory->wrap(typedMmlNode,typedWrapArguments);}
        catch(const opts::Exception&){typedWrapRejected=true;}
        expect(typedWrapRejected);
        expect(nodeGetterCalls==2);
        expect(defaultGetterCalls==1);
        expect(defaultKeyConversions==1);
        expect(typedMmlIdentityReads==1);
        expect(creatorReceiver==opts::Value(opts::Undefined{}));
        expect(creatorArgument==opts::Value(typedMmlIdentity));
        expect(creatorSecondArgument==opts::Value(23));

        // The Context method takes the same shared field-reader route.
        nodeGetterCalls=defaultGetterCalls=defaultKeyConversions=0;
        creatorReceiver=opts::Undefined{};
        creatorArgument=opts::Undefined{};
        creatorSecondArgument=opts::Undefined{};
        const auto createMethod=context.get(opts::Value(factoryObject),opts::Key(u"create"));
        const std::array<opts::Value,2> createArgs{opts::Value(u"missing"),opts::Value(23)};
        expect(context.callMember(createMethod,opts::Value(factoryObject),createArgs)==opts::Value(u"live-create"));
        expect(nodeGetterCalls==2);
        expect(defaultGetterCalls==1);
        expect(defaultKeyConversions==1);
        expect(std::holds_alternative<opts::Undefined>(creatorReceiver.data));
        expect(creatorArgument==opts::Value(23));
        context.erase(factoryObject,opts::Key(u"defaultKind"));
        expect(!context.hasOwn(opts::Value(factoryObject),opts::Key(u"defaultKind")));
        bool deletedDefaultObserved=false;
        try{(void)factory->createWrapper("missing",typedWrapArguments);}
        catch(const opts::Exception&){deletedDefaultObserved=true;}
        expect(deletedDefaultObserved);
        context.erase(factoryObject,opts::Key(u"node"));
        expect(!context.hasOwn(opts::Value(factoryObject),opts::Key(u"node")));
        bool deletedNodeObserved=false;
        try{(void)context.callMember(createMethod,opts::Value(factoryObject),createArgs);}
        catch(const opts::Exception&){deletedNodeObserved=true;}
        expect(deletedNodeObserved);
        bool deletedNodeTypedWrapObserved=false;
        try{(void)factory->wrap(typedMmlNode,typedWrapArguments);}
        catch(const opts::Exception&){deletedNodeTypedWrapObserved=true;}
        expect(deletedNodeTypedWrapObserved);

        // RuntimeModules retains the canonical Map service after the local
        // caller drops its reference; the map identity remains the same.
        const auto canonicalMapIdentity=externalStorage->identity();
        active->store(false,std::memory_order_release);
        maps.reset();
        expect(retainedExternalMapAccess->identity()==canonicalMapIdentity);
    };
};
}
