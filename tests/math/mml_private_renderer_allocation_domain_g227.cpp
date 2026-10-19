#include "support/folia_test.hpp"
import folia.math;
import folia.math.core.mml_tree.mml_context_bindings;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;

namespace {
using namespace boost::ut;
namespace m=folia::math::mml;
namespace o=folia::math::options;
namespace t=folia::math::tex;
namespace tree=folia::math::tree;

struct SharedGraph {
    o::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::regexp_host::RegExpBindings regexp{context};
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input{context};
    folia::math::core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context,findMath,regexp};
    folia::math::util::RuntimeModules modules{context};
    std::unique_ptr<t::ParserRuntime> makeRuntime(){
        return std::make_unique<t::ParserRuntime>(context,functions,regexp,unicode,input,findTeX,&modules);
    }
};

o::ObjectRef textConstructor(SharedGraph& graph,t::ParserRuntime& runtime,o::ObjectRef factory){
    const auto map=std::get<o::ObjectRef>(graph.context.get(o::Value(factory),o::Key(u"nodeMap")).data);
    return std::get<o::ObjectRef>(runtime.maps.resolve(map)->get(o::Value(u"text")).data);
}

o::ObjectRef mmlConstructor(SharedGraph& graph,t::ParserRuntime& runtime,o::ObjectRef factory,
                            std::u16string_view name){
    const auto map=std::get<o::ObjectRef>(graph.context.get(o::Value(factory),o::Key(u"nodeMap")).data);
    return std::get<o::ObjectRef>(runtime.maps.resolve(map)->get(o::Value(std::u16string(name))).data);
}

o::ObjectRef constructText(SharedGraph& graph,o::ObjectRef constructor,o::ObjectRef factory){
    const auto prototype=std::get<o::ObjectRef>(
        graph.context.get(o::Value(constructor),o::Key(u"prototype")).data);
    const auto receiver=graph.context.objectWithPrototype(prototype);
    const std::array<o::Value,1> args{o::Value(factory)};
    expect(graph.context.call(constructor,o::Value(receiver),args)==o::Value(receiver));
    return receiver;
}

std::weak_ptr<m::MmlFactory> productFactory(const o::Context& context,o::ObjectRef identity){
    using Payload=folia::math::NativeBindingPayload<m::MmlNode*>;
    const auto* payload=std::any_cast<Payload>(&context.payload(identity));
    if(!payload||!payload->lifetime)
        throw o::Exception("TypeError",u"Source product has no canonical allocation owner");
    return std::static_pointer_cast<m::MmlFactory>(payload->lifetime);
}
}

