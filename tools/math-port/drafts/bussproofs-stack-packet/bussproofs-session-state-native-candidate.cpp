// Draft two-session witness for the pinned BussproofsUtil module state.
// This stays outside tests/math until Root enrolls and reviews it.
import std;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.bussproofs.configuration;
import folia.math.input.tex.bussproofs.util;
import folia.math.input.tex.parser;
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
import folia.math.output.common.font_data_surface;
import folia.math.handlers.html.html_math_document;
import folia.math.core.math_item;
import folia.math.core.input_jax;
import folia.math.util.promise;
import folia.math.input.tex.stack;
import folia.math.input.tex.stack_context_bindings;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.stack_item_factory;
import folia.math.input.tex.bussproofs.items;

namespace {
namespace opts = folia::math::options;
namespace svg = folia::math::output::svg;
namespace common = folia::math::output::common;
namespace tex = folia::math::tex;
using namespace folia::math;

struct CountingStack final : tex::Stack {
  using tex::Stack::Stack;
  std::size_t pushCalls = 0;
  void Push(std::span<const tex::StackEntry> entries) override {
    ++pushCalls;
    tex::Stack::Push(entries);
  }
};

struct AlphaItem final : tex::BaseItem {
  std::string kind() const override { return "alpha"; }
  std::string toString() const override { return "α"; }
};

tex::StackItemClassRef registerAlphaItem(tex::ParserRuntime& runtime,
                                         tex::StackItemFactory& factory) {
  auto& context=runtime.context();
  const auto* module=std::get_if<opts::ObjectRef>(&runtime.stackItemModule.data);
  if(!module)throw std::runtime_error("StackItem module is unavailable for AlphaItem registration");
  const auto baseValue=context.get(opts::Value(*module),opts::Key(u"BaseItem"));
  const auto* base=std::get_if<opts::ObjectRef>(&baseValue.data);
  if(!base)throw std::runtime_error("BaseItem constructor is unavailable for AlphaItem registration");
  auto type=tex::StackItemFactoryBindings::registerSubclass(
      runtime,u"AlphaItem","alpha",1,typeid(AlphaItem),
      [] { return std::make_unique<AlphaItem>(); },
      [](tex::BaseItem&,tree::FactoryArgs) {},
      [](const tex::BaseItem* item){return dynamic_cast<const AlphaItem*>(item)!=nullptr;},
      [] { return tex::BaseItem::baseErrors(); },*base,false,
      tex::StackItemSourceSurface{true,{{u"toString",0}}, {}});
  factory.setNodeClass("alpha",type);
  return type;
}

struct Fixture {
  std::unique_ptr<svg::NativeRenderSession> session;
  std::shared_ptr<handlers::html::HTMLMathDocument> document;
  std::shared_ptr<tex::bussproofs_util::BussproofsUtilModule> util;
  std::shared_ptr<core::MathItem> item;
  opts::ObjectRef itemIdentity{};
  tex::TeXRef inputJax;
  opts::ObjectRef inputJaxIdentity{};
  opts::Context *context = nullptr;
  bool reusedModuleOwner = false;

  struct JaxDocument {
    std::shared_ptr<handlers::html::HTMLMathDocument> document;
    tex::TeXRef inputJax;
    opts::ObjectRef inputJaxIdentity{};
  };

