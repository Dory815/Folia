// Native loaded-module configuration for MathJax 4.1.3 Newcommand.
module folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.newcommand.items;
import folia.math.input.tex.newcommand.mappings;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.token_map;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value) { return Key(std::u16string(value)); }
Value array(Context &cx, std::initializer_list<Value> values) {
  const ObjectRef result = cx.array(static_cast<std::uint32_t>(values.size()));
  std::uint32_t index = 0;
  for (const auto &value : values) {
    const std::string decimal = std::to_string(index++);
    cx.set(Value(result), key(std::u16string(decimal.begin(), decimal.end())),
           value);
  }
  return Value(result);
}
Value record(
    Context &cx,
    std::initializer_list<std::pair<std::u16string_view, Value>> fields) {
  Value result(cx.object());
  for (const auto &[name, value] : fields)
    cx.set(result, key(name), value);
  return result;
}
void runConfig(ParserRuntime &runtime, ParserConfiguration &configuration,
               TeX &jax) {
  auto parse = jax.parseOptions();
  const auto *parseOptions = std::get_if<std::shared_ptr<ParseOptions>>(&parse);
  if (!parseOptions || !*parseOptions)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'packageData')");
  auto *data = std::get_if<std::shared_ptr<SourceStringMap<Value>>>(
      &(*parseOptions)->packageData);
  if (!data || !*data)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'has')");
  if ((*data)->has(u"newcommand"))
    return;
  Context &cx = runtime.context();
  (*data)->set(u"newcommand", Value(cx.object()));
  const Property parseMethods = runtime.parseMethods;
  const ParseMethod delimiter = functions::fromProperty(
      property_host::get(parseMethods, property_host::debugText(u"delimiter")));
  const ParseMethod environment = functions::fromProperty(property_host::get(
      parseMethods, property_host::debugText(u"environment")));
  (void)TokenMapHost::create<DelimiterMap>(runtime.mapHandler,Property("new-Delimiter"), delimiter,
                                           Property(PropertyList{}));
  (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("new-Command"),
                                         Property(PropertyList{}));
  (void)TokenMapHost::create<EnvironmentMap>(runtime.mapHandler,
      Property("new-Environment"), environment, Property(PropertyList{}));
  auto *handler =
      std::get_if<std::shared_ptr<SubHandlers>>(&(*parseOptions)->handlers);
  if (!handler || !*handler)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'add')");
  HandlerConfig config;
  config.setOwn("character", Property(PropertyArray{}));
  config.setOwn("delimiter",
                Property(PropertyArray{Property("new-Delimiter")}));
  config.setOwn("macro", Property(PropertyArray{Property("new-Delimiter"),
                                                Property("new-Command")}));
  config.setOwn("environment",
                Property(PropertyArray{Property("new-Environment")}));
  (*handler)->add(config, FallbackConfig{}, NEWCOMMAND_PRIORITY);
  (void)configuration;
}
Value configFunction(ParserRuntime &runtime) {
  auto body = std::make_shared<Function>();
  body->sourceMetadata = FunctionSourceMetadata{u"NewcommandConfig", 2};
  body->call = [&runtime](const FunctionReceiver &,
                          FunctionArgs args) -> std::any {
    if (args.size() < 2)
      throw Exception("TypeError",
                      u"NewcommandConfig requires ParserConfiguration and TeX");
    const auto *configuration = std::any_cast<ParserConfigurationRef>(&args[0]);
    const auto *jax = std::any_cast<TeX *>(&args[1]);
    if (!configuration || !*configuration || !jax || !*jax)
      throw Exception("TypeError", u"NewcommandConfig arguments are invalid");
    runConfig(runtime, **configuration, **jax);
    return std::monostate{};
  };
  OptionCallBinding binding;
  binding.receiver = [](Context &, Value) {
    return FunctionReceiver{Missing{}};
  };
  binding.arguments = [&runtime](Context &, std::span<const Value> args) {
    if (args.size() < 2)
      throw Exception("TypeError", u"NewcommandConfig requires (config, jax)");
    const auto *config = std::get_if<ObjectRef>(&args[0].data);
    const auto *jax = std::get_if<ObjectRef>(&args[1].data);
    if (!config || !jax)
      throw Exception("TypeError", u"NewcommandConfig arguments are invalid");
    return std::vector<std::any>{
        std::any(runtime.parserConfigurationIdentities.resolve(*config)),
        std::any(runtime.texIdentities.resolve(*jax).get())};
  };
  binding.result = [](Context &, const std::any &) {
    return Value(Undefined{});
  };
  return runtime.functions().bind(std::move(body), std::move(binding),
                                  u"NewcommandConfig");
}
} // namespace
void NewcommandConfig(ParserConfiguration &configuration, TeX &jax) {
  runConfig(configuration.runtime(), configuration, jax);
}
void NewcommandConfiguration::initializeModule(ParserRuntime &runtime) {
  const auto loaded = runtime.configurationHandler.get(u"newcommand");
  if (const auto *configuration =
          std::get_if<std::shared_ptr<Configuration>>(&loaded);
      configuration && *configuration)
    return;
  NewcommandItems::initializeModule(runtime);
  NewcommandMappings::initializeModule(runtime);
  Context &cx = runtime.context();
  const Value module = runtime.newcommandItemsModule;
  const Value constructor = cx.get(module, key(u"BeginEnvItem"));
  const Value handlers =
      record(cx, {{u"macro", array(cx, {Value(u"Newcommand-macros")})}});
  const Value items = record(cx, {{u"beginEnv", constructor}});
  const Value options = record(
      cx, {{u"maxMacros", Value(1000)},
           {u"protectedMacros", array(cx, {Value(u"begingroupSandbox")})}});
  Value config = record(cx, {{u"handler", handlers},
                             {u"items", items},
                             {u"options", options},
                             {u"config", configFunction(runtime)}});
  (void)Configuration::create(runtime, u"newcommand", std::move(config));
}
} // namespace folia::math::tex