suite mmlPrivateRendererAllocationDomainG227=[] {
    "private_attributes_fresh_source_results_retire_but_alias_values_survive"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;

        const auto sharedObject=graph.context.object();
        auto getterCalls=std::make_shared<std::size_t>(0);
        const auto getter=graph.context.function(u"get probe",[getterCalls](o::Context&,o::Value,std::span<const o::Value>){
            ++*getterCalls;return o::Value(1);
        });
        graph.context.define(sharedObject,o::Key(u"probe"),o::Descriptor::access(getter,std::nullopt,true,true));
        folia::math::PropertyList global;
        global.set("g",folia::math::Property("global-value"));
        auto owner=std::make_shared<folia::math::Attributes>(folia::math::PropertyList{},global);
        const auto sharedProperty=runtime->attributeBindings.fromValue(o::Value(sharedObject));
        owner->set("shared",sharedProperty);
        owner->setInherited("inheritedShared",sharedProperty);
        owner->defaultsLayer().set("defaultShared",sharedProperty);
        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        std::weak_ptr<folia::math::Attributes> weakOwner=owner;
        const auto identity=runtime->attributeBindings.bind(owner);

        auto call0=[&](std::u16string_view methodName){
            const auto method=std::get<o::ObjectRef>(graph.context.get(o::Value(identity),o::Key(std::u16string(methodName))).data);
            const std::array<o::Value,0> args{};
            return graph.context.callMember(method,o::Value(identity),args,methodName);
        };
        auto call1=[&](std::u16string_view methodName,const o::Value& argument){
            const auto method=std::get<o::ObjectRef>(graph.context.get(o::Value(identity),o::Key(std::u16string(methodName))).data);
            const std::array<o::Value,1> args{argument};
            return graph.context.callMember(method,o::Value(identity),args,methodName);
        };
        const auto attributesField=std::get<o::ObjectRef>(graph.context.get(o::Value(identity),o::Key(u"attributes")).data);
        const auto defaultsField=std::get<o::ObjectRef>(graph.context.get(o::Value(identity),o::Key(u"defaults")).data);
        const auto inheritedField=std::get<o::ObjectRef>(graph.context.get(o::Value(identity),o::Key(u"inherited")).data);
        const auto allAttributes=call0(u"getAllAttributes");
        const auto allDefaults=call0(u"getAllDefaults");
        const auto allInherited=call0(u"getAllInherited");
        expect(static_cast<bool>(allAttributes==o::Value(attributesField)));
        expect(static_cast<bool>(allDefaults==o::Value(defaultsField)));
        expect(static_cast<bool>(allInherited==o::Value(inheritedField)));
        const auto globalAlias=std::get<o::ObjectRef>(call0(u"getAllGlobals").data);
        const auto globalNamesField=std::get<o::ObjectRef>(graph.context.get(o::Value(identity),o::Key(u"global")).data);
        expect(globalAlias==globalNamesField);

        const auto list1=std::get<o::ObjectRef>(call1(u"getList",o::Value(u"shared")).data);
        const auto list2=std::get<o::ObjectRef>(call1(u"getList",o::Value(u"shared")).data);
        const auto explicit1=std::get<o::ObjectRef>(call0(u"getExplicitNames").data);
        const auto explicit2=std::get<o::ObjectRef>(call0(u"getExplicitNames").data);
        const auto inheritedNames=std::get<o::ObjectRef>(call0(u"getInheritedNames").data);
        const auto defaultNames=std::get<o::ObjectRef>(call0(u"getDefaultNames").data);
        const auto globalNames=std::get<o::ObjectRef>(call0(u"getGlobalNames").data);
        const std::array<o::ObjectRef,7> fresh{list1,list2,explicit1,explicit2,inheritedNames,defaultNames,globalNames};
        expect(static_cast<bool>(list1!=list2));
        expect(static_cast<bool>(explicit1!=explicit2));
        expect(static_cast<bool>(graph.context.get(o::Value(list1),o::Key(u"shared"))==o::Value(sharedObject)));
        expect(static_cast<bool>(graph.context.get(o::Value(list2),o::Key(u"shared"))==o::Value(sharedObject)));
        const auto getValue=call1(u"get",o::Value(u"shared"));
        const auto inheritedValue=call1(u"getInherited",o::Value(u"inheritedShared"));
        const auto defaultValue=call1(u"getDefault",o::Value(u"defaultShared"));
        expect(static_cast<bool>(getValue==o::Value(sharedObject)));
        expect(static_cast<bool>(inheritedValue==o::Value(sharedObject)));
        expect(static_cast<bool>(defaultValue==o::Value(sharedObject)));
        expect(static_cast<bool>(graph.context.get(o::Value(explicit1),o::Key(u"length"))==o::Value(1)));
        expect(static_cast<bool>(graph.context.get(o::Value(explicit1),o::Key(u"0"))==o::Value(u"shared")));
        expect(static_cast<bool>(graph.context.get(o::Value(inheritedNames),o::Key(u"length"))==o::Value(1)));
        expect(static_cast<bool>(graph.context.get(o::Value(inheritedNames),o::Key(u"0"))==o::Value(u"inheritedShared")));
        expect(static_cast<bool>(graph.context.get(o::Value(defaultNames),o::Key(u"length"))==o::Value(1)));
        expect(static_cast<bool>(graph.context.get(o::Value(defaultNames),o::Key(u"0"))==o::Value(u"defaultShared")));
        expect(static_cast<bool>(graph.context.get(o::Value(globalNames),o::Key(u"length"))==o::Value(1)));
        expect(static_cast<bool>(graph.context.get(o::Value(globalNames),o::Key(u"0"))==o::Value(u"g")));

        std::array<std::weak_ptr<folia::math::PropertyStorage>,7> weakFresh{};
        for(std::size_t i=0;i<fresh.size();++i){
            const auto storage=runtime->attributeBindings.fromValue(o::Value(fresh[i]));
            const auto* handle=std::get_if<std::shared_ptr<folia::math::PropertyStorage>>(&storage.value);
            expect(handle!=nullptr);
            if(handle)weakFresh[i]=*handle;
        }
        owner.reset();
        bindings->retirePrivateRendererProjectionCapture(capture);
        expect(weakOwner.expired());
        for(const auto object:fresh){
            bool retired=false;
            try{(void)graph.context.kind(object);}
            catch(const o::Exception& error){retired=error.name=="TypeError";}
            expect(retired);
        }
        expect(std::all_of(weakFresh.begin(),weakFresh.end(),[](const auto& storage){return storage.expired();}));
        expect(graph.context.kind(sharedObject)==o::ObjectKind::object);
        expect(graph.context.kind(globalAlias)==o::ObjectKind::object);
        expect(static_cast<bool>(getValue==o::Value(sharedObject)));
        expect(static_cast<bool>(inheritedValue==o::Value(sharedObject)));
        expect(static_cast<bool>(defaultValue==o::Value(sharedObject)));
        expect(*getterCalls==0u);
        for(const auto originalLayer:{attributesField,defaultsField,inheritedField}){
            bool retired=false;
            try{(void)graph.context.kind(originalLayer);}
            catch(const o::Exception& error){retired=error.name=="TypeError";}
            expect(retired);
        }
    };

    "private_attributes_fresh_projection_failure_retains_pending_capture_after_stale_projection"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;

        const auto countBefore=runtime->attributeBindings.canonicalHandleCountForDiagnostics();
        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        auto owner=std::make_shared<folia::math::Attributes>(folia::math::PropertyList{},folia::math::PropertyList{});
        std::weak_ptr<folia::math::Attributes> weakOwner=owner;
        const auto identity=runtime->attributeBindings.bind(owner);
        const auto listMethod=std::get<o::ObjectRef>(graph.context.get(o::Value(identity),o::Key(u"getList")).data);
        const std::array<o::Value,1> listArgs{o::Value(u"missing")};
        const auto list=std::get<o::ObjectRef>(graph.context.callMember(listMethod,o::Value(identity),listArgs).data);
        const auto namesMethod=std::get<o::ObjectRef>(graph.context.get(o::Value(identity),o::Key(u"getExplicitNames")).data);
        const std::array<o::Value,0> noArgs{};
        const auto names=std::get<o::ObjectRef>(graph.context.callMember(namesMethod,o::Value(identity),noArgs).data);
        std::weak_ptr<folia::math::PropertyStorage> weakNames,weakList;
        {
            const auto namesProperty=runtime->attributeBindings.fromValue(o::Value(names));
            const auto listProperty=runtime->attributeBindings.fromValue(o::Value(list));
            weakNames=std::get<std::shared_ptr<folia::math::PropertyStorage>>(namesProperty.value);
            weakList=std::get<std::shared_ptr<folia::math::PropertyStorage>>(listProperty.value);
        }
        owner.reset();

        // The names array is registered last and retires first. The stale list
        // record then makes cleanup stop after that earlier projection and the
        // canonical Attributes identity have already retired.
        graph.context.retireObjectRecord(list);
        std::optional<std::string> firstName,secondName;
        std::optional<std::u16string> firstMessage,secondMessage;
        try{bindings->retirePrivateRendererProjectionCapture(capture);}
        catch(const o::Exception& error){firstName=error.name;firstMessage=error.message;}
        expect(static_cast<bool>(firstName&&*firstName=="TypeError"));
        expect(firstMessage.has_value());
        expect(weakOwner.expired());
        expect(runtime->attributeBindings.canonicalHandleCountForDiagnostics()==countBefore);
        expect(weakNames.expired());
        expect(!weakList.expired());
        bool namesRetired=false,identityRetired=false;
        try{(void)graph.context.kind(names);}catch(const o::Exception& error){namesRetired=error.name=="TypeError";}
        try{(void)graph.context.kind(identity);}catch(const o::Exception& error){identityRetired=error.name=="TypeError";}
        expect(namesRetired&&identityRetired);

        try{bindings->retirePrivateRendererProjectionCapture(capture);}
        catch(const o::Exception& error){secondName=error.name;secondMessage=error.message;}
        expect(secondName&&firstName&&*secondName==*firstName);
        expect(static_cast<bool>(secondMessage&&firstMessage&&*secondMessage==*firstMessage));
        expect(runtime->attributeBindings.canonicalHandleCountForDiagnostics()==countBefore);
        expect(!weakList.expired());
    };


    "private_renderer_attributes_projection_failure_preserves_post_identity_retry_progress"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;

        const auto countBefore=runtime->attributeBindings.canonicalHandleCountForDiagnostics();
        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        auto owner=std::make_shared<folia::math::Attributes>(folia::math::PropertyList{},folia::math::PropertyList{});
        std::weak_ptr<folia::math::Attributes> weakOwner=owner;
        const auto identity=runtime->attributeBindings.bind(owner);
        const std::array<std::u16string_view,3> fields{u"defaults",u"inherited",u"attributes"};
        std::array<o::ObjectRef,3> projections{};
        for(std::size_t i=0;i<fields.size();++i){
            const auto value=graph.context.get(o::Value(identity),o::Key(std::u16string(fields[i])));
            const auto* object=std::get_if<o::ObjectRef>(&value.data);
            expect(object!=nullptr);
            if(object)projections[i]=*object;
        }
        const std::weak_ptr<folia::math::PropertyStorage> pendingDefaults=
            std::get<std::shared_ptr<folia::math::PropertyStorage>>(folia::math::Property(owner->defaultsLayer()).value);
        owner.reset();

        // Deliberately invalidate the last pending projection before the first
        // retirement. The capture has already retained the exact native storage.
        graph.context.retireObjectRecord(projections[0]);
        std::optional<std::string> firstName,secondName;
        std::optional<std::u16string> firstMessage,secondMessage;
        try{bindings->retirePrivateRendererProjectionCapture(capture);}
        catch(const o::Exception& error){firstName=error.name;firstMessage=error.message;}
        expect(firstName.has_value()&&firstMessage.has_value());
        expect(static_cast<bool>(firstName&&*firstName=="TypeError"));
        expect(static_cast<bool>(firstMessage&&
            *firstMessage==u"Object handle refers to a retired Context record"));
        expect(weakOwner.expired());
        expect(runtime->attributeBindings.canonicalHandleCountForDiagnostics()==countBefore);
        expect(!pendingDefaults.expired());
        for(const auto projection:projections){
            bool stale=false;
            try{(void)graph.context.kind(projection);}
            catch(const o::Exception& error){stale=error.name=="TypeError";}
            expect(stale);
        }
        bool identityStale=false;
        try{(void)graph.context.kind(identity);}
        catch(const o::Exception& error){identityStale=error.name=="TypeError";}
        expect(identityStale);

        // Identity retirement and the first two layer retirements have already
        // completed. The same token must resume at the still-pending defaults
        // storage, producing the same stale-record error rather than an inactive
        // identity-capture-token error. This fixture is intentionally not repaired.
        try{bindings->retirePrivateRendererProjectionCapture(capture);}
        catch(const o::Exception& error){secondName=error.name;secondMessage=error.message;}
        expect(secondName&&firstName&&*secondName==*firstName);
        expect(static_cast<bool>(secondMessage&&firstMessage&&*secondMessage==*firstMessage));
        expect(runtime->attributeBindings.canonicalHandleCountForDiagnostics()==countBefore);
        expect(!pendingDefaults.expired());
    };

