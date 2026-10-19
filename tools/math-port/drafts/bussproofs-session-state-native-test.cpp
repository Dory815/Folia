// Draft two-session witness for the pinned BussproofsUtil module state.
// This stays outside tests/math until Root enrolls and reviews it.
import std;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.bussproofs.configuration;
import folia.math.input.tex.bussproofs.util;
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
import folia.math.output.common.font_data_surface;
import folia.math.handlers.html.html_math_document;
import folia.math.core.math_item;
import folia.math.core.input_jax;
import folia.math.util.promise;

namespace {
namespace opts = folia::math::options;
namespace svg = folia::math::output::svg;
namespace common = folia::math::output::common;
namespace tex = folia::math::tex;
using namespace folia::math;

struct Fixture {
  std::unique_ptr<svg::NativeRenderSession> session;
  std::shared_ptr<handlers::html::HTMLMathDocument> document;
  std::shared_ptr<tex::bussproofs_util::BussproofsUtilModule> util;
  std::shared_ptr<core::MathItem> item;
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
    (void)target.inputJax->compile(
        *item, core::MathDocumentReference{target.document.get()});
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
    (void)inputJax->compile(*item,
                            core::MathDocumentReference{document.get()});
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
    constexpr std::u16string_view proof =
        uR"bsp(\begin{prooftree}\AxiomC{A}\UnaryInfC{B}\UnaryInfC{C}\end{prooftree})bsp";
    first.compileProof(proof);
    const auto sameRuntimeMath = first.compileProof(secondJax, proof);
    second.compileProof(proof);
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
