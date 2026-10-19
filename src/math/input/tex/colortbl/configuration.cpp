// Native package assembly for the single Source ColortblConfiguration.ts owner.
module folia.math.input.tex.colortbl.configuration;
import folia.math.input.tex.colortbl.items;
import folia.math.input.tex.colortbl.methods;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.parser;
import folia.math.input.tex.token_map;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name) { return Key(std::u16string(name)); }
Value array(Context &cx, std::initializer_list<Value> values) {
  const ObjectRef result = cx.array(static_cast<std::uint32_t>(values.size()));
  std::uint32_t index = 0;
  for (const auto &value : values) {
    const auto digits = std::to_string(index++);
    cx.set(Value(result), key(std::u16string(digits.begin(), digits.end())),
           value);
  }
  return Value(result);
}
Value record(
    Context &cx,
    std::initializer_list<std::pair<std::u16string_view, Value>> values) {
  const ObjectRef result = cx.object();
  for (const auto &[name, value] : values)
    cx.set(Value(result), key(name), value);
  return Value(result);
}
Value configMethod(ParserRuntime &runtime) {
  auto body = std::make_shared<Function>();
  body->sourceMetadata = FunctionSourceMetadata{u"config", 2};
  body->call = [&runtime](const FunctionReceiver &,
                          FunctionArgs args) -> std::any {
    if (args.size() < 2)
      throw Exception("TypeError", u"Colortbl config arguments are missing");
    const auto *parserConfiguration =
        std::any_cast<ParserConfigurationRef>(&args[0]);
    const auto *jax = std::any_cast<TeX *>(&args[1]);
    if (!parserConfiguration || !*parserConfiguration || !jax || !*jax)
      throw Exception("TypeError", u"Colortbl config arguments are invalid");
    const auto parsed = (*jax)->parseOptions();
    const auto *parseOptions = std::get_if<ParseOptionsRef>(&parsed);
    if (!parseOptions || !*parseOptions)
      throw Exception("TypeError", u"Cannot read properties of null or "
                                   u"undefined (reading 'packageData')");
    const auto *packages = std::get_if<std::shared_ptr<SourceStringMap<Value>>>(
        &(*parseOptions)->packageData);
    if (packages && *packages && (*packages)->get(u"color"))
      return std::monostate{};
    const auto color = runtime.configurationHandler.get(u"color");
    const auto *colorConfiguration =
        std::get_if<std::shared_ptr<Configuration>>(&color);
    if (!colorConfiguration || !*colorConfiguration)
      throw Exception(
          "TypeError",
          u"Cannot read properties of undefined (reading 'config')");
    const auto colorIdentity =
        runtime.configurationIdentities.find(*colorConfiguration);
    const auto parserIdentity =
        runtime.parserConfigurationIdentities.find(*parserConfiguration);
    if (!colorIdentity || !parserIdentity)
      throw Exception("TypeError",
                      u"Color configuration identity is unavailable");
    const Value callable = (*colorConfiguration)->config();
    const auto *function = std::get_if<ObjectRef>(&callable.data);
    if (!function || runtime.context().kind(*function) != ObjectKind::function)
      throw Exception("TypeError", u"color.config is not a function");
    const std::array<Value, 2> callArgs{Value(*parserIdentity),
                                        Value((*jax)->instanceIdentity())};
    (void)runtime.context().call(*function, Value(*colorIdentity), callArgs);
    return std::monostate{};
  };
  OptionCallBinding binding;
  binding.receiver = [](Context &, Value receiver) {
    return FunctionReceiver(std::move(receiver));
  };
  binding.arguments = [&runtime](Context &, std::span<const Value> values) {
    if (values.size() < 2)
      throw Exception("TypeError", u"Colortbl config arguments are missing");
    const auto *configuration = std::get_if<ObjectRef>(&values[0].data);
    const auto *jax = std::get_if<ObjectRef>(&values[1].data);
    if (!configuration || !jax)
      throw Exception("TypeError", u"Colortbl config identities are invalid");
    return std::vector<std::any>{
        std::any(runtime.parserConfigurationIdentities.resolve(*configuration)),
        std::any(runtime.texIdentities.resolve(*jax).get())};
  };
  binding.result = [](Context &, const std::any &) {
    return Value(Undefined{});
  };
  const Value result =
      runtime.functions().bind(std::move(body), std::move(binding), u"config");
  if (const auto *identity = std::get_if<ObjectRef>(&result.data))
    runtime.context().define(*identity, key(u"length"),
                             Descriptor::data(Value(2.0), false, false, true));
  return result;
}
} // namespace
ConfigurationRef
ColortblConfiguration::initializeModule(ParserRuntime &runtime) {
  const auto prior = runtime.configurationHandler.get(u"colortbl");
  if (const auto *value = std::get_if<ConfigurationRef>(&prior);
      value && *value)
    return *value;
  auto &cx = runtime.context();
  const auto itemClass = ColorArrayItem::initializeModule(runtime);
  const auto itemConstructor = runtime.itemClassIdentities.find(itemClass);
  if (!itemConstructor)
    throw Exception("TypeError",
                    u"ColorArrayItem constructor identity is unavailable");
  const auto methods = ColortblMethods::create(runtime);
  const Value tableColor =
      BaseMethods::bind(runtime, methods.tableColor, u"TableColor");
  if (const auto *identity = std::get_if<ObjectRef>(&tableColor.data))
    cx.define(*identity, key(u"length"),
              Descriptor::data(Value(3.0), false, false, true));
  else
    throw Exception("TypeError",
                    u"TableColor function identity is unavailable");
  PropertyList commands;
  commands.set("cellcolor", Property(PropertyArray{Property(methods.tableColor),
                                                   Property("cell")}));
  commands.set("rowcolor", Property(PropertyArray{Property(methods.tableColor),
                                                  Property("row")}));
  commands.set(
      "columncolor",
      Property(PropertyArray{Property(methods.tableColor), Property("col")}));
  (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("colortbl"),
                                         Property(std::move(commands)));
  const Value handlers =
      record(cx, {{u"macro", array(cx, {Value(u"colortbl")})}});
  const Value items = record(cx, {{u"array", Value(*itemConstructor)}});
  const Value config = array(cx, {configMethod(runtime), Value(10.0)});
  return Configuration::create(runtime, u"colortbl",
                               record(cx, {{u"handler", handlers},
                                           {u"items", items},
                                           {u"priority", Value(10.0)},
                                           {u"config", config}}));
}
} // namespace folia::math::tex