// Draft fragment for integration into mmlPrivateRendererAllocationDomainG227.
// Uses its SharedGraph, helpers, aliases and existing boost.ut imports.

"private_factory_node_map_has_owner_exact_canonical_retirement"_test=[] {
    SharedGraph graph;
    auto runtime=graph.makeRuntime();
    runtime->initializeParserRuntime();
    (void)t::BasePackage::initializeModule(*runtime);
    const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
    expect(bindings!=nullptr);
    if(!bindings)return;

    auto ambient=std::make_shared<m::MmlFactory>();
    const auto ambientIdentity=runtime->bindMmlFactory(ambient);
    const auto ambientMap=std::get<o::ObjectRef>(graph.context.get(
        o::Value(ambientIdentity),o::Key(u"nodeMap")).data);
    const auto baseline=runtime->maps.canonicalStorageCountForDiagnostics();
    auto domain=bindings->makePrivateSourceAllocationDomain();
    bindings->bindSourceAllocationDomain(domain);
    const auto factory=runtime->mmlFactoryIdentities.find(&domain.factory());
    expect(factory.has_value());
    if(!factory)return;

    const auto first=graph.context.get(o::Value(*factory),o::Key(u"nodeMap"));
    const auto* map=std::get_if<o::ObjectRef>(&first.data);
    expect(map!=nullptr);
    if(!map)return;
    const auto repeated=graph.context.get(o::Value(*factory),o::Key(u"nodeMap"));
    expect(repeated==first) << "projected native nodeMap remains Context-canonical";
    expect(runtime->maps.canonicalStorageCountForDiagnostics()==baseline+1u);
    auto storage=runtime->maps.resolve(*map);
    const std::weak_ptr<o::MapStorage> weakStorage=storage;
    const auto textValue=storage->get(o::Value(u"text"));
    const auto* textConstructor=std::get_if<o::ObjectRef>(&textValue.data);
    expect(textConstructor!=nullptr);
    if(!textConstructor)return;
    storage.reset();

    const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
    const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
    runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
    runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
    bindings->retireSourceAllocationDomain(domain);

    expect(throws<o::Exception>([&]{(void)graph.context.kind(*map);}))
        << "exact factory-owned projected Map Context object is retired with its owner";
    expect(throws<o::Exception>([&]{(void)runtime->maps.resolve(*map);}))
        << "retirement removes the canonical sidecar before owner storage can persist";
    expect(weakStorage.expired()) << "private projected MapStorage releases after its exact owner retires";
    expect(graph.context.kind(*textConstructor)==o::ObjectKind::function)
        << "shared class constructor metadata remains live after one private factory retires";
    expect(graph.context.kind(ambientMap)==o::ObjectKind::instance);
    expect(runtime->maps.resolve(ambientMap)!=nullptr)
        << "an ambient factory Map remains canonical and usable";
    expect(runtime->maps.canonicalStorageCountForDiagnostics()==baseline);
};

"private_factory_node_map_replacement_preserves_escaped_old_map_and_new_map"_test=[] {
    SharedGraph graph;
    auto runtime=graph.makeRuntime();
    runtime->initializeParserRuntime();
    (void)t::BasePackage::initializeModule(*runtime);
    const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
    expect(bindings!=nullptr);
    if(!bindings)return;

    const auto baseline=runtime->maps.canonicalStorageCountForDiagnostics();
    auto domain=bindings->makePrivateSourceAllocationDomain();
    bindings->bindSourceAllocationDomain(domain);
    const auto factory=runtime->mmlFactoryIdentities.find(&domain.factory());
    expect(factory.has_value());
    if(!factory)return;

    const auto privateValue=graph.context.get(o::Value(*factory),o::Key(u"nodeMap"));
    const auto* privateMap=std::get_if<o::ObjectRef>(&privateValue.data);
    expect(privateMap!=nullptr);
    if(!privateMap)return;
    const auto replacement=runtime->maps.create();
    graph.context.set(o::Value(*factory),o::Key(u"nodeMap"),o::Value(replacement));
    expect(graph.context.get(o::Value(*factory),o::Key(u"nodeMap"))==o::Value(replacement));

    const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
    const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
    runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
    runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
    bindings->retireSourceAllocationDomain(domain);

    expect(graph.context.kind(*privateMap)==o::ObjectKind::instance)
        << "the old Map may have escaped before replacement, so preserve its ECMAScript identity";
    expect(runtime->maps.resolve(*privateMap)!=nullptr);
    expect(graph.context.kind(replacement)==o::ObjectKind::instance)
        << "Source replacement Map remains a live canonical Map";
    expect(runtime->maps.resolve(replacement)!=nullptr);
    expect(runtime->maps.canonicalStorageCountForDiagnostics()==baseline+2u)
        << "both escaped old Map and replacement remain registered";
};