  Fixture() {
    common::FontLoadHost host;
    host.asyncIsSynchronous = [] { return true; };
    host.warn = [](const std::any &) {};
    host.syncLoad = [](common::FontData &, const common::DynamicFileRef &,
                       std::string) {
      throw opts::Exception("Unimplemented", u"Unexpected font load");
    };
    host.asyncLoad = [](std::shared_ptr<common::FontData>,
                        common::DynamicFileRef,
                        std::string) -> async::PromiseRef {
      throw opts::Exception("Unimplemented", u"Unexpected font load");
    };
    session = std::make_unique<svg::NativeRenderSession>(std::move(host));
    context = &session->context();
    auto &services = session->services();
    auto &runtime = services.parser();
    (void)tex::BasePackage::initializeModule(runtime);
    const auto configuration = tex::BussproofsConfiguration::initializeModule(
        runtime, services.modules());
    (void)configuration;

    const auto config = context->object();
    const auto packages = context->array(2);
    context->set(packages, opts::Key(u"0"), opts::Value(u"base"));
    context->set(packages, opts::Key(u"1"), opts::Value(u"bussproofs"));
    context->set(config, opts::Key(u"packages"), opts::Value(packages));
    const auto outputOptions = context->object();
    context->set(outputOptions, opts::Key(u"fontCache"), opts::Value(u"none"));
    context->set(outputOptions, opts::Key(u"exFactor"), opts::Value(0.5));
    (void)session->construct(opts::Value(outputOptions));
    document = svg::createNativeSvgDocument(
        *session, opts::Value(opts::Undefined{}), opts::Value(config));

    const opts::Value docValue(document->sourceIdentity());
    const auto inputArray = context->get(docValue, opts::Key(u"inputJax"));
    const auto inputValue = context->get(inputArray, opts::Key(u"0"));
    const auto *inputIdentity = std::get_if<opts::ObjectRef>(&inputValue.data);
    if (!inputIdentity)
      throw std::runtime_error("input Jax Context identity is missing");
    inputJaxIdentity = *inputIdentity;
    inputJax = runtime.texIdentities.resolve(inputJaxIdentity);
    if (!inputJax)
      throw std::runtime_error("input Jax native owner is missing");

    util = tex::bussproofs_util::BussproofsUtilModule::initializeModule(
        runtime, services.modules());
    const auto sameModule =
        tex::bussproofs_util::BussproofsUtilModule::initializeModule(
            runtime, services.modules());
    reusedModuleOwner = sameModule == util;
    if (!reusedModuleOwner)
      throw std::runtime_error(
          "RuntimeModules did not reuse the Bussproofs utility owner");
  }

  JaxDocument createJaxDocument() {
    const auto config = context->object();
    const auto packages = context->array(2);
    context->set(packages, opts::Key(u"0"), opts::Value(u"base"));
    context->set(packages, opts::Key(u"1"), opts::Value(u"bussproofs"));
    context->set(config, opts::Key(u"packages"), opts::Value(packages));
    auto nextDocument = svg::createNativeSvgDocument(
        *session, opts::Value(opts::Undefined{}), opts::Value(config));
    const opts::Value docValue(nextDocument->sourceIdentity());
    const auto inputArray = context->get(docValue, opts::Key(u"inputJax"));
    const auto inputValue = context->get(inputArray, opts::Key(u"0"));
    const auto *inputIdentity = std::get_if<opts::ObjectRef>(&inputValue.data);
    if (!inputIdentity)
      throw std::runtime_error("second input Jax Context identity is missing");
    auto &runtime = session->services().parser();
    auto nextJax = runtime.texIdentities.resolve(*inputIdentity);
    if (!nextJax)
      throw std::runtime_error("second input Jax native owner is missing");
    return JaxDocument{std::move(nextDocument), std::move(nextJax),
                       *inputIdentity};
  }

  std::shared_ptr<core::MathItem>
  compileProof(JaxDocument &target, std::u16string_view source) {
    const opts::Value docValue(target.document->sourceIdentity());
    const auto options = context->get(docValue, opts::Key(u"options"));
    const auto constructor = context->get(options, opts::Key(u"MathItem"));
    const auto *itemConstructor =
        std::get_if<opts::ObjectRef>(&constructor.data);
    if (!itemConstructor)
      throw std::runtime_error("second MathItem constructor is missing");
    const std::array<opts::Value, 3> args{
        opts::Value(std::u16string(source)),
        opts::Value(target.inputJaxIdentity), opts::Value(true)};
    const auto itemIdentity = context->construct(*itemConstructor, args);
    const auto *native = std::any_cast<std::shared_ptr<NativeObject>>(
        &context->payload(itemIdentity));
    auto item = native ? std::dynamic_pointer_cast<core::MathItem>(*native)
                       : nullptr;
    if (!item)
      throw std::runtime_error("second MathItem native owner is missing");
    const auto compileValue=context->get(opts::Value(itemIdentity),opts::Key(u"compile"));
    const auto *compileFunction=std::get_if<opts::ObjectRef>(&compileValue.data);
    if(!compileFunction)throw std::runtime_error("MathItem.compile Context method is unavailable");
    const std::array<opts::Value,1> compileArgs{opts::Value(target.document->sourceIdentity())};
    (void)context->call(*compileFunction,opts::Value(itemIdentity),compileArgs);
    return item;
  }

