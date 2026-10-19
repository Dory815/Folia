// Supplementary native reflection candidate for BussproofsUtil exports.
// Its output is compared separately from the locked 31-case package corpus.
import std;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.bussproofs.configuration;
import folia.math.input.tex.bussproofs.util;
import folia.math.output.svg.render_session;
import folia.math.output.common.font_data_surface;

namespace {
namespace opts = folia::math::options;
namespace svg = folia::math::output::svg;
namespace common = folia::math::output::common;
namespace tex = folia::math::tex;
using namespace folia::math;

std::string quote(std::string_view value) {
  std::string result = "\"";
  for (const char c : value) {
    if (c == '\\' || c == '"')
      result.push_back('\\');
    result.push_back(c);
  }
  return result + '"';
}
opts::ObjectRef object(const opts::Value &value) {
  const auto *identity = std::get_if<opts::ObjectRef>(&value.data);
  if (!identity)
    throw std::runtime_error("Expected a Context object identity");
  return *identity;
}
struct Fixture {
  std::unique_ptr<svg::NativeRenderSession> session;
  opts::Context &context;
  tex::ParserRuntime &runtime;
  opts::ObjectRef configuration;
  std::shared_ptr<tex::bussproofs_util::BussproofsUtilModule> util;

  Fixture()
      : session([] {
          common::FontLoadHost host;
          host.asyncIsSynchronous = [] { return true; };
          host.warn = [](const std::any &) {};
          host.syncLoad = [](common::FontData &,
                             const common::DynamicFileRef &, std::string) {
            throw opts::Exception("Unimplemented", u"Unexpected font load");
          };
          host.asyncLoad = [](std::shared_ptr<common::FontData>,
                              common::DynamicFileRef,
                              std::string) -> async::PromiseRef {
            throw opts::Exception("Unimplemented", u"Unexpected font load");
          };
          return std::make_unique<svg::NativeRenderSession>(std::move(host));
        }()),
        context(session->context()), runtime(session->services().parser()) {
    (void)tex::BasePackage::initializeModule(runtime);
    const auto config = tex::BussproofsConfiguration::initializeModule(
        runtime, session->services().modules());
    const auto identity = runtime.configurationIdentities.find(config);
    if (!identity)
      throw std::runtime_error("Bussproofs configuration identity is missing");
    configuration = *identity;
    util = tex::bussproofs_util::BussproofsUtilModule::initializeModule(
        runtime, session->services().modules());
  }
};

std::string functionInfo(Fixture &fixture, const opts::Value &function) {
  const auto identity = object(function);
  const auto name = fixture.context.get(identity, opts::Key(u"name"));
  const auto length = fixture.context.get(identity, opts::Key(u"length"));
  const auto *nameText = std::get_if<std::u16string>(&name.data);
  const auto *arity = std::get_if<double>(&length.data);
  if (!nameText || !arity)
    throw std::runtime_error("Source function name or length is missing");
  return "\"functionName\":" +
         quote(property_host::debugText(*nameText)) +
         ",\"length\":" + std::to_string(static_cast<unsigned>(*arity));
}

std::string processors(Fixture &fixture, std::u16string_view phase,
                       std::span<const std::string_view> names) {
  const auto value = fixture.context.get(fixture.configuration,
                                         opts::Key(std::u16string(phase)));
  const auto identity = object(value);
  const auto length = fixture.context.arrayLength(identity);
  std::string result = "[";
  for (std::size_t i = 0; i < length; ++i) {
    if (i)
      result += ',';
    const auto index = std::to_string(i);
    const auto tuple = object(fixture.context.get(
        identity, opts::Key(std::u16string(index.begin(), index.end()))));
    const auto callable = fixture.context.get(tuple, opts::Key(u"0"));
    const auto priority = fixture.context.get(tuple, opts::Key(u"1"));
    const auto callableIdentity = object(callable);
    std::string exportName = "null";
    for (const auto name : names) {
      const auto candidate = object(fixture.util->function(
          std::u16string(name.begin(), name.end())));
      if (candidate == callableIdentity) {
        exportName = quote(name);
        break;
      }
    }
    const auto *number = std::get_if<double>(&priority.data);
    if (!number)
      throw std::runtime_error("Processor priority is not numeric");
    result += "{\"export\":" + exportName + "," +
              functionInfo(fixture, callable) + ",\"priority\":" +
              std::to_string(static_cast<int>(*number)) + "}";
  }
  return result + ']';
}
} // namespace

int main() {
  try {
    Fixture fixture;
    constexpr std::array<std::string_view, 7> names{
        "setProperty", "getProperty", "removeProperty", "balanceRules",
        "makeBsprAttributes", "saveDocument", "clearDocument"};
    std::string exports = "[";
    for (std::size_t i = 0; i < names.size(); ++i) {
      if (i)
        exports += ',';
      const auto name = names[i];
      const auto function = fixture.util->function(
          std::u16string(name.begin(), name.end()));
      exports += "{\"name\":" + quote(name) + ',' +
                 functionInfo(fixture, function) + '}';
    }
    exports += ']';
    std::cout << "{\"source\":\"MathJax-src 4.1.3 "
                 "fb987178c2d279a99b0db5ea02e751691703955e\","
              << "\"utilExports\":" << exports
              << ",\"configuration\":{\"preprocessors\":"
              << processors(fixture, u"preprocessors", names)
              << ",\"postprocessors\":"
              << processors(fixture, u"postprocessors", names) << "}}\n";
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
