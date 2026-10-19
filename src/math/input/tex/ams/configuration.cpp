// Source-owned AMS package configuration.
module folia.math.input.tex.ams.configuration;
import folia.math.input.tex.ams.items;
import folia.math.input.tex.ams.methods;
import folia.math.input.tex.ams.mappings;
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.parser;
import folia.math.util.function_types;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Value object(
    Context &cx,
    std::initializer_list<std::pair<std::u16string_view, Value>> fields) {
  Value result(cx.object());
  for (const auto &[name, value] : fields)
    cx.set(result, Key(std::u16string(name)), value);
  return result;
}
Value array(Context &cx, std::initializer_list<Value> values) {
  const ObjectRef result = cx.array(static_cast<std::uint32_t>(values.size()));
  std::uint32_t i = 0;
  for (const auto &value : values) {
    const std::string digits = std::to_string(i++);
    cx.set(Value(result), Key(std::u16string(digits.begin(), digits.end())),
           value);
  }
  return Value(result);
}
} // namespace
ConfigurationRef AmsConfiguration::initializeModule(ParserRuntime &runtime) {
  const auto loaded = runtime.configurationHandler.get(u"ams");
  if (const auto *configuration =
          std::get_if<std::shared_ptr<Configuration>>(&loaded);
      configuration && *configuration)
    return *configuration;
  AmsItems::initializeModule(runtime);
  AmsMethods::initializeModule(runtime);
  AmsMappings::initializeModule(runtime);
  NewcommandConfiguration::initializeModule(runtime);
  const Value parent = runtime.abstractTagsClass;
  const auto *parentObject = std::get_if<ObjectRef>(&parent.data);
  if (!parentObject)
    throw Exception("TypeError", u"AbstractTags constructor is unavailable");
  const TagsClassRef amsTags = TagsFactory::registerSubclass(
      runtime, u"AmsTags",
      [&runtime]() { return std::make_shared<AmsTags>(runtime); },
      *parentObject);
  const auto constructor = runtime.tagClassIdentities.find(amsTags);
  if (!constructor)
    throw Exception("Unimplemented",
                    u"AmsTags constructor identity was not registered");
  Context &cx = runtime.context();
  const auto *itemsModule =
      std::get_if<ObjectRef>(&runtime.amsItemsModule.data);
  if (!itemsModule)
    throw Exception("TypeError", u"AmsItems module exports are unavailable");
  const Value multline = cx.get(Value(*itemsModule), Key(u"MultlineItem"));
  const Value flalign = cx.get(Value(*itemsModule), Key(u"FlalignItem"));
  const Value handlers = object(
      cx,
      {{u"character", array(cx, {Value(u"AMSmath-operatorLetter")})},
       {u"delimiter", array(cx, {Value(u"AMSsymbols-delimiter"),
                                 Value(u"AMSmath-delimiter")})},
       {u"macro",
        array(cx,
              {Value(u"AMSsymbols-mathchar0mi"),
               Value(u"AMSsymbols-mathchar0mo"), Value(u"AMSsymbols-delimiter"),
               Value(u"AMSsymbols-macros"), Value(u"AMSmath-mathchar0mo"),
               Value(u"AMSmath-macros"), Value(u"AMSmath-delimiter")})},
       {u"environment", array(cx, {Value(u"AMSmath-environment")})}});
  const Value items =
      object(cx, {{u"multline", multline}, {u"flalign", flalign}});
  const Value tags = object(cx, {{u"ams", Value(*constructor)}});
  const Value amsOptions =
      object(cx, {{u"operatornamePattern",
                   Value(runtime.regexp().create(u"^[-*a-zA-Z0-9]+"))},
                  {u"multlineWidth", Value(u"100%")},
                  {u"multlineIndent", Value(u"1em")}});
  const Value options =
      object(cx, {{u"multlineWidth", Value(u"")}, {u"ams", amsOptions}});
  const ParserShared<Configuration> newcommand =
      runtime.configurationHandler.get(u"newcommand");
  const auto *package =
      std::get_if<std::shared_ptr<Configuration>>(&newcommand);
  if (!package || !*package)
    throw Exception("Unimplemented",
                    u"The loaded Newcommand configuration is unavailable");
  const Value configCallback = (*package)->config();
  const Value definitions = object(cx, {{u"handler", handlers},
                                        {u"items", items},
                                        {u"tags", tags},
                                        {u"options", options},
                                        {u"config", configCallback}});
  return Configuration::create(runtime, u"ams", definitions);
}
} // namespace folia::math::tex