  core::InputJaxFilterDataRef filterData(
      JaxDocument &target, const std::shared_ptr<core::MathItem> &math) {
    auto &runtime=session->services().parser();
    const auto options = target.inputJax->parseOptions();
    const auto *parseOptions = std::get_if<tex::ParseOptionsRef>(&options);
    if (!parseOptions || !*parseOptions)
      throw std::runtime_error("second ParseOptions native owner is missing");
    auto result=std::make_shared<core::InputJaxFilterData>();
    result->initialize(core::MathItemReference{math.get()},
                       core::MathDocumentReference{target.document.get()},
                       std::any(*parseOptions));
    result->encodeData=[&runtime](const std::any& value)->opts::Value{
      if(const auto* contextValue=std::any_cast<opts::Value>(&value))return *contextValue;
      if(const auto* owner=std::any_cast<tex::ParseOptionsRef>(&value);owner&&*owner){
        const auto identity=runtime.parseOptionsIdentities.find(*owner);
        if(!identity)throw opts::Exception("TypeError",u"ParseOptions lacks its canonical Context identity");
        return opts::Value(*identity);
      }
      throw opts::Exception("TypeError",u"filter data has no canonical Context value");
    };
    result->decodeData=[&runtime](opts::Value value)->std::any{
      const auto* identity=std::get_if<opts::ObjectRef>(&value.data);
      if(!identity)throw opts::Exception("TypeError",u"filter data is not ParseOptions");
      return std::any(runtime.parseOptionsIdentities.resolve(*identity));
    };
    result->bindSourceRecord(*context,result,runtime.executionLifetime());
    return result;
  }

  void compileProof(std::u16string_view source) {
    const opts::Value docValue(document->sourceIdentity());
    const auto options = context->get(docValue, opts::Key(u"options"));
    const auto constructor = context->get(options, opts::Key(u"MathItem"));
    const auto *itemConstructor =
        std::get_if<opts::ObjectRef>(&constructor.data);
    if (!itemConstructor)
      throw std::runtime_error("MathItem constructor is missing");
    const std::array<opts::Value, 3> args{
        opts::Value(std::u16string(source)), opts::Value(inputJaxIdentity),
        opts::Value(true)};
    const auto itemIdentity = context->construct(*itemConstructor, args);
    const auto *native = std::any_cast<std::shared_ptr<NativeObject>>(
        &context->payload(itemIdentity));
    item = native ? std::dynamic_pointer_cast<core::MathItem>(*native) : nullptr;
    if (!item)
      throw std::runtime_error("MathItem native owner is missing");
    this->itemIdentity=itemIdentity;
    const auto compileValue=context->get(opts::Value(itemIdentity),opts::Key(u"compile"));
    const auto *compileFunction=std::get_if<opts::ObjectRef>(&compileValue.data);
    if(!compileFunction)throw std::runtime_error("MathItem.compile Context method is unavailable");
    const std::array<opts::Value,1> compileArgs{opts::Value(document->sourceIdentity())};
    (void)context->call(*compileFunction,opts::Value(itemIdentity),compileArgs);
  }

  core::InputJaxFilterDataRef filterData() {
    JaxDocument target{document,inputJax,inputJaxIdentity};
    return filterData(target,item);
  }

  void invoke(std::u16string_view name, core::InputJaxFilterDataRef data) {
    const auto function = util->function(name);
    const auto *identity = std::get_if<opts::ObjectRef>(&function.data);
    if (!identity)
      throw std::runtime_error("Bussproofs export identity is missing");
    const auto argument = context->object();
    context->payload(argument) = std::move(data);
    const std::array<opts::Value, 1> args{opts::Value(argument)};
    (void)context->call(*identity, opts::Value(opts::Undefined{}), args);
  }
};
struct ErrorObservation {
  bool threw = false;
  std::string name;
  std::string message;
};
ErrorObservation balanceError(Fixture &fixture,
                              core::InputJaxFilterDataRef data) {
  try {
    fixture.invoke(u"balanceRules", std::move(data));
    return {};
  } catch (const opts::Exception &error) {
    return {true, error.name, property_host::debugText(error.message)};
  }
}
} // namespace

