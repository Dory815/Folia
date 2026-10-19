#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.stack_property;
import folia.math.core.mml_tree.mml_nodes.mtext;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.munderover;

namespace {
using namespace boost::ut;
namespace t=folia::math::tex;
namespace o=folia::math::options;
namespace m=folia::math::mml;
namespace tree=folia::math::tree;

struct PrivateGraph {
    o::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::regexp_host::RegExpBindings regexp{context};
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input{context};
    folia::math::core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context,findMath,regexp};
    std::unique_ptr<t::ParserRuntime> makeRuntime(folia::math::util::RuntimeModules* modules=nullptr){
        return std::make_unique<t::ParserRuntime>(context,functions,regexp,unicode,input,findTeX,modules);
    }
};

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

auto constructorFor(o::Context& context,t::ParserRuntime& runtime,o::ObjectRef factory,
                    std::u16string_view kind){
    const auto map=std::get<o::ObjectRef>(context.get(o::Value(factory),o::Key(u"nodeMap")).data);
    return std::get<o::ObjectRef>(runtime.maps.resolve(map)->get(o::Value(std::u16string(kind))).data);
}

suite mmlContextAllocatorOwnership = [] {
  "mml_context_registry_and_source_constructor_survive_one_parser_retiring"_test = [] {
    SharedGraph graph;
    auto first=graph.makeRuntime();
    auto later=graph.makeRuntime();
    auto firstFactory=std::make_shared<m::MmlFactory>();
    auto laterFactory=std::make_shared<m::MmlFactory>();
    const auto firstIdentity=first->bindMmlFactory(firstFactory);
    const auto laterIdentity=later->bindMmlFactory(laterFactory);
    const auto firstText=constructorFor(graph.context,*first,firstIdentity,u"text");
    const auto laterText=constructorFor(graph.context,*later,laterIdentity,u"text");
    expect(firstText==laterText);
    const auto oldHandle=first->mmlNodeIdentities.canonicalHandleCount();
    first.reset();
    const auto prototype=std::get<o::ObjectRef>(graph.context.get(o::Value(laterText),o::Key(u"prototype")).data);
    const auto receiver=graph.context.objectWithPrototype(prototype);
    const std::array<o::Value,1> args{o::Value(laterIdentity)};
    const auto returned=graph.context.call(laterText,o::Value(receiver),args);
    expect(returned==o::Value(receiver));
    expect(later->mmlNodeIdentities.canonicalHandleCount()==oldHandle+1u);
    auto* node=dynamic_cast<m::TextNode*>(&later->resolveMmlNode(receiver));
    expect(node!=nullptr);
    if(node)expect(&node->factoryOwner()==laterFactory.get());
  };

  "mml_source_constructor_keeps_foreign_factory_value_on_exact_receiver"_test = [] {
    SharedGraph graph;
    auto runtime=graph.makeRuntime();
    auto allocatorFactory=std::make_shared<m::MmlFactory>();
    const auto allocatorIdentity=runtime->bindMmlFactory(allocatorFactory);
    const auto text=constructorFor(graph.context,*runtime,allocatorIdentity,u"text");
    const auto prototype=std::get<o::ObjectRef>(graph.context.get(o::Value(text),o::Key(u"prototype")).data);
    const auto receiver=graph.context.objectWithPrototype(prototype);
    const auto foreignFactory=graph.context.object();
    std::size_t getterCalls=0,setterCalls=0;
    o::Value written=o::Undefined{};
    const auto getter=graph.context.function(u"getFactory",[&](o::Context&,o::Value self,std::span<const o::Value>){
        expect(self==o::Value(receiver));++getterCalls;return written;
    });
    const auto setter=graph.context.function(u"setFactory",[&](o::Context&,o::Value self,std::span<const o::Value> args){
        expect(self==o::Value(receiver));++setterCalls;written=args.empty()?o::Value(o::Undefined{}):args.front();return o::Value(o::Undefined{});
    });
    graph.context.define(receiver,o::Key(u"factory"),o::Descriptor::access(getter,setter,true,true));
    const std::array<o::Value,1> args{o::Value(foreignFactory)};
    const auto result=graph.context.call(text,o::Value(receiver),args);
    expect(result==o::Value(receiver));
    expect(setterCalls==1u);
    expect(getterCalls>0u);
    expect(written==o::Value(foreignFactory));
    auto* node=dynamic_cast<m::TextNode*>(&runtime->resolveMmlNode(receiver));
    expect(node!=nullptr);
    expect(graph.context.get(o::Value(receiver),o::Key(u"factory"))==o::Value(foreignFactory));
    const auto bindings=runtime->modules().find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
    expect(bindings!=nullptr);
    if(bindings){
        const auto* binding=std::any_cast<folia::math::NativeBindingPayload<m::MmlNode*>>(
            &graph.context.payload(receiver));
        expect(binding!=nullptr&&binding->identity==node);
        expect(binding&&binding->lifetime.get()==static_cast<void*>(&bindings->sourceAllocator().backend()));
    }
  };

  "mml_foreign_factory_retry_reuses_the_exact_receiver_product"_test = [] {
    SharedGraph graph;
    auto runtime=graph.makeRuntime();
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=runtime->bindMmlFactory(factory);
    const auto mrow=constructorFor(graph.context,*runtime,factoryIdentity,u"mrow");
    const auto prototype=std::get<o::ObjectRef>(graph.context.get(o::Value(mrow),o::Key(u"prototype")).data);
    const auto receiver=graph.context.objectWithPrototype(prototype);
    const auto foreign=graph.context.object();
    const auto originalGet=graph.context.get(o::Value(factoryIdentity),o::Key(u"getNodeClass"));
    std::size_t getClassCalls=0,receiverWrites=0;
    o::Value rawFactory=o::Undefined{};
    const auto getClass=graph.context.function(u"getNodeClass",[&](o::Context& context,o::Value,
            std::span<const o::Value> args){
        ++getClassCalls;
        if(getClassCalls==1u)throw o::Exception("TypeError",u"one-shot foreign factory failure");
        return context.call(std::get<o::ObjectRef>(originalGet.data),o::Value(factoryIdentity),args);
    });
    const auto originalCreate=graph.context.get(o::Value(factoryIdentity),o::Key(u"create"));
    const auto create=graph.context.function(u"create",[&](o::Context& context,o::Value,
            std::span<const o::Value> args){
        return context.call(std::get<o::ObjectRef>(originalCreate.data),o::Value(factoryIdentity),args);
    });
    graph.context.define(foreign,o::Key(u"getNodeClass"),o::Descriptor::data(o::Value(getClass)));
    graph.context.define(foreign,o::Key(u"create"),o::Descriptor::data(o::Value(create)));
    const auto factorySetter=graph.context.function(u"setFactory",[&](o::Context&,o::Value self,
            std::span<const o::Value> args){
        expect(self==o::Value(receiver));++receiverWrites;
        rawFactory=args.empty()?o::Value(o::Undefined{}):args.front();
        return o::Value(o::Undefined{});
    });
    const auto factoryGetter=graph.context.function(u"getFactory",[&](o::Context&,o::Value self,
            std::span<const o::Value>){expect(self==o::Value(receiver));return rawFactory;});
    graph.context.define(receiver,o::Key(u"factory"),o::Descriptor::access(factoryGetter,factorySetter,true,true));
    const std::array<o::Value,3> args{o::Value(foreign),o::Value(o::Undefined{}),o::Value(o::Undefined{})};
    const auto before=runtime->mmlNodeIdentities.canonicalHandleCount();
    const auto bindings=runtime->modules().find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
    expect(bindings!=nullptr);
    if(!bindings)return;
    const auto productsBefore=bindings->sourceAllocator().backend().productCountForDiagnostics();
    bool firstFailed=false;
    try{(void)graph.context.call(mrow,o::Value(receiver),args);}
    catch(const o::Exception&){firstFailed=true;}
    expect(firstFailed);
    const auto afterFailure=runtime->mmlNodeIdentities.canonicalHandleCount();
    expect(afterFailure==before+1u);
    auto* const failedProduct=&runtime->resolveMmlNode(receiver);
    expect(bindings->sourceAllocator().backend().productCountForDiagnostics()==productsBefore+1u);
    expect(rawFactory==o::Value(foreign));
    expect(receiverWrites==1u);
    const auto partialChildren=graph.context.get(o::Value(receiver),o::Key(u"childNodes"));
    const auto* partialArray=std::get_if<o::ObjectRef>(&partialChildren.data);
    expect(partialArray!=nullptr);
    const auto result=graph.context.call(mrow,o::Value(receiver),args);
    expect(result==o::Value(receiver));
    expect(runtime->mmlNodeIdentities.canonicalHandleCount()==afterFailure);
    expect(&runtime->resolveMmlNode(receiver)==failedProduct);
    expect(bindings->sourceAllocator().backend().productCountForDiagnostics()==productsBefore+1u);
    expect(rawFactory==o::Value(foreign));
    expect(graph.context.get(o::Value(receiver),o::Key(u"childNodes"))==partialChildren);
    auto* node=dynamic_cast<m::AbstractMmlNode*>(&runtime->resolveMmlNode(receiver));
    expect(node!=nullptr);
  };
  "mml_source_constructor_call_reuses_existing_native_product_owner"_test = [] {
    SharedGraph graph;
    auto runtime=graph.makeRuntime();
    auto allocationFactory=std::make_shared<m::MmlFactory>();
    auto lexicalFactory=std::make_shared<m::MmlFactory>();
    // Create before binding so this product keeps a native FactoryRef owned by
    // allocationFactory instead of entering the Source allocator backend.
    auto* const node=dynamic_cast<m::TextNode*>(
        allocationFactory->create("text",tree::FactoryArgs{}));
    expect(node!=nullptr);
    if(!node)return;
    expect(&node->factoryOwner()==allocationFactory.get());
    (void)runtime->bindMmlFactory(allocationFactory);
    const auto lexicalIdentity=runtime->bindMmlFactory(lexicalFactory);
    const auto text=constructorFor(graph.context,*runtime,lexicalIdentity,u"text");
    const auto prototype=std::get<o::ObjectRef>(graph.context.get(o::Value(text),o::Key(u"prototype")).data);
    const auto receiver=graph.context.objectWithPrototype(prototype);
    const auto bindings=runtime->modules().find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
    expect(bindings!=nullptr);
    if(!bindings)return;
    (void)bindings->bindNode(receiver,*node,allocationFactory);
    expect(runtime->mmlNodeIdentities.find(node)==std::optional<o::ObjectRef>(receiver));
    const auto* beforeBinding=std::any_cast<folia::math::NativeBindingPayload<m::MmlNode*>>(
        &graph.context.payload(receiver));
    expect(beforeBinding&&beforeBinding->lifetime.get()==static_cast<void*>(allocationFactory.get()));
    const std::array<o::Value,1> args{o::Value(lexicalIdentity)};
    const auto backendProducts=bindings->sourceAllocator().backend().productCountForDiagnostics();
    const auto allocationProducts=allocationFactory->productCountForDiagnostics();
    const auto lexicalProducts=lexicalFactory->productCountForDiagnostics();
    const auto returned=graph.context.call(text,o::Value(receiver),args);
    expect(returned==o::Value(receiver));
    expect(runtime->mmlNodeIdentities.find(node)==std::optional<o::ObjectRef>(receiver));
    expect(&runtime->resolveMmlNode(receiver)==node);
    expect(&node->factoryOwner()==lexicalFactory.get());
    const auto* afterBinding=std::any_cast<folia::math::NativeBindingPayload<m::MmlNode*>>(
        &graph.context.payload(receiver));
    expect(afterBinding&&afterBinding->identity==node);
    expect(afterBinding&&afterBinding->lifetime.get()==static_cast<void*>(allocationFactory.get()));
    expect(bindings->sourceAllocator().backend().productCountForDiagnostics()==backendProducts);
    expect(allocationFactory->productCountForDiagnostics()==allocationProducts);
    expect(lexicalFactory->productCountForDiagnostics()==lexicalProducts);
  };

  "mml_native_factory_create_keeps_actual_factory_as_product_owner"_test = [] {
    SharedGraph graph;
    auto runtime=graph.makeRuntime();
    auto factory=std::make_shared<m::MmlFactory>();
    (void)runtime->bindMmlFactory(factory);
    const auto before=runtime->mmlNodeIdentities.canonicalHandleCount();
    auto* node=dynamic_cast<m::TextNode*>(factory->create("text",tree::FactoryArgs{}));
    expect(node!=nullptr);
    expect(runtime->mmlNodeIdentities.canonicalHandleCount()==before+1u);
    if(node){
        expect(&node->factoryOwner()==factory.get());
        const auto identity=runtime->mmlNodeIdentities.find(node);
        expect(identity.has_value());
        if(identity)expect(runtime->mmlNodeIdentities.resolve(*identity)==node);
    }
  };

  "mml_private_runtime_modules_expire_saved_constructor_callbacks"_test = [] {
    PrivateGraph graph;
    auto runtime=graph.makeRuntime();
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=runtime->bindMmlFactory(factory);
    const auto text=constructorFor(graph.context,*runtime,factoryIdentity,u"text");
    runtime.reset();
    const auto prototype=std::get<o::ObjectRef>(graph.context.get(o::Value(text),o::Key(u"prototype")).data);
    const auto receiver=graph.context.objectWithPrototype(prototype);
    const std::array<o::Value,1> args{o::Value(factoryIdentity)};
    expect(throws<o::Exception>([&]{(void)graph.context.call(text,o::Value(receiver),args);}));
  };

  "mml_explicit_runtime_modules_facade_can_retire_before_parser_without_dangling"_test = [] {
    PrivateGraph graph;
    auto modules=std::make_unique<folia::math::util::RuntimeModules>(graph.context);
    auto runtime=graph.makeRuntime(modules.get());
    auto factory=std::make_shared<m::MmlFactory>();
    const auto identity=runtime->bindMmlFactory(factory);
    const auto text=constructorFor(graph.context,*runtime,identity,u"text");
    modules.reset();
    const std::array<o::Value,1> args{o::Value(identity)};
    const auto receiver=graph.context.construct(text,args);
    expect(&runtime->resolveMmlNode(receiver)==runtime->mmlNodeIdentities.resolve(receiver));
    runtime.reset();
    expect(throws<o::Exception>([&]{(void)graph.context.construct(text,args);}));
  };

  "mml_explicit_runtime_modules_facade_keeps_services_alive_after_parser_retirement"_test = [] {
    PrivateGraph graph;
    auto modules=std::make_unique<folia::math::util::RuntimeModules>(graph.context);
    auto runtime=graph.makeRuntime(modules.get());
    auto factory=std::make_shared<m::MmlFactory>();
    const auto identity=runtime->bindMmlFactory(factory);
    const auto text=constructorFor(graph.context,*runtime,identity,u"text");
    runtime.reset();
    const std::array<o::Value,1> args{o::Value(identity)};
    const auto receiver=graph.context.construct(text,args);
    expect(graph.context.kind(receiver)==o::ObjectKind::instance);
    modules.reset();
    expect(throws<o::Exception>([&]{(void)graph.context.construct(text,args);}));
  };

  "mml_native_factory_can_create_without_a_context_module"_test = [] {
    auto factory=std::make_shared<m::MmlFactory>();
    auto* node=dynamic_cast<m::TextNode*>(factory->create("text",tree::FactoryArgs{}));
    expect(node!=nullptr);
    if(node)expect(&node->factoryOwner()==factory.get());
  };
  "mml_factory_binding_order_does_not_change_shared_constructor_identity"_test = [] {
    SharedGraph graph;
    auto first=graph.makeRuntime();
    auto second=graph.makeRuntime();
    auto a=std::make_shared<m::MmlFactory>();
    auto b=std::make_shared<m::MmlFactory>();
    const auto bIdentity=second->bindMmlFactory(b);
    const auto bText=constructorFor(graph.context,*second,bIdentity,u"text");
    const auto aIdentity=first->bindMmlFactory(a);
    const auto aText=constructorFor(graph.context,*first,aIdentity,u"text");
    expect(aText==bText);
  };

  "mml_default_parser_runtimes_share_context_registry_and_constructor_owner"_test = [] {
    PrivateGraph graph;
    auto first=graph.makeRuntime();
    auto second=graph.makeRuntime();
    expect(&first->mmlNodeIdentities==&second->mmlNodeIdentities);
    expect(&first->mmlFactoryIdentities==&second->mmlFactoryIdentities);
    const auto firstBindings=first->modules().find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
    const auto secondBindings=second->modules().find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
    expect(firstBindings!=nullptr&&firstBindings==secondBindings);
    auto a=std::make_shared<m::MmlFactory>();
    auto b=std::make_shared<m::MmlFactory>();
    const auto aIdentity=first->bindMmlFactory(a);
    const auto bIdentity=second->bindMmlFactory(b);
    const auto aText=constructorFor(graph.context,*first,aIdentity,u"text");
    const auto bText=constructorFor(graph.context,*second,bIdentity,u"text");
    expect(aText==bText);
    first.reset();
    const std::array<o::Value,1> args{o::Value(bIdentity)};
    const auto receiver=graph.context.construct(aText,args);
    expect(&second->resolveMmlNode(receiver)==second->mmlNodeIdentities.resolve(receiver));
  };

  "mml_context_composition_rolls_back_attachment_and_retries_after_cached_module_failure"_test = [] {
    SharedGraph graph;
    const auto nodeModule=graph.modules.getOrCreate<m::MmlNodeContextModule>(
        u"core/MmlTree/MmlNode.js",[&]{return std::make_shared<m::MmlNodeContextModule>(graph.context);});
    const auto maps=graph.modules.getOrCreate<o::MapContextBindings>(
        u"ECMAScript/Map",[&]{return std::make_shared<o::MapContextBindings>(graph.context);});
    (void)graph.modules.getOrCreate<int>(u"core/Tree/Factory",[]{return std::make_shared<int>(1);});
    expect(throws<o::Exception>([&]{
        (void)graph.modules.getOrCreate<m::MmlContextBindings>(u"core/MmlTree/ContextBindings",[&]{
            return m::MmlContextBindings::create(graph.context,graph.modules,nodeModule,maps);
        });
    }));
    graph.modules.clear(u"core/Tree/Factory");
    const auto bindings=graph.modules.getOrCreate<m::MmlContextBindings>(
        u"core/MmlTree/ContextBindings",[&]{
            return m::MmlContextBindings::create(graph.context,graph.modules,nodeModule,maps);
        });
    const auto cached=graph.modules.getOrCreate<m::MmlContextBindings>(
        u"core/MmlTree/ContextBindings",[]{return std::shared_ptr<m::MmlContextBindings>{};});
    expect(bindings==cached);
    expect(&bindings->context()==&graph.context);
    auto factory=std::make_shared<m::MmlFactory>();
    const auto identity=bindings->bindFactory(factory);
    expect(graph.context.prototype(identity)==std::optional<o::ObjectRef>(bindings->factoryMethods().prototype()));
  };

  "tex_request_owner_later_identity_retirement_failure_retries_remaining_stage"_test = [] {
    SharedGraph graph;
    auto runtime=graph.makeRuntime();
    runtime->initializeParserRuntime();
    (void)t::BasePackage::initializeModule(*runtime);
    auto tex=t::TeX::make(*runtime);
    auto storage=std::make_shared<t::StackPropertyStorage>();
    const auto constructor=graph.context.constructor(u"request stack property");
    const auto object=graph.context.instance(constructor); // Older Context record is not request-transient.
    tex->beginRequestOwnerScope();
    const auto captured=runtime->stackPropertyIdentities.bind(storage.get(),object,storage);
    const auto original=std::any_cast<folia::math::NativeBindingPayload<t::StackPropertyStorage*>>(
        graph.context.payload(captured));
    graph.context.setPayload(captured,std::string("one-shot payload mismatch"));
    expect(throws<o::Exception>([&]{tex->releaseRequestReferences();}));
    expect(runtime->stackPropertyIdentities.ownsHandle(captured));
    graph.context.setPayload(captured,original);
    tex->releaseRequestReferences();
    expect(!runtime->stackPropertyIdentities.ownsHandle(captured));
    expect(throws<o::Exception>([&]{(void)graph.context.kind(captured);}));
  };

  "mml_node_binding_progress_retires_with_identity_record_at_reused_address"_test = [] {
    SharedGraph graph;
    auto runtime=graph.makeRuntime();
    auto factory=std::make_shared<m::MmlFactory>();
    (void)runtime->bindMmlFactory(factory);
    const auto bindings=graph.modules.find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
    expect(bindings!=nullptr);
    if(!bindings)return;
    const auto descriptor=factory->getNodeClass("mtext");
    std::optional<m::MmlMtext> storage;
    auto& first=storage.emplace(*factory);
    auto* const reusedAddress=&first;
    first.bindNativeConstructorClass(descriptor);
    const auto firstIdentity=graph.context.object();
    const auto firstCapture=bindings->nodes().beginRequestCapture();
    (void)bindings->bindNode(firstIdentity,first,factory);
    expect(bindings->nodeBindingComplete(firstIdentity));
    bindings->nodes().retireCapturedRequestBindings(firstCapture);
    expect(throws<o::Exception>([&]{(void)graph.context.kind(firstIdentity);}));
    storage.reset();

    auto& second=storage.emplace(*factory);
    expect(&second==reusedAddress);
    second.bindNativeConstructorClass(descriptor);
    const auto secondIdentity=graph.context.object();
    expect(secondIdentity!=firstIdentity);
    const auto secondCapture=bindings->nodes().beginRequestCapture();
    (void)bindings->bindNode(secondIdentity,second,factory);
    expect(bindings->nodeBindingComplete(secondIdentity));
    expect(&bindings->resolveNode(secondIdentity)==reusedAddress);
    bindings->nodes().retireCapturedRequestBindings(secondCapture);
    storage.reset();
  };

  "mml_factory_bind_failure_keeps_cleanup_owner_and_retryable_identity"_test = [] {
    SharedGraph graph;
    const auto nodeModule=graph.modules.getOrCreate<m::MmlNodeContextModule>(
        u"core/MmlTree/MmlNode.js",[&]{return std::make_shared<m::MmlNodeContextModule>(graph.context);});
    const auto maps=graph.modules.getOrCreate<o::MapContextBindings>(
        u"ECMAScript/Map",[&]{return std::make_shared<o::MapContextBindings>(graph.context);});
    auto bindings=m::MmlContextBindings::create(graph.context,graph.modules,nodeModule,maps);
    auto factory=std::make_shared<m::MmlFactory>();
    int foreignBinderOwner=0;
    factory->setIdentityBinder(&foreignBinderOwner,[](m::MmlNode&,std::shared_ptr<m::MmlFactory>){});
    expect(throws<tree::FactoryError>([&]{(void)bindings->bindFactory(factory);}));
    const auto pending=bindings->factories().find(factory.get());
    expect(pending.has_value());
    factory->clearIdentityBinder(&foreignBinderOwner);
    const auto identity=bindings->bindFactory(factory);
    expect(pending==identity);
    bindings.reset();
    const auto nativeInfo=factory->sourceClassInfo(factory->getNodeClass("text"));
    expect(nativeInfo!=nullptr);
    expect(nativeInfo->sourceConstructorName==u"TextNode");
  };

  "mml_context_construct_allocates_one_product_and_keeps_canonical_prototype_fields"_test = [] {
    SharedGraph graph;
    auto runtime=graph.makeRuntime();
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=runtime->bindMmlFactory(factory);
    const auto mover=constructorFor(graph.context,*runtime,factoryIdentity,u"mover");
    const auto moverPrototype=std::get<o::ObjectRef>(graph.context.get(o::Value(mover),o::Key(u"prototype")).data);
    const auto defaults=std::get<o::ObjectRef>(graph.context.get(o::Value(mover),o::Key(u"defaults")).data);
    const auto accents=std::get<o::ObjectRef>(graph.context.get(o::Value(mover),o::Key(u"ACCENTS")).data);
    const auto bindings=runtime->modules().find<m::MmlContextBindings>(u"core/MmlTree/ContextBindings");
    expect(bindings!=nullptr);
    if(!bindings)return;
    const auto beforeProducts=bindings->sourceAllocator().backend().productCountForDiagnostics();
    const auto beforeHandles=runtime->mmlNodeIdentities.canonicalHandleCount();
    const std::array<o::Value,1> args{o::Value(factoryIdentity)};
    const auto receiver=graph.context.construct(mover,args);
    expect(runtime->mmlNodeIdentities.canonicalHandleCount()==beforeHandles+1u);
    expect(bindings->sourceAllocator().backend().productCountForDiagnostics()==beforeProducts+1u);
    auto* node=dynamic_cast<m::MmlMover*>(&runtime->resolveMmlNode(receiver));
    expect(node!=nullptr);
    if(node)expect(&node->factoryOwner()==factory.get());
    expect(graph.context.prototype(receiver)==std::optional<o::ObjectRef>(moverPrototype));
    expect(graph.context.kind(defaults)==o::ObjectKind::object);
    expect(graph.context.kind(accents)==o::ObjectKind::array);
    const auto keys=graph.context.ownKeys(o::Value(mover));
    const auto defaultsAt=std::find(keys.begin(),keys.end(),o::Key(u"defaults"));
    const auto accentsAt=std::find(keys.begin(),keys.end(),o::Key(u"ACCENTS"));
    expect(defaultsAt!=keys.end()&&accentsAt!=keys.end()&&defaultsAt<accentsAt);
  };

  "mml_context_cache_clear_rebuilds_a_fresh_constructor_allocator_pair"_test = [] {
    PrivateGraph graph;
    folia::math::util::RuntimeModules modules(graph.context);
    auto first=graph.makeRuntime(&modules);
    auto firstFactory=std::make_shared<m::MmlFactory>();
    const auto firstIdentity=first->bindMmlFactory(firstFactory);
    const auto oldText=constructorFor(graph.context,*first,firstIdentity,u"text");
    modules.clear(u"core/MmlTree/ContextBindings");
    const std::array<o::Value,1> oldArgs{o::Value(firstIdentity)};
    const auto oldReceiver=graph.context.construct(oldText,oldArgs);
    expect(&first->resolveMmlNode(oldReceiver)==first->mmlNodeIdentities.resolve(oldReceiver));
    expect(throws<o::Exception>([&]{(void)graph.makeRuntime(&modules);}));
    first.reset();
    auto rebuilt=graph.makeRuntime(&modules);
    auto secondFactory=std::make_shared<m::MmlFactory>();
    const auto secondIdentity=rebuilt->bindMmlFactory(secondFactory);
    const auto newText=constructorFor(graph.context,*rebuilt,secondIdentity,u"text");
    expect(newText!=oldText);
    expect(throws<o::Exception>([&]{
        const auto oldPrototype=std::get<o::ObjectRef>(graph.context.get(o::Value(oldText),o::Key(u"prototype")).data);
        const auto staleReceiver=graph.context.objectWithPrototype(oldPrototype);
        const std::array<o::Value,1> args{o::Value(secondIdentity)};
        (void)graph.context.call(oldText,o::Value(staleReceiver),args);
    }));
    const std::array<o::Value,1> args{o::Value(secondIdentity)};
    const auto newReceiver=graph.context.construct(newText,args);
    expect(&rebuilt->resolveMmlNode(newReceiver)==rebuilt->mmlNodeIdentities.resolve(newReceiver));
  };
};
}