"map_binding_retirement_is_exact_retryable_and_does_not_remove_another_map"_test=[] {
    SharedGraph graph;
    auto runtime=graph.makeRuntime();
    const auto storage=runtime->maps.createStorage();
    const auto object=runtime->maps.bind(storage);
    const auto identity=storage->identity();
    const auto unregistered=graph.context.object();
    expect(!runtime->maps.retireBinding(unregistered,identity));
    expect(graph.context.kind(unregistered)==o::ObjectKind::object);
    const auto otherStorage=runtime->maps.createStorage();
    const auto other=runtime->maps.bind(otherStorage);
    expect(throws<o::Exception>([&]{
        (void)runtime->maps.retireBinding(object,otherStorage->identity());
    })) << "wrong expected identity fails before touching the object or registry";
    expect(graph.context.kind(object)==o::ObjectKind::instance);
    expect(runtime->maps.resolve(object)==storage);
    expect(graph.context.kind(other)==o::ObjectKind::instance);
    expect(runtime->maps.resolve(other)==otherStorage);
    expect(runtime->maps.retireBinding(object,identity));
    expect(throws<o::Exception>([&]{(void)graph.context.kind(object);}));
    expect(!runtime->maps.retireBinding(object,identity))
        << "already retired binding is an idempotent no-op";
    expect(graph.context.kind(unregistered)==o::ObjectKind::object);
    expect(graph.context.kind(other)==o::ObjectKind::instance);
};

    "private_renderer_attributes_capture_retires_only_new_canonical_owners_after_layer_materialization"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;

        auto ambient=std::make_shared<folia::math::Attributes>(folia::math::PropertyList{},folia::math::PropertyList{});
        const auto ambientIdentity=runtime->attributeBindings.bind(ambient);
        const auto ambientCount=runtime->attributeBindings.canonicalHandleCountForDiagnostics();

        auto factory=std::make_shared<m::MmlFactory>();
        const auto factoryIdentity=runtime->bindMmlFactory(factory);
        const auto nodeMap=std::get<o::ObjectRef>(graph.context.get(o::Value(factoryIdentity),o::Key(u"nodeMap")).data);
        const auto mrow=std::get<o::ObjectRef>(runtime->maps.resolve(nodeMap)->get(o::Value(u"mrow")).data);
        const auto classDefaults=std::get<o::ObjectRef>(graph.context.get(o::Value(mrow),o::Key(u"defaults")).data);
        const auto defaultMathVariant=graph.context.get(o::Value(classDefaults),o::Key(u"mathvariant"));

        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        const auto sharedGlobal=graph.context.object();
        auto requestOwner=std::make_shared<folia::math::Attributes>(folia::math::PropertyList{},
            runtime->attributeBindings.fromValue(o::Value(sharedGlobal)));
        std::weak_ptr<folia::math::Attributes> weakRequestOwner=requestOwner;
        const auto requestIdentity=runtime->attributeBindings.bind(requestOwner);
        std::array<std::weak_ptr<folia::math::PropertyStorage>,3> weakStorages{
            std::get<std::shared_ptr<folia::math::PropertyStorage>>(folia::math::Property(requestOwner->defaultsLayer()).value),
            std::get<std::shared_ptr<folia::math::PropertyStorage>>(folia::math::Property(requestOwner->inheritedLayer()).value),
            std::get<std::shared_ptr<folia::math::PropertyStorage>>(folia::math::Property(requestOwner->attributesLayer()).value)};
        requestOwner.reset();
        std::array<o::ObjectRef,4> projections{};
        const std::array<std::u16string_view,4> fields{u"global",u"defaults",u"inherited",u"attributes"};
        for(std::size_t i=0;i<fields.size();++i){
            const auto field=graph.context.get(o::Value(requestIdentity),o::Key(std::u16string(fields[i])));
            const auto* object=std::get_if<o::ObjectRef>(&field.data);
            expect(object!=nullptr);
            if(object)projections[i]=*object;
        }
        for(std::size_t i=0;i<fields.size();++i)
            expect(graph.context.get(o::Value(requestIdentity),o::Key(std::u16string(fields[i])))==o::Value(projections[i]));
        auto userGetterCalls=std::make_shared<std::size_t>(0);
        const auto userGetter=graph.context.function(u"get diagnostic",[userGetterCalls](o::Context&,o::Value,std::span<const o::Value>){
            ++*userGetterCalls;return o::Value(7);
        });
        graph.context.define(sharedGlobal,o::Key(u"userGetterProbe"),
            o::Descriptor::access(userGetter,std::nullopt,true,true));

        bindings->retirePrivateRendererProjectionCapture(capture);
        expect(weakRequestOwner.expired());
        expect(runtime->attributeBindings.canonicalHandleCountForDiagnostics()==ambientCount);
        expect(static_cast<bool>(runtime->attributeBindings.resolve(ambientIdentity)==ambient));
        expect(throws<o::Exception>([&]{(void)graph.context.kind(requestIdentity);}));
        expect(*userGetterCalls==0u);
        expect(graph.context.kind(projections[0])==o::ObjectKind::object);
        expect(graph.context.kind(sharedGlobal)==o::ObjectKind::object);
        expect(std::all_of(weakStorages.begin(),weakStorages.end(),[](const auto& storage){return storage.expired();}));
        expect(graph.context.get(o::Value(sharedGlobal),o::Key(u"userGetterProbe"))==o::Value(7));
        expect(graph.context.kind(classDefaults)==o::ObjectKind::object);
        expect(graph.context.get(o::Value(classDefaults),o::Key(u"mathvariant"))==defaultMathVariant);

        const auto lazyCapture=bindings->beginPrivateRendererProjectionCapture();
        auto lazyOwner=std::make_shared<folia::math::Attributes>(folia::math::PropertyList{},folia::math::PropertyList{});
        const auto lazyIdentity=runtime->attributeBindings.bind(lazyOwner);
        const std::array<std::weak_ptr<folia::math::PropertyStorage>,3> lazyStorages{
            std::get<std::shared_ptr<folia::math::PropertyStorage>>(folia::math::Property(lazyOwner->defaultsLayer()).value),
            std::get<std::shared_ptr<folia::math::PropertyStorage>>(folia::math::Property(lazyOwner->inheritedLayer()).value),
            std::get<std::shared_ptr<folia::math::PropertyStorage>>(folia::math::Property(lazyOwner->attributesLayer()).value)};
        lazyOwner.reset();
        bindings->retirePrivateRendererProjectionCapture(lazyCapture);
        expect(throws<o::Exception>([&]{(void)graph.context.kind(lazyIdentity);}));
        expect(std::all_of(lazyStorages.begin(),lazyStorages.end(),[](const auto& storage){return storage.expired();}));
    };

    "private_renderer_attributes_capture_retires_only_initial_layers_and_keeps_source_replacements"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;

        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        auto owner=std::make_shared<folia::math::Attributes>(folia::math::PropertyList{},folia::math::PropertyList{});
        const auto identity=runtime->attributeBindings.bind(owner);
        const std::weak_ptr<folia::math::PropertyStorage> initialDefaults=
            std::get<std::shared_ptr<folia::math::PropertyStorage>>(folia::math::Property(owner->defaultsLayer()).value);
        const auto sourceReplacement=graph.context.object();
        graph.context.set(o::Value(identity),o::Key(u"defaults"),o::Value(sourceReplacement));
        const auto replacementProjection=graph.context.get(o::Value(identity),o::Key(u"defaults"));
        expect(std::get_if<o::ObjectRef>(&replacementProjection.data)!=nullptr);
        bindings->retirePrivateRendererProjectionCapture(capture);
        owner.reset();
        expect(initialDefaults.expired());
        expect(graph.context.kind(sourceReplacement)==o::ObjectKind::object);
        expect(throws<o::Exception>([&]{(void)graph.context.kind(identity);}));
    };

    "private_renderer_attributes_capture_preserves_foreign_token_and_failed_retirement_retry"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;
        using AttributesOwner=std::shared_ptr<folia::math::Attributes>;
        folia::math::NativeIdentityBindings<AttributesOwner> foreign(graph.context);
        const auto foreignToken=foreign.beginRequestCapture();
        expect(throws<o::Exception>([&]{bindings->retirePrivateRendererProjectionCapture(foreignToken);}));
        foreign.retireCapturedRequestBindings(foreignToken);

        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        auto owner=std::make_shared<folia::math::Attributes>(folia::math::PropertyList{},folia::math::PropertyList{});
        std::weak_ptr<folia::math::Attributes> weakOwner=owner;
        const auto identity=runtime->attributeBindings.bind(owner);
        std::any originalPayload=graph.context.payload(identity);
        graph.context.setPayload(identity,std::string("temporary invalid payload"));
        expect(throws<o::Exception>([&]{bindings->retirePrivateRendererProjectionCapture(capture);}));
        graph.context.setPayload(identity,std::move(originalPayload));
        bindings->retirePrivateRendererProjectionCapture(capture);
        owner.reset();originalPayload.reset();
        expect(weakOwner.expired());
        expect(throws<o::Exception>([&]{(void)graph.context.kind(identity);}));
    };
    "private_domain_reuses_the_single_mml_prototype_getter_after_warmup"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;
        const auto prototype=bindings->factoryMethods().prototype();
        std::optional<o::ObjectRef> initialGetter;
        auto bindRetire=[&]{
            auto domain=bindings->makePrivateSourceAllocationDomain();
            bindings->bindSourceAllocationDomain(domain);
            const auto descriptor=graph.context.descriptor(o::Value(prototype),o::Key(u"MML"));
            expect(descriptor&&descriptor->accessor&&descriptor->getter.has_value());
            if(descriptor&&descriptor->getter)initialGetter=descriptor->getter;
            bindings->retireSourceAllocationDomain(domain);
        };
        bindRetire();
        const auto warmed=graph.context.recordCountsForDiagnostics();
        const auto getterAfterWarm=initialGetter;
        for(std::size_t i=0;i<4;++i){
            auto domain=bindings->makePrivateSourceAllocationDomain();
            bindings->bindSourceAllocationDomain(domain);
            const auto descriptor=graph.context.descriptor(o::Value(prototype),o::Key(u"MML"));
            expect(descriptor&&descriptor->accessor&&descriptor->getter&&getterAfterWarm&&
                   *descriptor->getter==*getterAfterWarm);
            bindings->retireSourceAllocationDomain(domain);
            const auto after=graph.context.recordCountsForDiagnostics();
            expect(after.live==warmed.live);
            expect(after.ownerBearing==warmed.ownerBearing);
        }
    };

    "later_private_factory_binding_preserves_a_replaced_mml_prototype_property"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;
        const auto prototype=bindings->factoryMethods().prototype();
        {
            auto domain=bindings->makePrivateSourceAllocationDomain();
            bindings->bindSourceAllocationDomain(domain);
            bindings->retireSourceAllocationDomain(domain);
        }
        const auto userValue=o::Value(u"user MML property");
        graph.context.define(prototype,o::Key(u"MML"),o::Descriptor::data(userValue,false,false,true));
        {
            auto domain=bindings->makePrivateSourceAllocationDomain();
            bindings->bindSourceAllocationDomain(domain);
            const auto descriptor=graph.context.descriptor(o::Value(prototype),o::Key(u"MML"));
            expect(descriptor&&!descriptor->accessor&&descriptor->value==userValue&&
                   !descriptor->writable&&!descriptor->enumerable&&descriptor->configurable);
            expect(graph.context.get(o::Value(prototype),o::Key(u"MML"))==userValue);
            bindings->retireSourceAllocationDomain(domain);
        }
    };

    "later_private_factory_binding_preserves_deleted_mml_prototype_property"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;
        const auto prototype=bindings->factoryMethods().prototype();
        {
            auto domain=bindings->makePrivateSourceAllocationDomain();
            bindings->bindSourceAllocationDomain(domain);
            bindings->retireSourceAllocationDomain(domain);
        }
        graph.context.erase(prototype,o::Key(u"MML"));
        {
            auto domain=bindings->makePrivateSourceAllocationDomain();
            bindings->bindSourceAllocationDomain(domain);
            expect(!graph.context.hasOwn(o::Value(prototype),o::Key(u"MML")));
            bindings->retireSourceAllocationDomain(domain);
        }
    };

    "private_domain_supports_native_factory_creation_and_retires_exact_factory_identity"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;

        std::weak_ptr<m::MmlFactory> owner;
        const auto factoryCount=runtime->mmlFactoryIdentities.canonicalHandleCount();
        {
            const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
            const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
            auto domain=bindings->makePrivateSourceAllocationDomain();
            bindings->bindSourceAllocationDomain(domain);
            auto* const domainFactory=&domain.factory();
            owner=domainFactory->weak_from_this();
            const auto factoryIdentity=runtime->mmlFactoryIdentities.find(domainFactory);
            expect(factoryIdentity.has_value())
                << "private allocator has a canonical Context factory identity for native initialization";
            bindings->bindSourceAllocationDomain(domain);
            expect(runtime->mmlFactoryIdentities.find(domainFactory)==factoryIdentity)
                << "repeated binding resumes through the canonical completion-aware binder without changing identity";
            if(factoryIdentity){
                const auto factoryNode=graph.context.get(o::Value(*factoryIdentity),o::Key(u"node"));
                const auto* creatorRecord=std::get_if<o::ObjectRef>(&factoryNode.data);
                expect(creatorRecord!=nullptr)
                    << "factory.node exposes the Context creator record for this exact factory";
                if(creatorRecord){
                    const auto mfrac=graph.context.get(o::Value(*creatorRecord),o::Key(u"mfrac"));
                    expect(std::get_if<o::ObjectRef>(&mfrac.data)!=nullptr)
                        << "factory.node lazily materializes its owner-retaining creator function";
                }
            }

            {
                auto active=bindings->activateSourceAllocation(domain);
                auto* fraction=dynamic_cast<m::AbstractMmlNode*>(
                    domain.factory().create("mfrac",tree::FactoryArgs{}));
                expect(fraction!=nullptr);
                if(fraction){
                    const auto identity=runtime->mmlNodeIdentities.find(fraction);
                    expect(identity.has_value());
                    if(identity){
                        expect(productFactory(graph.context,*identity).lock().get()==&domain.factory());
                        fraction->setInheritedAttributes(folia::math::PropertyList{},false,0,false);
                        const auto children=fraction->childArrayOwner();
                        expect(children&&children->size()==2_u)
                            << "inheritance repairs both missing fraction children through the native factory";
                        if(children)for(std::size_t index=0;index<children->size();++index){
                            auto* child=children->get(index).nodeOrNull();
                            expect(child&&child->kind()=="mrow");
                            const auto childIdentity=runtime->mmlNodeIdentities.find(child);
                            expect(childIdentity.has_value());
                            if(childIdentity)expect(productFactory(graph.context,*childIdentity).lock().get()==&domain.factory());
                        }
                    }
                }
                auto* text=dynamic_cast<m::TextNode*>(domain.factory().create("text",tree::FactoryArgs{}));
                expect(text!=nullptr);
                if(text)expect(dynamic_cast<m::TextNode*>(text->copy())!=nullptr);
            }

            runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
            runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
            bindings->retireSourceAllocationDomain(domain);
            expect(!runtime->mmlFactoryIdentities.find(domainFactory).has_value())
                << "domain retirement removes only its exact canonical factory handle";
            expect(runtime->mmlFactoryIdentities.canonicalHandleCount()==factoryCount)
                << "repeated request domains do not accumulate canonical factory identities";
        }
        expect(owner.expired())
            << "native products and the canonical factory record release the domain after retirement";
    };

    "private_source_domain_releases_its_products_after_canonical_request_retirement"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        auto lexicalFactory=std::make_shared<m::MmlFactory>();
        const auto factoryIdentity=runtime->bindMmlFactory(lexicalFactory);
        const auto text=textConstructor(graph,*runtime,factoryIdentity);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;
        auto& ambient=bindings->sourceAllocator().backend();
        const auto ambientBefore=ambient.productCountForDiagnostics();
        const auto ambientReceiver=constructText(graph,text,factoryIdentity);
        auto* const ambientProduct=&runtime->resolveMmlNode(ambientReceiver);
        expect(productFactory(graph.context,ambientReceiver).lock().get()==&ambient);
        const auto ambientProducts=ambient.productCountForDiagnostics();
        expect(ambientProducts==ambientBefore+1u)
            << "direct Source construction remains owned by the generic Context backend";

        for(std::size_t request=0;request<3;++request){
            std::weak_ptr<m::MmlFactory> owner;
            {
                auto domain=bindings->makePrivateSourceAllocationDomain();
                bindings->bindSourceAllocationDomain(domain);
                const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
                const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
                o::ObjectRef receiver;
                {
                    auto active=bindings->activateSourceAllocation(domain);
                    receiver=constructText(graph,text,factoryIdentity);
                }
                (void)runtime->resolveMmlNode(receiver);
                owner=productFactory(graph.context,receiver);
                expect(owner.lock().get()==&domain.factory());
                expect(domain.factory().productCountForDiagnostics()==1_u);
                expect(ambient.productCountForDiagnostics()==ambientProducts);
                runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
                runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
                expect(throws<o::Exception>([&]{(void)graph.context.kind(receiver);}));
                bindings->retireSourceAllocationDomain(domain);
            }
            expect(owner.expired())
                << "request product owner dies only after identity/array retirement";
            expect(ambient.productCountForDiagnostics()==ambientProducts);
        }
        expect(&runtime->resolveMmlNode(ambientReceiver)==ambientProduct)
            << "private request retirement never releases an ambient Source product";
    };

    "lexical_activation_restores_outer_domain_after_nested_failure"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        auto lexicalFactory=std::make_shared<m::MmlFactory>();
        const auto factoryIdentity=runtime->bindMmlFactory(lexicalFactory);
        const auto text=textConstructor(graph,*runtime,factoryIdentity);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;
        auto outer=bindings->makePrivateSourceAllocationDomain();
        auto inner=bindings->makePrivateSourceAllocationDomain();
        bindings->bindSourceAllocationDomain(outer);
        bindings->bindSourceAllocationDomain(inner);
        const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
        const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
        o::ObjectRef first{},second{},third{};
        {
            auto activeOuter=bindings->activateSourceAllocation(outer);
            first=constructText(graph,text,factoryIdentity);
            try{
                auto activeInner=bindings->activateSourceAllocation(inner);
                second=constructText(graph,text,factoryIdentity);
                const std::array<o::Value,1> retryArgs{o::Value(factoryIdentity)};
                expect(graph.context.call(text,o::Value(first),retryArgs)==o::Value(first));
                expect(productFactory(graph.context,first).lock().get()==&outer.factory())
                    << "retry keeps the original product owner despite a different active domain";
                throw std::runtime_error("leave nested Source call");
            }catch(const std::runtime_error&){}
            third=constructText(graph,text,factoryIdentity);
        }
        expect(productFactory(graph.context,first).lock().get()==&outer.factory());
        expect(productFactory(graph.context,second).lock().get()==&inner.factory());
        expect(productFactory(graph.context,third).lock().get()==&outer.factory());
        expect(outer.factory().productCountForDiagnostics()==2_u);
        expect(inner.factory().productCountForDiagnostics()==1_u);
        runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
        runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
        bindings->retireSourceAllocationDomain(inner);
        bindings->retireSourceAllocationDomain(outer);
    };

    "public_renderer_reuses_session_and_detached_scene_after_request_cleanup"_test=[] {
        std::optional<folia::math::output::scene::Scene> detached;
        {
            folia::math::Renderer renderer;
            for(std::size_t i=0;i<3;++i){
                auto rendered=renderer.renderScene(uR"(\mathtip{x+1}{y^2})");
                expect(!rendered.roots().empty());
                if(!detached)detached.emplace(std::move(rendered));
                expect(!renderer.render(u"x+1").svg.empty());
            }
            std::stop_source cancelled;
            cancelled.request_stop();
            expect(throws<folia::math::RenderError>([&]{
                (void)renderer.renderScene(std::u16string_view(u"x"),cancelled.get_token(),{});
            }));
            // Pinned Source 4.1.3 raises RangeError before tooltip output for
            // this public TeX input; the same Renderer must recover afterward.
            expect(throws<folia::math::RenderError>([&]{
                (void)renderer.renderScene(uR"(\mathtip{x}{\unicode{x110000}})");
            }));
            expect(!renderer.renderScene(u"x").roots().empty());
        }
        expect(detached.has_value()&&!detached->roots().empty())
            << "frozen Scene geometry survives request factory and Renderer retirement";
    };

    "private_source_constructor_retires_exact_base_properties_object_without_touching_nested_aliases"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);if(!bindings)return;
        const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
        const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        auto domain=bindings->makePrivateSourceAllocationDomain();
        bindings->bindSourceAllocationDomain(domain);
        const auto owner=domain.factory().weak_from_this().lock();
        expect(owner!=nullptr);if(!owner)return;
        const auto factory=bindings->bindFactory(owner);
        const auto constructor=textConstructor(graph,*runtime,factory);
        const std::array<o::Value,1> args{o::Value(factory)};
        o::ObjectRef receiver{};
        {
            auto active=bindings->activateSourceAllocation(domain);
            receiver=graph.context.construct(constructor,args);
        }
        const auto properties=std::get<o::ObjectRef>(
            graph.context.get(o::Value(receiver),o::Key(u"properties")).data);
        const auto nested=graph.context.object();
        const auto parent=graph.context.object();
        graph.context.set(o::Value(properties),o::Key(u"nested"),o::Value(nested));
        graph.context.setPrototype(properties,parent);
        auto getterCalls=std::make_shared<std::size_t>(0);
        const auto getter=graph.context.function(u"get probe",[getterCalls](o::Context&,o::Value,std::span<const o::Value>){
            ++*getterCalls;return o::Value(1);
        });
        graph.context.define(nested,o::Key(u"probe"),o::Descriptor::access(getter,std::nullopt,true,true));
        const auto& node=runtime->resolveMmlNode(receiver);
        const auto* nativeCell=dynamic_cast<const tree::AbstractNode<m::MmlNode,m::MmlNode>*>(&node);
        expect(nativeCell!=nullptr);if(!nativeCell)return;
        const auto sourceCell=nativeCell->sourcePropertiesCell();
        const auto* storage=std::get_if<std::shared_ptr<folia::math::PropertyStorage>>(&sourceCell.value);
        expect(storage&&*storage&&(*storage)->binding&&
               (*storage)->binding->sourceObjectIdentity()==std::optional<o::ObjectRef>{properties})
            << "the native cell keeps the exact ObjectRef created by the Source base constructor";

        runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
        runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
        bindings->retirePrivateRendererProjectionCapture(capture);
        bindings->retireSourceAllocationDomain(domain);
        expect(throws<o::Exception>([&]{(void)graph.context.kind(properties);}))
            << "request cleanup retires the exact initial Source object record";
        expect(graph.context.kind(nested)==o::ObjectKind::object);
        expect(graph.context.kind(parent)==o::ObjectKind::object);
        expect(*getterCalls==0u);
    };

    "private_source_construct_capture_uses_active_backend_with_ambient_lexical_factory"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);if(!bindings)return;
        auto ambient=std::make_shared<m::MmlFactory>();
        const auto ambientIdentity=runtime->bindMmlFactory(ambient);
        const auto constructor=textConstructor(graph,*runtime,ambientIdentity);
        const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
        const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        auto domain=bindings->makePrivateSourceAllocationDomain();bindings->bindSourceAllocationDomain(domain);
        const auto args=std::array<o::Value,1>{o::Value(ambientIdentity)};
        o::ObjectRef receiver{};
        {
            auto active=bindings->activateSourceAllocation(domain);
            receiver=graph.context.construct(constructor,args);
        }
        const auto initialProperties=std::get<o::ObjectRef>(
            graph.context.get(o::Value(receiver),o::Key(u"properties")).data);
        expect(graph.context.get(o::Value(receiver),o::Key(u"factory"))==o::Value(ambientIdentity))
            << "Source keeps the ambient lexical factory on the receiver";
        expect(productFactory(graph.context,receiver).lock().get()==&domain.factory())
            << "Context::construct allocates the native product from the active private backend";
        const auto* nativeCell=dynamic_cast<const tree::AbstractNode<m::MmlNode,m::MmlNode>*>(&runtime->resolveMmlNode(receiver));
        expect(nativeCell!=nullptr);if(!nativeCell)return;
        const auto cell=nativeCell->sourcePropertiesCell();
        const auto* storage=std::get_if<std::shared_ptr<folia::math::PropertyStorage>>(&cell.value);
        expect(storage&&*storage&&(*storage)->binding&&
               (*storage)->binding->sourceObjectIdentity()==std::optional<o::ObjectRef>{initialProperties})
            << "the captured birth is the exact object transferred into the canonical node";
        runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
        runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
        bindings->retirePrivateRendererProjectionCapture(capture);
        expect(throws<o::Exception>([&]{(void)graph.context.kind(initialProperties);}))
            << "the exact Source birth is retired despite the distinct lexical factory";
        bindings->retireSourceAllocationDomain(domain);
    };

    "private_source_base_properties_capture_preserves_mtable_class_field_replacement"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);if(!bindings)return;
        const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
        const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        auto domain=bindings->makePrivateSourceAllocationDomain();bindings->bindSourceAllocationDomain(domain);
        const auto owner=domain.factory().weak_from_this().lock();
        expect(owner!=nullptr);if(!owner)return;
        const auto factory=bindings->bindFactory(owner);
        const auto constructor=mmlConstructor(graph,*runtime,factory,u"mtable");
        const std::array<o::Value,1> args{o::Value(factory)};
        o::ObjectRef receiver{};
        {
            auto active=bindings->activateSourceAllocation(domain);
            receiver=graph.context.construct(constructor,args);
        }
        const auto replacement=std::get<o::ObjectRef>(
            graph.context.get(o::Value(receiver),o::Key(u"properties")).data);
        runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
        runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
        bindings->retirePrivateRendererProjectionCapture(capture);
        expect(graph.context.kind(replacement)==o::ObjectKind::object)
            << "MmlMtable::initializeClassFields replacement is outside the base birth capture";
        bindings->retireSourceAllocationDomain(domain);
    };

    "private_source_construct_skips_inherited_properties_setter_that_retains_value"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);if(!bindings)return;
        const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
        const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        auto domain=bindings->makePrivateSourceAllocationDomain();bindings->bindSourceAllocationDomain(domain);
        const auto owner=domain.factory().weak_from_this().lock();
        expect(owner!=nullptr);if(!owner)return;
        const auto factory=bindings->bindFactory(owner);
        const auto constructor=textConstructor(graph,*runtime,factory);
        const auto prototype=std::get<o::ObjectRef>(graph.context.get(o::Value(constructor),o::Key(u"prototype")).data);
        const auto saved=graph.context.object();
        auto setterCalls=std::make_shared<std::size_t>(0);
        const auto setter=graph.context.function(u"set properties",[saved,setterCalls](o::Context& cx,o::Value,std::span<const o::Value> args){
            ++*setterCalls;cx.set(o::Value(saved),o::Key(u"value"),args[0]);return o::Value(o::Undefined{});
        });
        graph.context.define(prototype,o::Key(u"properties"),o::Descriptor::access(std::nullopt,setter,true,true));
        const std::array<o::Value,1> args{o::Value(factory)};
        {
            auto active=bindings->activateSourceAllocation(domain);
            (void)graph.context.construct(constructor,args);
        }
        const auto retained=std::get<o::ObjectRef>(graph.context.get(o::Value(saved),o::Key(u"value")).data);
        runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
        runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
        bindings->retirePrivateRendererProjectionCapture(capture);
        expect(*setterCalls==1u)<<"the normal Source assignment invokes the inherited setter exactly once";
        expect(graph.context.kind(retained)==o::ObjectKind::object)
            << "construct-created receivers with a retaining properties setter are excluded from retirement";
        graph.context.erase(prototype,o::Key(u"properties"));
        bindings->retireSourceAllocationDomain(domain);
    };

    "private_source_capture_excludes_borrowed_receiver_with_a_properties_setter"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);if(!bindings)return;
        const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
        const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        auto domain=bindings->makePrivateSourceAllocationDomain();bindings->bindSourceAllocationDomain(domain);
        const auto owner=domain.factory().weak_from_this().lock();
        expect(owner!=nullptr);if(!owner)return;
        const auto factory=bindings->bindFactory(owner);
        const auto constructor=textConstructor(graph,*runtime,factory);
        const auto prototype=std::get<o::ObjectRef>(graph.context.get(o::Value(constructor),o::Key(u"prototype")).data);
        const auto saved=graph.context.object();
        const auto setter=graph.context.function(u"set properties",[saved](o::Context& cx,o::Value,std::span<const o::Value> args){
            cx.set(o::Value(saved),o::Key(u"value"),args[0]);return o::Value(o::Undefined{});
        });
        graph.context.define(prototype,o::Key(u"properties"),o::Descriptor::access(std::nullopt,setter,true,true));
        const auto receiver=graph.context.objectWithPrototype(prototype);
        const std::array<o::Value,1> args{o::Value(factory)};
        {
            auto active=bindings->activateSourceAllocation(domain);
            (void)graph.context.call(constructor,o::Value(receiver),args);
        }
        const auto retained=std::get<o::ObjectRef>(graph.context.get(o::Value(saved),o::Key(u"value")).data);
        runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
        runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
        bindings->retirePrivateRendererProjectionCapture(capture);
        expect(graph.context.kind(retained)==o::ObjectKind::object)
            << "borrowed-call setter values remain live and are not treated as private initial records";
        graph.context.erase(prototype,o::Key(u"properties"));
        bindings->retireSourceAllocationDomain(domain);
    };

    "private_source_initial_properties_orphan_before_adopt_is_retired"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);if(!bindings)return;
        const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
        const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        auto domain=bindings->makePrivateSourceAllocationDomain();bindings->bindSourceAllocationDomain(domain);
        const auto owner=domain.factory().weak_from_this().lock();
        expect(owner!=nullptr);if(!owner)return;
        const auto factory=bindings->bindFactory(owner);
        const auto constructor=textConstructor(graph,*runtime,factory);
        const auto prototype=std::get<o::ObjectRef>(graph.context.get(o::Value(constructor),o::Key(u"prototype")).data);
        const auto saved=std::make_shared<std::optional<o::ObjectRef>>();
        const auto setter=graph.context.function(u"set childNodes",[saved](o::Context& cx,o::Value receiver,std::span<const o::Value>){
            *saved=std::get<o::ObjectRef>(cx.get(receiver,o::Key(u"properties")).data);
            throw o::Exception("TypeError",u"Injected childNodes setter failure before product adoption");
            return o::Value(o::Undefined{});
        });
        graph.context.define(prototype,o::Key(u"childNodes"),o::Descriptor::access(std::nullopt,setter,true,true));
        const std::array<o::Value,1> args{o::Value(factory)};
        {
            auto active=bindings->activateSourceAllocation(domain);
            expect(throws<o::Exception>([&]{(void)graph.context.construct(constructor,args);}));
        }
        expect(saved->has_value())<<"the Source properties object was created before the later base-field failure";
        runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
        runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
        bindings->retirePrivateRendererProjectionCapture(capture);
        if(*saved)expect(throws<o::Exception>([&]{(void)graph.context.kind(**saved);}))
            << "pre-adopt orphan is retired by its request token";
        graph.context.erase(prototype,o::Key(u"childNodes"));
        bindings->retireSourceAllocationDomain(domain);
    };

    "private_source_projection_token_blocks_new_capture_after_mml_retirement_failure"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);if(!bindings)return;
        const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
        const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
        const auto capture=bindings->beginPrivateRendererProjectionCapture();
        auto domain=bindings->makePrivateSourceAllocationDomain();bindings->bindSourceAllocationDomain(domain);
        const auto owner=domain.factory().weak_from_this().lock();
        expect(owner!=nullptr);if(!owner)return;
        const auto factory=bindings->bindFactory(owner);
        const auto constructor=textConstructor(graph,*runtime,factory);
        const std::array<o::Value,1> args{o::Value(factory)};
        o::ObjectRef receiver{};
        {
            auto active=bindings->activateSourceAllocation(domain);
            receiver=graph.context.construct(constructor,args);
        }
        const auto properties=std::get<o::ObjectRef>(
            graph.context.get(o::Value(receiver),o::Key(u"properties")).data);
        graph.context.retireObjectRecord(properties);
        runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
        runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
        expect(throws<o::Exception>([&]{bindings->retirePrivateRendererProjectionCapture(capture);}));
        expect(throws<o::Exception>([&]{(void)bindings->beginPrivateRendererProjectionCapture();}))
            << "an unclosed token blocks a new capture even after Attributes completed";
        expect(throws<o::Exception>([&]{bindings->retirePrivateRendererProjectionCapture(capture);}))
            << "a tombstoned Context object cannot be fabricated back into a successful retry";
        bindings->retireSourceAllocationDomain(domain);
    };
};

