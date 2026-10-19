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
    "private_renderer_attributes_projection_failure_preserves_post_identity_retry_progress"_test=[] {
        SharedGraph graph;
        auto runtime=graph.makeRuntime();
        runtime->initializeParserRuntime();
        (void)t::BasePackage::initializeModule(*runtime);
        const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
        expect(bindings!=nullptr);
        if(!bindings)return;

        const auto countBefore=runtime->attributeBindings.canonicalHandleCountForDiagnostics();
        const auto capture=bindings->beginPrivateRendererAttributesCapture();
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
        try{bindings->retirePrivateRendererAttributesCapture(capture);}
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
        try{bindings->retirePrivateRendererAttributesCapture(capture);}
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

        const auto capture=bindings->beginPrivateRendererAttributesCapture();
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

        bindings->retirePrivateRendererAttributesCapture(capture);
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

        const auto lazyCapture=bindings->beginPrivateRendererAttributesCapture();
        auto lazyOwner=std::make_shared<folia::math::Attributes>(folia::math::PropertyList{},folia::math::PropertyList{});
        const auto lazyIdentity=runtime->attributeBindings.bind(lazyOwner);
        const std::array<std::weak_ptr<folia::math::PropertyStorage>,3> lazyStorages{
            std::get<std::shared_ptr<folia::math::PropertyStorage>>(folia::math::Property(lazyOwner->defaultsLayer()).value),
            std::get<std::shared_ptr<folia::math::PropertyStorage>>(folia::math::Property(lazyOwner->inheritedLayer()).value),
            std::get<std::shared_ptr<folia::math::PropertyStorage>>(folia::math::Property(lazyOwner->attributesLayer()).value)};
        lazyOwner.reset();
        bindings->retirePrivateRendererAttributesCapture(lazyCapture);
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

        const auto capture=bindings->beginPrivateRendererAttributesCapture();
        auto owner=std::make_shared<folia::math::Attributes>(folia::math::PropertyList{},folia::math::PropertyList{});
        const auto identity=runtime->attributeBindings.bind(owner);
        const std::weak_ptr<folia::math::PropertyStorage> initialDefaults=
            std::get<std::shared_ptr<folia::math::PropertyStorage>>(folia::math::Property(owner->defaultsLayer()).value);
        const auto sourceReplacement=graph.context.object();
        graph.context.set(o::Value(identity),o::Key(u"defaults"),o::Value(sourceReplacement));
        const auto replacementProjection=graph.context.get(o::Value(identity),o::Key(u"defaults"));
        expect(std::get_if<o::ObjectRef>(&replacementProjection.data)!=nullptr);
        bindings->retirePrivateRendererAttributesCapture(capture);
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
        expect(throws<o::Exception>([&]{bindings->retirePrivateRendererAttributesCapture(foreignToken);}));
        foreign.retireCapturedRequestBindings(foreignToken);

        const auto capture=bindings->beginPrivateRendererAttributesCapture();
        auto owner=std::make_shared<folia::math::Attributes>(folia::math::PropertyList{},folia::math::PropertyList{});
        std::weak_ptr<folia::math::Attributes> weakOwner=owner;
        const auto identity=runtime->attributeBindings.bind(owner);
        std::any originalPayload=graph.context.payload(identity);
        graph.context.setPayload(identity,std::string("temporary invalid payload"));
        expect(throws<o::Exception>([&]{bindings->retirePrivateRendererAttributesCapture(capture);}));
        graph.context.setPayload(identity,std::move(originalPayload));
        bindings->retirePrivateRendererAttributesCapture(capture);
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
