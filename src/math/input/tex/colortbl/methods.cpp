// Native implementation of TableColor from ColortblConfiguration.ts.
module folia.math.input.tex.colortbl.methods;
import folia.math.input.tex.parser;
import folia.math.input.tex.colortbl.items;
import folia.math.input.tex.stack_host;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view s) { return Key(std::u16string(s)); }
TexParser &parser(ParserContext &c) {
  auto *p = dynamic_cast<TexParser *>(&c);
  if (!p)
    property_host::typeError("TableColor requires TexParser");
  return *p;
}
ParseOptions &parseOptions(TexParser &p) {
  const auto *value = std::get_if<ParseOptions *>(&p.configuration);
  if (!value || !*value)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'packageData')");
  return **value;
}
std::u16string atomText(const ParseAtom &atom) {
  if (const auto *p = std::get_if<Property>(&atom))
    return property_host::string(*p);
  if (const auto *t = std::get_if<TokenRef>(&atom); t && *t)
    return property_host::string((*t)->token());
  property_host::typeError("TableColor macro name is not a token");
}
std::u16string restText(ParseRest rest) {
  if (rest.empty())
    property_host::typeError(
        "Cannot read properties of undefined (reading 'toLowerCase')");
  if (const auto *p = std::any_cast<Property>(&rest[0]))
    return property_host::string(*p);
  if (const auto *s = std::any_cast<std::u16string>(&rest[0]))
    return *s;
  property_host::typeError("TableColor type is not a string");
}
Value getColorModel(TexParser &p) {
  auto &runtime = parseOptions(p).runtime();
  auto &cx = runtime.context();
  const auto &packageData = parseOptions(p).packageData;
  const auto *map =
      std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&packageData);
  if (!map || !*map)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'get')");
  const auto entry = (*map)->get(u"color");
  if (!entry)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'model')");
  const auto *package = std::get_if<ObjectRef>(&entry->data);
  if (!package)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'model')");
  return cx.get(Value(*package), key(u"model"));
}
Value getColorMethod(Context &cx, const Value &model) {
  return cx.get(model, key(u"getColor"));
}
std::u16string callGetColor(Context &cx, const Value &model,
                            const Value &method, std::u16string modelName,
                            std::u16string definition) {
  const auto *function = std::get_if<ObjectRef>(&method.data);
  if (!function || cx.kind(*function) != ObjectKind::function)
    throw Exception("TypeError", u"lookup.getColor is not a function");
  const std::array<Value, 2> args{Value(std::move(modelName)),
                                  Value(std::move(definition))};
  const Value result = cx.call(*function, model, args);
  const auto *text = std::get_if<std::u16string>(&result.data);
  if (!text)
    throw Exception("TypeError",
                    u"ColorModel.getColor did not return a string");
  return *text;
}
ParseResult tableColor(const FunctionReceiver &, ParserContext &source,
                       const ParseAtom &rawName, ParseRest rest) {
  auto &p = parser(source);
  const auto name = atomText(rawName);
  const auto type = restText(rest);
  const Value model = getColorModel(p);
  const Property modelValue = p.GetBrackets(name, Property(""));
  auto &cx = parseOptions(p).runtime().context();
  const Value method = getColorMethod(cx, model);
  const Property definition = p.GetArgument(name);
  const auto color =
      callGetColor(cx, model, method, property_host::string(modelValue),
                   property_host::string(definition));
  const auto *stackValue = std::get_if<std::shared_ptr<Stack>>(&p.stack);
  if (!stackValue || !*stackValue)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'Top')");
  const StackEntry entry = (**stackValue).Top();
  auto *top =
      dynamic_cast<ColorArrayItem *>(&stack_host::item(entry, "isKind"));
  if (!top)
    throw TexError(u"UnsupportedTableColor", u"Unsupported use of %1",
                   std::array<Property, 1>{p.currentCS()});
  if (type == u"col") {
    const auto *row =
        std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&top->row);
    const auto *table =
        std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(
            &top->table);
    const std::size_t rowLength = row && *row ? (*row)->size() : 0;
    const bool hasTable = table && *table && !(*table)->empty();
    const std::size_t index = rowLength;
    if (hasTable && (index >= top->color.col.size() || !top->color.col[index] ||
                     *top->color.col[index] != color))
      throw TexError(u"ColumnColorNotTop",
                     u"%1 must be in the top row or preamble",
                     std::array<Property, 1>{property_host::debugText(name)});
    if (top->color.col.size() <= index)
      top->color.col.resize(index + 1);
    top->color.col[index] = color;
    const Property overlap = p.GetBrackets(name, Property(""));
    if (property_host::truthy(overlap))
      (void)p.GetBrackets(name, Property(""));
  } else {
    if (type == u"cell")
      top->color.cell = color;
    else if (type == u"row")
      top->color.row = color;
    else
      property_host::typeError("Unknown TableColor type");
    const auto *row =
        std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&top->row);
    if (type == u"row" &&
        (top->Size() != 0 || (row && *row && !(*row)->empty())))
      throw TexError(u"RowColorNotFirst",
                     u"%1 must be at the beginning of a row",
                     std::array<Property, 1>{property_host::debugText(name)});
  }
  return Property(Missing{});
}
} // namespace
ColortblMethodSet ColortblMethods::create(ParserRuntime &) {
  return {makeParseCallable(tableColor)};
}
} // namespace folia::math::tex
