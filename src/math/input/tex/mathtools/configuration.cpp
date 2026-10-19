// Native package configuration from MathtoolsConfiguration.ts.
module folia.math.input.tex.mathtools.configuration;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.mathtools.items;
import folia.math.input.tex.mathtools.mappings;
import folia.math.input.tex.mathtools.methods;
import folia.math.input.tex.mathtools.tags;
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.macro;
import folia.math.input.tex.node_util;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value) { return Key(std::u16string(value)); }
Value record(
    Context &cx,
    std::initializer_list<std::pair<std::u16string_view, Value>> fields) {
  const auto out = cx.object();
  for (const auto &[name, value] : fields)
    cx.set(out, key(name), value);
  return Value(out);
}
Value array(Context &cx, std::initializer_list<Value> values) {
  const auto out = cx.array(static_cast<std::uint32_t>(values.size()));
  std::uint32_t index = 0;
  for (const auto &value : values) {
    const auto digits = std::to_string(index++);
    cx.set(out, key(std::u16string(digits.begin(), digits.end())), value);
  }
  return Value(out);
}
Value expandableEmpty(Context &cx) {
  return options::expandable(cx, Value(cx.object()));
}
ParseOptionsRef parseOptionsFor(ParserRuntime &runtime, const Value &identity) {
  const auto *object = std::get_if<ObjectRef>(&identity.data);
  if (!object)
    throw Exception("TypeError",
                    u"Mathtools config ParseOptions identity is invalid");
  return runtime.parseOptionsIdentities.resolve(*object);
}
Value configFunction(ParserRuntime &runtime, FunctionRef tagFormat) {
  auto body = std::make_shared<Function>();
  body->sourceMetadata = FunctionSourceMetadata{u"configMathtools", 2};
  body->call = [&runtime, tagFormat = std::move(tagFormat)](
                   const FunctionReceiver &, FunctionArgs args) -> std::any {
    if (args.size() < 2)
      throw Exception("TypeError", u"Mathtools config arguments are missing");
    const auto *configuration = std::any_cast<ParserConfigurationRef>(&args[0]);
    const auto *jax = std::any_cast<TeX *>(&args[1]);
    if (!configuration || !*configuration || !jax || !*jax)
      throw Exception("TypeError", u"Mathtools config arguments are invalid");
    NewcommandConfig(**configuration, **jax);
    const auto shared = (**jax).parseOptions();
    const auto *parsed = std::get_if<ParseOptionsRef>(&shared);
    if (!parsed || !*parsed)
      property_host::typeError(
          "Cannot read properties of null or undefined (reading 'options')");
    Context &cx = runtime.context();
    const auto *handlers =
        std::get_if<std::shared_ptr<SubHandlers>>(&(*parsed)->handlers);
    if (!handlers || !*handlers)
      property_host::typeError(
          "Cannot read properties of undefined (reading 'retrieve')");
    TokenMapRef rawCommandMap = (**handlers).retrieve(Property("new-Command"));
    auto *commandMap = rawCommandMap
                           ? dynamic_cast<CommandMap *>(rawCommandMap.get())
                           : nullptr;
    if (!commandMap)
      property_host::typeError(
          "Cannot read properties of undefined (reading 'add')");
    const Value mtOptions = cx.get((*parsed)->options, key(u"mathtools"));
    const Value paired = cx.get(mtOptions, key(u"pairedDelimiters"));
    const ParseMethod pairedMethod = functions::fromProperty(property_host::get(
        MathtoolsMethods::initializeModule(runtime),
        property_host::debugText(u"PairedDelimiters")));
    for (const auto &name : cx.enumerableStringKeys(paired)) {
      const Value args = cx.get(paired, key(name));
      commandMap->add(Property(property_host::debugText(name)),
                      MacroRef(std::make_shared<Macro>(
                          Property(property_host::debugText(name)),
                          pairedMethod, runtime.properties.from(args))));
    }
    if (property_host::truthy(runtime.properties.from(
            cx.get(mtOptions, key(u"legacycolonsymbols"))))) {
      HandlerConfig legacy;
      legacy.setOwn("macro", Property(PropertyArray{
                                 Property("mathtools-legacycolonsymbols")}));
      (**handlers)
          .add(legacy, FallbackConfig{}, MathtoolsMethods::LEGACYPRIORITY);
    }
    const auto callable = runtime.functions().find(tagFormat);
    const auto configurationIdentity =
        runtime.parserConfigurationIdentities.find(*configuration);
    if (!callable || !configurationIdentity)
      throw Exception(
          "TypeError",
          u"MathtoolsTagFormat canonical identities are unavailable");
    const std::array<Value, 2> tagArguments{
        Value(*configurationIdentity), Value((*jax)->instanceIdentity())};
    (void)runtime.context().call(*callable, Value(Undefined{}), tagArguments);
    return std::monostate{};
  };
  OptionCallBinding binding;
  binding.receiver = [](Context &, Value) {
    return FunctionReceiver{Missing{}};
  };
  binding.arguments = [&runtime](Context &, std::span<const Value> args) {
    if (args.size() < 2)
      throw Exception("TypeError",
                      u"Mathtools config requires (configuration, TeX)");
    const auto *config = std::get_if<ObjectRef>(&args[0].data);
    const auto *jax = std::get_if<ObjectRef>(&args[1].data);
    if (!config || !jax)
      throw Exception("TypeError", u"Mathtools config identities are invalid");
    return std::vector<std::any>{
        std::any(runtime.parserConfigurationIdentities.resolve(*config)),
        std::any(runtime.texIdentities.resolve(*jax).get())};
  };
  binding.result = [](Context &, const std::any &) {
    return Value(Undefined{});
  };
  const Value bound =
      runtime.functions().bind(body, std::move(binding), u"configMathtools");
  if (const auto *identity = std::get_if<ObjectRef>(&bound.data))
    runtime.context().define(*identity, Key(u"length"),
                             Descriptor::data(Value(2), false, false, true));
  return bound;
}
Value postprocessor(ParserRuntime &runtime) {
  auto body = std::make_shared<Function>();
  body->sourceMetadata = FunctionSourceMetadata{u"fixPrescripts", 1};
  body->call = [](const FunctionReceiver &, FunctionArgs args) -> std::any {
    if (args.empty())
      throw Exception("TypeError", u"Mathtools postprocessor data is missing");
    ParseOptionsRef options;
    if (const auto *value = std::any_cast<ParseOptionsRef>(&args[0]);
        value && *value)
      options = *value;
    else if (const auto *data =
                 std::any_cast<core::InputJaxFilterDataRef>(&args[0]);
             data && *data) {
      const auto current=(*data)->currentData();
      if (const auto *value = std::any_cast<ParseOptionsRef>(&current);
          value && *value)
        options = *value;
    }
    if (!options)
      throw Exception("TypeError",
                      u"Mathtools postprocessor requires ParseOptions");
    const auto nodes = options->getList(u"mmultiscripts");
    if (!nodes)
      return std::monostate{};
    for (std::size_t i = 0; i < nodes->size(); ++i) {
      const auto node = nodes->get(i);
      if (!property_host::truthy(
              NodeUtil::instance().getProperty(node, "fixPrescript")))
        continue;
      const auto children = NodeUtil::instance().getChildren(node);
      if (!children)
        property_host::typeError(
            "Cannot read properties of undefined (reading 'length')");
      int missing = 0;
      for (const std::size_t index : {1u, 2u})
        if (!NodeUtil::instance().getChildAt(node, index).isNode()) {
          const auto empty = options->nodeFactoryOwner().create(
              "node", FunctionArgs(
                          std::array<std::any, 1>{std::any(Property("none"))}));
          NodeUtil::instance().setChild(node, index, empty);
          ++missing;
        }
      if (missing == 2)
        children->splice(1, 2, {});
    }
    return std::monostate{};
  };
  OptionCallBinding binding;
  binding.receiver = [](Context &, Value) {
    return FunctionReceiver{Missing{}};
  };
  binding.arguments = [&runtime](Context &, std::span<const Value> args) {
    if (args.empty())
      throw Exception("TypeError", u"Mathtools postprocessor data is missing");
    return std::vector<std::any>{std::any(parseOptionsFor(runtime, args[0]))};
  };
  binding.result = [](Context &, const std::any &) {
    return Value(Undefined{});
  };
  const Value bound =
      runtime.functions().bind(body, std::move(binding), u"fixPrescripts");
  if (const auto *identity = std::get_if<ObjectRef>(&bound.data))
    runtime.context().define(*identity, Key(u"length"),
                             Descriptor::data(Value(1), false, false, true));
  return bound;
}
} // namespace
ConfigurationRef
MathtoolsConfiguration::initializeModule(ParserRuntime &runtime) {
  const auto loaded = runtime.configurationHandler.get(u"mathtools");
  if (const auto *previous = std::get_if<ConfigurationRef>(&loaded);
      previous && *previous)
    return *previous;
  const auto item = MultlinedItem::initializeModule(runtime);
  const auto itemIdentity = runtime.itemClassIdentities.find(item);
  if (!itemIdentity)
    throw Exception("TypeError",
                    u"MultlinedItem constructor identity was not registered");
  MathtoolsMappings::initializeModule(runtime);
  Context &cx = runtime.context();
  const Value handlers = record(
      cx, {{u"macro", array(cx, {Value(u"mathtools-macros"),
                                 Value(u"mathtools-delimiters")})},
           {u"environment", array(cx, {Value(u"mathtools-environments")})},
           {u"delimiter", array(cx, {Value(u"mathtools-delimiters")})},
           {u"character", array(cx, {Value(u"mathtools-characters")})}});
  const Value items = record(cx, {{u"multlined", Value(*itemIdentity)}});
  const Value mt = record(cx, {{u"multlined-gap", Value(u"1em")},
                               {u"multlined-pos", Value(u"c")},
                               {u"multlined-width", Value(u"")},
                               {u"firstline-afterskip", Value(u"")},
                               {u"lastline-preskip", Value(u"")},
                               {u"smallmatrix-align", Value(u"c")},
                               {u"shortvdotsadjustabove", Value(u".2em")},
                               {u"shortvdotsadjustbelow", Value(u".2em")},
                               {u"centercolon", Value(false)},
                               {u"centercolon-offset", Value(u".04em")},
                               {u"thincolon-dx", Value(u"-.04em")},
                               {u"thincolon-dw", Value(u"-.08em")},
                               {u"use-unicode", Value(false)},
                               {u"legacycolonsymbols", Value(false)},
                               {u"prescript-sub-format", Value(u"")},
                               {u"prescript-sup-format", Value(u"")},
                               {u"prescript-arg-format", Value(u"")},
                               {u"allow-mathtoolsset", Value(true)},
                               {u"pairedDelimiters", expandableEmpty(cx)},
                               {u"tagforms", expandableEmpty(cx)}});
  const Value packageOptions = record(cx, {{u"mathtools", mt}});
  const Property tagModuleExports = MathtoolsTags::moduleExports(runtime);
  const Property tagFormat =
      property_host::get(tagModuleExports,
                         property_host::debugText(u"MathtoolsTagFormat"));
  const auto *tagFunction = std::get_if<FunctionRef>(&tagFormat.value);
  if (!tagFunction || !*tagFunction)
    throw Exception("TypeError", u"MathtoolsTagFormat export is unavailable");
  const Value definition = record(
      cx, {{u"handler", handlers},
           {u"items", items},
           {u"options", packageOptions},
           {u"config", configFunction(runtime, *tagFunction)},
            {u"postprocessors",
             array(cx, {array(cx, {postprocessor(runtime), Value(-6.0)})})}});
  return Configuration::create(runtime, u"mathtools", definition);
}
} // namespace folia::math::tex