namespace {
enum class DomainOwnerProbe {
    bind_only,
    factory_node,
    factory_node_mfrac,
    factory_node_map,
    direct_mfrac,
    missing_child_inheritance,
    text_copy,
};
struct DomainOwnerProbeResult {
    long useCountAfterRetirement=0;
    bool expiredAfterScope=false;
    std::size_t nodeCountBefore=0,nodeCountAfter=0;
    std::size_t arrayCountBefore=0,arrayCountAfter=0;
    std::size_t factoryCountBefore=0,factoryCountAfter=0;
};

DomainOwnerProbeResult runDomainOwnerProbe(DomainOwnerProbe probe){
    SharedGraph graph;
    auto runtime=graph.makeRuntime();
    runtime->initializeParserRuntime();
    (void)t::BasePackage::initializeModule(*runtime);
    const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
    if(!bindings)throw o::Exception("TypeError",u"probe has no canonical MML Context bindings");

    DomainOwnerProbeResult result;
    result.nodeCountBefore=runtime->mmlNodeIdentities.canonicalHandleCount();
    result.arrayCountBefore=runtime->nodeArrayIdentities.canonicalHandleCount();
    result.factoryCountBefore=runtime->mmlFactoryIdentities.canonicalHandleCount();
    std::weak_ptr<m::MmlFactory> owner;
    {
        const auto nodes=runtime->mmlNodeIdentities.beginRequestCapture();
        const auto arrays=runtime->nodeArrayIdentities.beginRequestCapture();
        {
            auto domain=bindings->makePrivateSourceAllocationDomain();
            bindings->bindSourceAllocationDomain(domain);
            auto* const factory=&domain.factory();
            owner=factory->weak_from_this();
            const auto identity=runtime->mmlFactoryIdentities.find(factory);
            if(!identity)throw o::Exception("TypeError",u"probe factory has no canonical identity");

            auto readFactoryNode=[&]{
                const auto value=graph.context.get(o::Value(*identity),o::Key(u"node"));
                const auto* object=std::get_if<o::ObjectRef>(&value.data);
                if(!object)throw o::Exception("TypeError",u"factory.node is not a Context object");
                return *object;
            };
            switch(probe){
            case DomainOwnerProbe::bind_only:
                break;
            case DomainOwnerProbe::factory_node:{
                const auto node=readFactoryNode();(void)node;
                break;
            }
            case DomainOwnerProbe::factory_node_mfrac:{
                const auto node=readFactoryNode();
                const auto creator=graph.context.get(o::Value(node),o::Key(u"mfrac"));
                if(!std::get_if<o::ObjectRef>(&creator.data))
                    throw o::Exception("TypeError",u"factory.node.mfrac is not a Context function");
                break;
            }
            case DomainOwnerProbe::factory_node_map:{
                const auto value=graph.context.get(o::Value(*identity),o::Key(u"nodeMap"));
                if(!std::get_if<o::ObjectRef>(&value.data))
                    throw o::Exception("TypeError",u"factory.nodeMap is not a Context Map");
                break;
            }
            case DomainOwnerProbe::direct_mfrac:{
                auto active=bindings->activateSourceAllocation(domain);
                auto* product=dynamic_cast<m::AbstractMmlNode*>(
                    domain.factory().create("mfrac",tree::FactoryArgs{}));
                if(!product||!runtime->mmlNodeIdentities.find(product))
                    throw o::Exception("TypeError",u"direct mfrac has no canonical node identity");
                break;
            }
            case DomainOwnerProbe::missing_child_inheritance:{
                auto active=bindings->activateSourceAllocation(domain);
                auto* product=dynamic_cast<m::AbstractMmlNode*>(
                    domain.factory().create("mfrac",tree::FactoryArgs{}));
                if(!product)throw o::Exception("TypeError",u"direct mfrac creation failed");
                product->setInheritedAttributes(folia::math::PropertyList{},false,0,false);
                const auto children=product->childArrayOwner();
                if(!children||children->size()!=2)
                    throw o::Exception("TypeError",u"mfrac did not create two missing children");
                for(std::size_t i=0;i<2;++i){
                    auto* child=children->get(i).nodeOrNull();
                    if(!child||child->kind()!="mrow"||!runtime->mmlNodeIdentities.find(child))
                        throw o::Exception("TypeError",u"missing child is not a canonical native mrow");
                }
                break;
            }
            case DomainOwnerProbe::text_copy:{
                auto active=bindings->activateSourceAllocation(domain);
                auto* text=dynamic_cast<m::TextNode*>(domain.factory().create("text",tree::FactoryArgs{}));
                if(!text)throw o::Exception("TypeError",u"direct TextNode creation failed");
                auto* copy=dynamic_cast<m::TextNode*>(text->copy());
                if(!copy||!runtime->mmlNodeIdentities.find(copy))
                    throw o::Exception("TypeError",u"TextNode copy has no canonical node identity");
                break;
            }
            }

            runtime->mmlNodeIdentities.retireCapturedRequestBindings(nodes);
            runtime->nodeArrayIdentities.retireCapturedRequestBindings(arrays);
            bindings->retireSourceAllocationDomain(domain);
            result.useCountAfterRetirement=owner.use_count();
            result.nodeCountAfter=runtime->mmlNodeIdentities.canonicalHandleCount();
            result.arrayCountAfter=runtime->nodeArrayIdentities.canonicalHandleCount();
            result.factoryCountAfter=runtime->mmlFactoryIdentities.canonicalHandleCount();
        }
        result.expiredAfterScope=owner.expired();
    }
    return result;
}

suite mmlPrivateDomainOwnerRetentionAblationG242=[] {
    const auto verify=[](DomainOwnerProbe probe){
        const auto result=runDomainOwnerProbe(probe);
        expect(result.useCountAfterRetirement==1)
            << "post-retirement weak use_count must contain only the still-live domain handle; observed "
            << result.useCountAfterRetirement;
        expect(result.expiredAfterScope)
            << "private factory must expire immediately after its domain scope ends";
        expect(result.nodeCountAfter==result.nodeCountBefore)
            << "probe retires every request node identity";
        expect(result.arrayCountAfter==result.arrayCountBefore)
            << "probe retires every request child-array identity";
        expect(result.factoryCountAfter==result.factoryCountBefore)
            << "probe retires exactly its request factory identity";
    };
    "g242_private_domain_bind_only_owner_ablation"_test=[verify]{verify(DomainOwnerProbe::bind_only);};
    "g242_private_domain_factory_node_only_owner_ablation"_test=[verify]{verify(DomainOwnerProbe::factory_node);};
    "g242_private_domain_node_mfrac_materialization_owner_ablation"_test=[verify]{verify(DomainOwnerProbe::factory_node_mfrac);};
    "g242_private_domain_node_map_getter_owner_ablation"_test=[verify]{verify(DomainOwnerProbe::factory_node_map);};
    "g242_private_domain_direct_mfrac_owner_ablation"_test=[verify]{verify(DomainOwnerProbe::direct_mfrac);};
    "g242_private_domain_missing_child_inheritance_owner_ablation"_test=[verify]{verify(DomainOwnerProbe::missing_child_inheritance);};
    "g242_private_domain_text_copy_owner_ablation"_test=[verify]{verify(DomainOwnerProbe::text_copy);};
};
}
