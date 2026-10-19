// Native port of MathtoolsUtil.ts helpers over canonical parser/MML owners.
module folia.math.input.tex.mathtools.util;
import folia.math.input.tex.node_util;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.unit_util;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.stack;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.mathtools.methods;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
std::u16string argumentText(ParserRuntime &runtime, const FunctionArgs &args,
                            std::size_t index) {
  if (index >= args.size())
    return {};
  if (const auto *value = std::any_cast<Value>(&args[index]))
    return property_host::string(runtime.properties.from(*value));
  if (const auto *value = std::any_cast<Property>(&args[index]))
    return property_host::string(*value);
  return {};
}
TexParser &argumentParser(ParserRuntime &runtime, const FunctionArgs &args,
                          std::size_t index) {
  if (index >= args.size())
    property_host::typeError("MathtoolsUtil parser argument is missing");
  const auto *value = std::any_cast<Value>(&args[index]);
  const auto *object = value ? std::get_if<ObjectRef>(&value->data) : nullptr;
  if (!object)
    property_host::typeError("MathtoolsUtil parser argument is invalid");
  const auto parser = runtime.parserIdentities.resolve(*object);
  if (!parser)
    property_host::typeError("MathtoolsUtil parser argument is unavailable");
  return *parser;
}
mml::NodeValue argumentNode(ParserRuntime &runtime, const FunctionArgs &args,
                            std::size_t index) {
  if (index >= args.size())
    property_host::typeError("MathtoolsUtil MML argument is missing");
  const auto *value = std::any_cast<Value>(&args[index]);
  const auto *object = value ? std::get_if<ObjectRef>(&value->data) : nullptr;
  if (!object)
    property_host::typeError("MathtoolsUtil MML argument is invalid");
  return runtime.mmlNodeIdentities.resolve(*object);
}
FunctionRef exportedFunction(ParserRuntime &runtime, std::u16string name,
                             double length,
                             std::function<std::any(FunctionArgs)> call) {
  auto body = std::make_shared<Function>();
  body->call = [call = std::move(call)](const FunctionReceiver &,
                                        FunctionArgs args) -> std::any {
    return call(std::move(args));
  };
  OptionCallBinding binding;
  binding.receiver = [](Context &, Value) {
    return FunctionReceiver{Missing{}};
  };
  binding.arguments = [](Context &, std::span<const Value> args) {
    std::vector<std::any> result;
    result.reserve(args.size());
    for (const auto &value : args)
      result.emplace_back(value);
    return result;
  };
  binding.result = [](Context &, const std::any &result) {
    if (const auto *value = std::any_cast<Value>(&result))
      return *value;
    return Value(Undefined{});
  };
  const FunctionRef function = body;
  const Value identityValue =
      runtime.functions().bind(function, std::move(binding), name);
  const auto *identity = std::get_if<ObjectRef>(&identityValue.data);
  if (!identity)
    throw Exception("TypeError",
                    u"MathtoolsUtil function identity was not bound");
  runtime.context().define(*identity, Key(u"length"),
                           Descriptor::data(Value(length), false, false, true));
  return function;
}
} // namespace
Property MathtoolsUtil::moduleExports(ParserRuntime &runtime) {
  PropertyList exports;
  exports.set(
      "setDisplayLevel",
      Property(exportedFunction(
          runtime, u"setDisplayLevel", 2,
          [&runtime](FunctionArgs args) -> std::any {
            if (args.size() < 2)
              property_host::typeError(
                  "MathtoolsUtil.setDisplayLevel requires two arguments");
            setDisplayLevel(argumentNode(runtime, args, 0),
                            argumentText(runtime, args, 1));
            return std::monostate{};
          })));
  exports.set(
      "checkAlignment",
      Property(exportedFunction(
          runtime, u"checkAlignment", 2,
          [&runtime](FunctionArgs args) -> std::any {
            auto &parser = argumentParser(runtime, args, 0);
            (void)checkAlignment(parser, argumentText(runtime, args, 1));
            const auto *stack =
                std::get_if<std::shared_ptr<Stack>>(&parser.stack);
            if (!stack || !*stack)
              property_host::typeError("Cannot read properties of null or "
                                       "undefined (reading 'Top')");
            const StackEntry top = (**stack).Top();
            const auto *item = std::get_if<StackItemRef>(&top);
            if (!item || !*item)
              property_host::typeError(
                  "Alignment stack item has no canonical identity");
            const auto identity = runtime.stackItemIdentities.find(*item);
            if (!identity)
              throw Exception("Unimplemented",
                              u"Alignment stack item has no Context identity");
            return Value(*identity);
          })));
  exports.set(
      "addPairedDelims",
      Property(exportedFunction(
          runtime, u"addPairedDelims", 3,
          [&runtime](FunctionArgs args) -> std::any {
            if (args.size() < 3)
              property_host::typeError(
                  "MathtoolsUtil.addPairedDelims requires three arguments");
            const auto *definitions = std::any_cast<Value>(&args[2]);
            if (!definitions)
              property_host::typeError(
                  "Mathtools paired delimiter definition is invalid");
            addPairedDelims(argumentParser(runtime, args, 0),
                            argumentText(runtime, args, 1),
                            *definitions);
            return std::monostate{};
          })));
  exports.set("spreadLines",
              Property(exportedFunction(
                  runtime, u"spreadLines", 2,
                  [&runtime](FunctionArgs args) -> std::any {
                    if (args.size() < 2)
                      property_host::typeError(
                          "MathtoolsUtil.spreadLines requires two arguments");
                    spreadLines(argumentNode(runtime, args, 0),
                                argumentText(runtime, args, 1));
                    return std::monostate{};
                  })));
  exports.set("plusOrMinus",
              Property(exportedFunction(
                  runtime, u"plusOrMinus", 2,
                  [&runtime](FunctionArgs args) -> std::any {
                    if (args.size() < 2)
                      property_host::typeError(
                          "MathtoolsUtil.plusOrMinus requires two arguments");
                    return Value(plusOrMinus(argumentText(runtime, args, 0),
                                             argumentText(runtime, args, 1)));
                  })));
  exports.set(
      "getScript",
      Property(exportedFunction(
          runtime, u"getScript", 3, [&runtime](FunctionArgs args) -> std::any {
            if (args.size() < 3)
              property_host::typeError(
                  "MathtoolsUtil.getScript requires three arguments");
            const auto result = getScript(argumentParser(runtime, args, 0),
                                          argumentText(runtime, args, 1),
                                          argumentText(runtime, args, 2));
            if (!result.isNode())
              return Value(Null{});
            const auto identity =
                runtime.mmlNodeIdentities.find(result.requireNode());
            if (!identity)
              throw Exception("Unimplemented",
                              u"Mathtools script node has no Context identity");
            return Value(*identity);
          })));
  return Property(std::move(exports));
}
void MathtoolsUtil::setDisplayLevel(mml::NodeValue value,
                                    std::u16string_view style) {
  if (style.empty())
    return;
  bool display = false;
  int script = 0;
  bool found = true;
  if (style == u"\\displaystyle") {
    display = true;
    script = 0;
  } else if (style == u"\\textstyle") {
    display = false;
    script = 0;
  } else if (style == u"\\scriptstyle") {
    display = false;
    script = 1;
  } else if (style == u"\\scriptscriptstyle") {
    display = false;
    script = 2;
  } else
    found = false;
  if (!found)
    return;
  auto *node = value.nodeOrNull();
  if (!node)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'attributes')");
  const auto attributes = node->attributes();
  if (!attributes)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'set')");
  attributes->set("displaystyle", Property(display));
  attributes->set("scriptlevel", Property(static_cast<double>(script)));
}
EqnArrayItem &MathtoolsUtil::checkAlignment(TexParser &parser,
                                            std::u16string_view name) {
  const auto stackRef = std::get_if<std::shared_ptr<Stack>>(&parser.stack);
  if (!stackRef || !*stackRef)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'Top')");
  const auto top = (**stackRef).Top();
  auto *item = std::get_if<StackItemRef>(&top);
  if (!item || !*item)
    throw TexError(u"NotInAlignment",
                   u"%1 can only be used in aligment environments",
                   std::array<Property, 1>{property_host::debugText(name)});
  auto *array = dynamic_cast<EqnArrayItem *>(item->get());
  if (!array || array->kind() != "eqnarray")
    throw TexError(u"NotInAlignment",
                   u"%1 can only be used in aligment environments",
                   std::array<Property, 1>{property_host::debugText(name)});
  return *array;
}
void MathtoolsUtil::addPairedDelims(TexParser &parser, std::u16string cs,
                                    Value args) {
  const auto *configuration =
      std::get_if<ParseOptions *>(&parser.configuration);
  if (!configuration || !*configuration)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'handlers')");
  const auto *handlers =
      std::get_if<std::shared_ptr<SubHandlers>>(&(**configuration).handlers);
  if (!handlers || !*handlers)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'get')");
  const auto macro = (**handlers).get("macro");
  if (macro && macro->contains(Property(property_host::debugText(cs))))
    throw TexError(u"CommadExists", u"Command %1 already defined",
                   std::array<Property, 1>{
                       Property(property_host::debugText(u"\\" + cs))});
  const auto method = functions::fromProperty(property_host::get(
      MathtoolsMethods::initializeModule((**configuration).runtime()),
      property_host::debugText(u"PairedDelimiters")));
  NewcommandUtil::addMacro(
      parser, std::move(cs), method,
      (**configuration).runtime().properties.from(args));
}
void MathtoolsUtil::spreadLines(mml::NodeValue value,
                                std::u16string_view spread) {
  auto *node = value.nodeOrNull();
  if (!node || !node->isKind("mtable"))
    return;
  const Property row = NodeUtil::instance().getAttribute(value, "rowspacing");
  const std::u16string source = property_host::string(row);
  const double add =
      UnitUtil::instance().dimen2em(Property(property_host::debugText(spread)));
  std::u16string output;
  std::size_t pos = 0;
  bool first = true;
  while (true) {
    auto end = source.find(u' ', pos);
    if (end == std::u16string::npos)
      end = source.size();
    const double value =
        UnitUtil::instance().dimen2em(Property(property_host::debugText(
            std::u16string_view(source).substr(pos, end - pos))));
    if (!first)
      output.push_back(u' ');
    first = false;
    output += property_host::text(
        UnitUtil::instance().em(std::max(0.0, value + add)));
    if (end == source.size())
      break;
    pos = end + 1;
  }
  NodeUtil::instance().setAttribute(value, "rowspacing",
                                    Property(property_host::debugText(output)));
}
std::u16string MathtoolsUtil::plusOrMinus(std::u16string_view name,
                                          std::u16string value) {
  const auto whitespace = [](char16_t c) {
    return c == u'\t' || c == u'\n' || c == u'\v' || c == u'\f' || c == u'\r' ||
           c == u' ' || c == 0x00A0 || c == 0x1680 ||
           (c >= 0x2000 && c <= 0x200A) || c == 0x2028 || c == 0x2029 ||
           c == 0x202F || c == 0x205F || c == 0x3000 || c == 0xFEFF;
  };
  while (!value.empty() && whitespace(value.front()))
    value.erase(value.begin());
  while (!value.empty() && whitespace(value.back()))
    value.pop_back();
  std::size_t index = 0;
  if (index < value.size() && (value[index] == u'+' || value[index] == u'-'))
    ++index;
  bool digit = false;
  while (index < value.size() && value[index] >= u'0' && value[index] <= u'9') {
    digit = true;
    ++index;
  }
  if (index < value.size() && value[index] == u'.') {
    ++index;
    while (index < value.size() && value[index] >= u'0' &&
           value[index] <= u'9') {
      digit = true;
      ++index;
    }
  }
  if (!digit || index != value.size())
    throw TexError(u"NotANumber", u"Argument to %1 is not a number",
                   std::array<Property, 1>{property_host::debugText(name)});
  if (value.empty() || (value.front() != u'+' && value.front() != u'-'))
    value.insert(value.begin(), u'+');
  return value;
}
mml::NodeValue MathtoolsUtil::getScript(TexParser &parser,
                                        std::u16string_view name,
                                        std::u16string_view position) {
  const Property argument =
      UnitUtil::instance().trimSpaces(parser.GetArgument(std::u16string(name)));
  std::u16string source = property_host::string(argument);
  if (source.empty()) {
    const std::array<std::any, 1> args{std::any(Property("none"))};
    return parser.create(u"node", FunctionArgs(args));
  }
  const auto *configuration =
      std::get_if<ParseOptions *>(&parser.configuration);
  if (!configuration || !*configuration)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'options')");
  ParseOptions &options = **configuration;
  const auto *optionObject = std::get_if<ObjectRef>(&options.options.data);
  if (!optionObject)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'mathtools')");
  const Value mathtools =
      options.runtime().context().get(*optionObject, Key(u"mathtools"));
  const auto *mathtoolsObject = std::get_if<ObjectRef>(&mathtools.data);
  if (!mathtoolsObject)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'prescript-sub-format')");
  const std::u16string key =
      u"prescript-" + std::u16string(position) + u"-format";
  const Property format = options.runtime().properties.from(
      options.runtime().context().get(*mathtoolsObject, Key(key)));
  if (property_host::truthy(format))
    source = property_host::string(format) + u"{" + source + u"}";
  const auto *stackRef = std::get_if<std::shared_ptr<Stack>>(&parser.stack);
  if (!stackRef || !*stackRef)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'env')");
  const auto parsed = TexParser::make(source, (**stackRef).env(), options);
  const mml::NodeValue result = parsed->mml();
  if (auto *node = result.nodeOrNull();
      node && node->isKind("TeXAtom") && node->isEmpty()) {
    const std::array<std::any, 1> args{std::any(Property("none"))};
    return parser.create(u"node", FunctionArgs(args));
  }
  return result;
}
} // namespace folia::math::tex