int main() {
  try {
    Fixture first;
    Fixture second;
    const bool independentModuleOwners = first.util != second.util;
    if (!independentModuleOwners)
      throw std::runtime_error(
          "separate sessions unexpectedly share a Bussproofs utility owner");
    auto secondJax = first.createJaxDocument();
    auto &parserRuntime=first.session->services().parser();
    const auto bindStackFor=[&](const tex::TeXRef& jax){
      const auto parseValue=jax->parseOptions();
      const auto *parse=std::get_if<tex::ParseOptionsRef>(&parseValue);
      if(!parse||!*parse)throw std::runtime_error("TeX Jax has no actual ParseOptions owner");
      const auto *factory=std::get_if<std::shared_ptr<tex::StackItemFactory>>(&(*parse)->itemFactory);
      if(!factory||!*factory)throw std::runtime_error("ParseOptions has no actual StackItemFactory owner");
      auto stack=std::make_shared<tex::Stack>(
          tex::StackFactoryReference(static_cast<tex::StackItemFactoryService*>(factory->get())),
          PropertyList{},false);
      const auto identity=parserRuntime.bindStack(stack);
      return std::pair{std::move(stack),identity};
    };
    const auto firstStack=bindStackFor(first.inputJax);
    const auto sameEnvironmentStack=bindStackFor(secondJax.inputJax);
    const auto stackConstructor=parserRuntime.stackBindings.constructor();
    const bool sameModuleStackSurface=firstStack.second!=sameEnvironmentStack.second&&
      firstStack.first!=sameEnvironmentStack.first&&
      first.context->prototype(firstStack.second)==first.context->prototype(sameEnvironmentStack.second)&&
      first.context->get(opts::Value(firstStack.second),opts::Key(u"constructor"))==opts::Value(stackConstructor)&&
      first.context->get(opts::Value(sameEnvironmentStack.second),opts::Key(u"constructor"))==opts::Value(stackConstructor);
    if(!sameModuleStackSurface)
      throw std::runtime_error("two TeX Jax instances did not share the canonical Stack module surface");
    const auto stackPrototype=first.context->get(opts::Value(stackConstructor),opts::Key(u"prototype"));
    const auto stringValue=first.context->get(stackPrototype,opts::Key(u"toString"));
    const auto *stringFunction=std::get_if<opts::ObjectRef>(&stringValue.data);
    if(!stringFunction)throw std::runtime_error("Stack.toString has no Context function identity");
    const auto initialStackString=first.context->call(*stringFunction,opts::Value(firstStack.second));
    const bool projectedStackToString=initialStackString==opts::Value(u"stack[\n  start[]\n]");
    if(!projectedStackToString)
      throw std::runtime_error("projected Stack.toString did not stringify its StackItem array element");

    const auto parseValue=first.inputJax->parseOptions();
    const auto *parse=std::get_if<tex::ParseOptionsRef>(&parseValue);
    if(!parse||!*parse)throw std::runtime_error("ParseOptions owner is unavailable for StackItem string witness");
    const auto *itemFactory=std::get_if<std::shared_ptr<tex::StackItemFactory>>(&(*parse)->itemFactory);
    if(!itemFactory||!*itemFactory)throw std::runtime_error("StackItemFactory owner is unavailable for StackItem string witness");
    const auto alphaClass=registerAlphaItem(parserRuntime,**itemFactory);
    (void)alphaClass;
    const auto alphaConstructor=parserRuntime.itemClassIdentities.find(alphaClass);
    if(!alphaConstructor||first.context->get(opts::Value(*alphaConstructor),opts::Key(u"length"))!=opts::Value(1.0))
      throw std::runtime_error("AlphaItem constructor length does not match inherited Source constructor(factory)");
    const auto alphaPrototype=std::get<opts::ObjectRef>(first.context->get(opts::Value(*alphaConstructor),opts::Key(u"prototype")).data);
    if(!first.context->hasOwn(alphaPrototype,opts::Key(u"toString")))
      throw std::runtime_error("AlphaItem.toString is not an own Source prototype member");
    auto *alphaRaw=(*itemFactory)->itemFactory().create("alpha",{});
    const auto alphaRef=(*itemFactory)->reference(alphaRaw);
    auto alphaStack=std::make_shared<tex::Stack>(firstStack.first->sourceFactory(),PropertyList{},false);
    alphaStack->stackArray()->pushEntry(tex::StackEntry(alphaRef));
    const std::string expectedAlphaStack="stack[\n  start[]\n  α\n]";
    const bool nativeAlphaStackToString=alphaStack->toString()==expectedAlphaStack;
    const auto alphaStackIdentity=parserRuntime.bindStack(alphaStack);
    const auto alphaStackString=first.context->call(*stringFunction,opts::Value(alphaStackIdentity));
    const bool projectedAlphaStackToString=alphaStackString==opts::Value(u"stack[\n  start[]\n  α\n]");
    if(!nativeAlphaStackToString||!projectedAlphaStackToString)
      throw std::runtime_error("UTF-8 StackItem ToString did not preserve alpha before and after Context projection");

    auto *proofRaw=(*itemFactory)->itemFactory().create("proofTree",{});
    auto *proof=dynamic_cast<tex::ProofTreeItem*>(proofRaw);
    if(!proof)throw std::runtime_error("ProofTreeItem fixture was not created by the actual factory");
    const auto proofRef=(*itemFactory)->reference(proofRaw);
    const auto proofIdentity=parserRuntime.stackItemIdentities.find(proofRef);
    if(!proofIdentity)throw std::runtime_error("ProofTreeItem has no canonical Context identity");
    const auto oldStackValue=first.context->get(opts::Value(*proofIdentity),opts::Key(u"innerStack"));
    const auto *oldStackIdentity=std::get_if<opts::ObjectRef>(&oldStackValue.data);
    if(!oldStackIdentity)throw std::runtime_error("ProofTreeItem innerStack is not the canonical Stack identity");
    const auto originalProofStack=parserRuntime.resolveStack(*oldStackIdentity);
    auto replacementProofStack=std::make_shared<CountingStack>(
        firstStack.first->sourceFactory(),PropertyList{},true);
    const auto replacementProofIdentity=parserRuntime.bindStack(replacementProofStack);
    first.context->set(opts::Value(*proofIdentity),opts::Key(u"innerStack"),opts::Value(replacementProofIdentity));
    Property startGlobalProperty(PropertyList{});
    std::any startGlobal(startGlobalProperty);
    if(!std::any_cast<Property>(&startGlobal))
      throw std::runtime_error("StartItem(factory, global) fixture argument is not the canonical Property type");
    auto *startItem=(*itemFactory)->itemFactory().create("start",tree::FactoryArgs(&startGlobal,1));
    (void)proof->checkItem(startItem);
    const auto currentProofStack=first.context->get(opts::Value(*proofIdentity),opts::Key(u"innerStack"));
    const bool proofInnerStackReplacementUsesOneOwner=
      currentProofStack==opts::Value(replacementProofIdentity)&&
      replacementProofStack->pushCalls==1&&originalProofStack->height()==opts::Value(1.0);
    if(!proofInnerStackReplacementUsesOneOwner)
      throw std::runtime_error("ProofTreeItem.innerStack replacement did not route checkItem through the replacement Stack owner");
    first.context->set(opts::Value(*proofIdentity),opts::Key(u"innerStack"),opts::Value(opts::Null{}));
    ErrorObservation delayedNullInnerStackError;
    try {
      (void)proof->checkItem(startItem);
    } catch (const opts::Exception& error) {
      delayedNullInnerStackError={true,error.name,property_host::debugText(error.message)};
    }
    const bool innerStackNullErrorIsDelayed=
      first.context->get(opts::Value(*proofIdentity),opts::Key(u"innerStack"))==opts::Value(opts::Null{})&&
      delayedNullInnerStackError.threw&&delayedNullInnerStackError.name=="TypeError"&&
      delayedNullInnerStackError.message=="Cannot read properties of null (reading 'Push')";
    if(!innerStackNullErrorIsDelayed)
      throw std::runtime_error("ProofTreeItem.innerStack null assignment did not fail at the later Push read");
    constexpr std::u16string_view proofSource =
        uR"bsp(\begin{prooftree}\AxiomC{A}\UnaryInfC{B}\UnaryInfC{C}\end{prooftree})bsp";
    first.compileProof(proofSource);
    const auto arrayAlias=first.context->get(opts::Value(firstStack.second),opts::Key(u"stack"));
    const auto rootValue=first.context->get(opts::Value(first.itemIdentity),opts::Key(u"root"));
    const auto *mmlIdentity=std::get_if<opts::ObjectRef>(&rootValue.data);
    if(!mmlIdentity)throw std::runtime_error("compiled MathItem.root projection has no canonical MML Context identity");
    const auto pushValue=first.context->get(stackPrototype,opts::Key(u"Push"));
    const auto *pushFunction=std::get_if<opts::ObjectRef>(&pushValue.data);
    if(!pushFunction)throw std::runtime_error("Stack.Push has no Context function identity");
    const std::array<opts::Value,1> mmlArgument{opts::Value(*mmlIdentity)};
    (void)first.context->call(*pushFunction,opts::Value(firstStack.second),mmlArgument);
    const auto topAfterPush=firstStack.first->Top();
    const auto *topItem=std::get_if<tex::StackItemRef>(&topAfterPush);
    const auto topIdentity=topItem
      ? parserRuntime.stackItemIdentities.find(*topItem)
      : std::optional<opts::ObjectRef>{};
    const auto topMml=topItem
      ? tex::stack_host::item(topAfterPush,"First").First()
      : mml::NodeValue(Missing{});
    const auto topMmlIdentity=topMml.isNode()
      ? parserRuntime.mmlNodeIdentities.find(topMml.requireNode("StackItem.First"))
      : std::optional<opts::ObjectRef>{};
    const auto boundArrayIdentity=std::get_if<opts::ObjectRef>(&arrayAlias.data);
    const bool canonicalMmlPushThroughBoundArray=topIdentity&&topMmlIdentity&&boundArrayIdentity&&
      *topMmlIdentity==*mmlIdentity&&
      first.context->get(arrayAlias,opts::Key(u"0"))==opts::Value(*topIdentity)&&
      std::holds_alternative<opts::Undefined>(first.context->get(arrayAlias,opts::Key(u"1")).data)&&
      first.context->arrayLength(*boundArrayIdentity)==1;
    if(!canonicalMmlPushThroughBoundArray)
      throw std::runtime_error("MML Push did not follow Source start-item consumption through the bound Context array owner");
    const auto sameRuntimeMath = first.compileProof(secondJax, proofSource);
    second.compileProof(proofSource);
    const auto firstData = first.filterData();
    const auto sameRuntimeData = first.filterData(secondJax, sameRuntimeMath);
    const auto secondData = second.filterData();
    first.invoke(u"saveDocument", firstData);
    first.invoke(u"saveDocument", sameRuntimeData);
    first.invoke(u"clearDocument", firstData);
    const ErrorObservation sameRuntimeError =
        balanceError(first, sameRuntimeData);
    constexpr std::string_view expectedError =
        "Cannot read properties of null (reading 'outputJax')";
    if (!sameRuntimeError.threw || sameRuntimeError.name != "TypeError" ||
        sameRuntimeError.message != expectedError)
      throw std::runtime_error(
          "same RuntimeModules did not match Source's cleared-document error");

    second.invoke(u"saveDocument", secondData);
    first.invoke(u"clearDocument", firstData);
    const ErrorObservation separateSessionError =
        balanceError(second, secondData);
    if (separateSessionError.threw)
      throw std::runtime_error(
          "independent session did not preserve its Source document");
    std::cout
        << "{\"source\":\"MathJax-src 4.1.3 BussproofsUtil.js\","
        << "\"sameEnvironmentUsesSameExports\":"
        << (first.reusedModuleOwner ? "true" : "false") << ','
        << "\"sameEnvironmentTwoJaxShareStackSurface\":"
        << (sameModuleStackSurface ? "true" : "false") << ','
        << "\"projectedStackToString\":"
        << (projectedStackToString ? "true" : "false") << ','
        << "\"nativeAlphaStackToString\":"
        << (nativeAlphaStackToString ? "true" : "false") << ','
        << "\"projectedAlphaStackToString\":"
        << (projectedAlphaStackToString ? "true" : "false") << ','
        << "\"proofInnerStackReplacementUsesOneOwner\":"
        << (proofInnerStackReplacementUsesOneOwner ? "true" : "false") << ','
        << "\"innerStackNullErrorIsDelayed\":"
        << (innerStackNullErrorIsDelayed ? "true" : "false") << ','
        << "\"canonicalMmlPushThroughBoundArray\":"
        << (canonicalMmlPushThroughBoundArray ? "true" : "false") << ','
        << "\"independentModuleHasDistinctExports\":"
        << (independentModuleOwners ? "true" : "false") << ','
        << "\"sameEnvironmentAfterOtherJaxClear\":{\"name\":"
        << '"' << sameRuntimeError.name << "\",\"message\":\""
        << sameRuntimeError.message << "\"},"
        << "\"independentEnvironmentAfterOtherModuleClear\":{"
        << "\"name\":null,\"message\":null}}\n";
    return 0;
  } catch (const opts::Exception &error) {
    std::cerr << error.name << ": "
              << property_host::debugText(error.message) << '\n';
    return 1;
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
