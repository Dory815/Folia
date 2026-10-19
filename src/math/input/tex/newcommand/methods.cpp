module folia.math.input.tex.newcommand.methods;
import folia.math.input.tex.parser;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.newcommand.items;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.macro;
import folia.math.input.tex.token_map;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.stack_item_factory;
import folia.math.input.tex.unit_util;
import folia.math.input.tex.tex_error;
import folia.math.util.property_host;
import folia.math.util.js_numeric_input;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
TexParser &parserFrom(ParserContext &context) {
  auto *p = dynamic_cast<TexParser *>(&context);
  if (!p)
    property_host::typeError("Newcommand method requires a TexParser");
  return *p;
}
ParserRuntime &runtimeFrom(TexParser &parser) {
  const auto *po = std::get_if<ParseOptions *>(&parser.configuration);
  if (!po || !*po)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'runtime')");
  return (**po).runtime();
}
Property atom(const ParseAtom &value) {
  if (const auto *p = std::get_if<Property>(&value))
    return *p;
  if (const auto *t = std::get_if<TokenRef>(&value); t && *t)
    return (*t)->token();
  property_host::typeError("Newcommand method requires a token");
}
Property arg(const ParseRest &rest, std::size_t n) {
  if (n >= rest.size())
    return Missing{};
  if (const auto *p = std::any_cast<Property>(&rest[n]))
    return *p;
  if (const auto *s = std::any_cast<std::u16string>(&rest[n]))
    return property_host::debugText(*s);
  if (const auto *d = std::any_cast<double>(&rest[n]))
    return *d;
  if (const auto *b = std::any_cast<bool>(&rest[n]))
    return *b;
  return Missing{};
}
void eraseProperty(const Property &receiver, std::string_view key) {
  auto object = PropertyList::record(receiver);
  if (!object)
    property_host::typeError("Cannot convert undefined or null to object");
  object->erase(key);
}
std::u16string string(const Property &p) { return property_host::string(p); }
std::size_t stringSliceStart(double index, std::size_t length) {
  if (std::isnan(index) || index == 0)
    return 0;
  if (index == std::numeric_limits<double>::infinity())
    return length;
  if (index == -std::numeric_limits<double>::infinity())
    return 0;
  const double integer = std::trunc(index);
  const double size = static_cast<double>(length);
  const double start = integer < 0 ? std::max(size + integer, 0.0)
                                   : std::min(integer, size);
  return static_cast<std::size_t>(start);
}
std::size_t stringSubstringEnd(double index, std::size_t length) {
  if (std::isnan(index) || index <= 0)
    return 0;
  if (index == std::numeric_limits<double>::infinity())
    return length;
  const double end = std::min(std::trunc(index), static_cast<double>(length));
  return static_cast<std::size_t>(end);
}
ParseMethod method(ParserRuntime &runtime, std::u16string_view name) {
  const Property p = NewcommandMethods::get(runtime, name);
  if (const auto *f = std::get_if<FunctionRef>(&p.value); f && *f)
    return *f;
  throw Exception("Unimplemented",
                  u"Newcommand canonical method is unavailable");
}
BaseItem *createItem(TexParser &parser, std::string_view kind,
                     FunctionArgs args = {}) {
  const auto shared = parser.itemFactory();
  const auto *f = std::get_if<std::shared_ptr<StackItemFactory>>(&shared);
  if (!f || !*f)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'create')");
  return (*f)->create(kind, tree::FactoryArgs(args));
}
void pushNull(TexParser &parser) {
  const auto factory = parser.itemFactory();
  const auto *owner = std::get_if<std::shared_ptr<StackItemFactory>>(&factory);
  if (!owner || !*owner)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'create')");
  BaseItem *item = (*owner)->create("null");
  parser.Push((*owner)->reference(item));
}
options::Value array(ParserRuntime &runtime, std::span<const Property> values) {
  Context &cx = runtime.context();
  const ObjectRef out = cx.array(static_cast<std::uint32_t>(values.size()));
  for (std::size_t i = 0; i < values.size(); ++i) {
    const std::string index = std::to_string(i);
    cx.set(Value(out), Key(std::u16string(index.begin(), index.end())),
           runtime.properties.to(values[i]));
  }
  return Value(out);
}
FunctionRef parseFn(ParseBody body) {
  return makeParseCallable(std::move(body));
}
void bindDynamicMethod(ParserRuntime &runtime, FunctionRef body,
                       std::u16string_view name, std::size_t length) {
  const Value bound = BaseMethods::bind(runtime, body, std::u16string(name));
  const auto *identity = std::get_if<ObjectRef>(&bound.data);
  if (!identity)
    throw Exception("TypeError",
                    u"Dynamic Newcommand method has no Context identity");
  runtime.context().define(
      *identity, Key(u"length"),
      Descriptor::data(Value(static_cast<double>(length)), false, false,
                       true));
}
} // namespace
Property NewcommandMethods::initializeModule(ParserRuntime &runtime) {
  if (!runtime.newcommandMethods.missing())
    return runtime.newcommandMethods;
  PropertyList methods;
  const auto add = [&](std::string name, FunctionRef body) {
    const Value bound = BaseMethods::bind(
        runtime, body, property_host::text(name));
    const auto *identity = std::get_if<ObjectRef>(&bound.data);
    if (!identity)
      throw Exception("TypeError",
                      u"Newcommand method has no Context identity");
    std::size_t length = 2;
    if (name == "MacroWithTemplate") length = 4;
    else if (name == "BeginEnv") length = 6;
    runtime.context().define(
        *identity, Key(u"length"),
        Descriptor::data(Value(static_cast<double>(length)), false, false,
                         true));
    methods.set(std::move(name), Property(std::move(body)));
  };
  add("NewCommand",
      parseFn([&runtime](const FunctionReceiver &, ParserContext &context,
                         const ParseAtom &nameAtom, ParseRest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string name = string(atom(nameAtom));
        const std::u16string cs =
            NewcommandUtil::GetCsNameArgument(parser, name);
        const Property n = NewcommandUtil::GetArgCount(parser, name);
        const Property optional = parser.GetBrackets(name);
        const Property definition = parser.GetArgument(name);
        const Property args = Property(PropertyArray{definition, n, optional});
        NewcommandUtil::addMacro(parser, cs, method(runtime, u"Macro"), args);
        pushNull(parser);
        return Property(Missing{});
      }));
  add("NewEnvironment",
      parseFn([&runtime](const FunctionReceiver &, ParserContext &context,
                         const ParseAtom &nameAtom, ParseRest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string name = string(atom(nameAtom));
        const std::u16string env = property_host::string(
            UnitUtil::instance().trimSpaces(parser.GetArgument(name)));
        const Property n = NewcommandUtil::GetArgCount(parser, name);
        const Property optional = parser.GetBrackets(name);
        const Property begin = parser.GetArgument(name),
                       end = parser.GetArgument(name);
        const Property args =
            Property(PropertyArray{Property(true), begin, end, n, optional});
        NewcommandUtil::addEnvironment(parser, env,
                                       method(runtime, u"BeginEnv"), args);
        pushNull(parser);
        return Property(Missing{});
      }));
  add("MacroDef",
      parseFn([&runtime](const FunctionReceiver &, ParserContext &context,
                         const ParseAtom &nameAtom, ParseRest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string name = string(atom(nameAtom));
        const std::u16string cs = NewcommandUtil::GetCSname(parser, name);
        auto params = NewcommandUtil::GetTemplate(parser, name, u"\\" + cs);
        const Property definition = parser.GetArgument(name);
        if (const auto *count = std::get_if<double>(&params)) {
          const std::array<Property, 2> args{definition, *count};
          NewcommandUtil::addMacro(
              parser, cs, method(runtime, u"Macro"),
              Property(PropertyArray{args[0], Property(*count)}));
        } else {
          const auto &templateArgs = std::get<PropertyArray>(params);
          PropertyArray args;
          args.push(definition);
          for (std::size_t i = 0; i < templateArgs.size(); ++i) {
            if (templateArgs.has(i))
              args.push(templateArgs.get(i));
            else
              args.resize(args.size() + 1);
          }
          NewcommandUtil::addMacro(parser, cs,
                                   method(runtime, u"MacroWithTemplate"),
                                   Property(std::move(args)));
        }
        pushNull(parser);
        return Property(Missing{});
      }));
  add("Let",
      parseFn([&runtime](const FunctionReceiver &, ParserContext &context,
                         const ParseAtom &nameAtom, ParseRest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string command = string(atom(nameAtom));
        const std::u16string cs = NewcommandUtil::GetCSname(parser, command);
        std::u16string c = parser.GetNext();
        if (c == u"=") {
          ++parser.i;
          c = parser.GetNext();
        }
        const auto *po = std::get_if<ParseOptions *>(&parser.configuration);
        if (!po || !*po)
          property_host::typeError("Cannot read properties of null or "
                                   "undefined (reading 'handlers')");
        auto *sub = std::get_if<std::shared_ptr<SubHandlers>>(&(**po).handlers);
        if (!sub || !*sub)
          property_host::typeError(
              "Cannot read properties of null or undefined (reading 'get')");
        pushNull(parser);
        if (c == u"\\") {
          const std::u16string other =
              NewcommandUtil::GetCSname(parser, command);
          if (cs == other)
            return Property(Missing{});
          const auto macros = (*sub)->get("macro");
          if (!macros)
            property_host::typeError(
                "Cannot read properties of undefined (reading 'applicable')");
          const auto macroMap =
              macros->applicable(property_host::debugText(other));
          if (dynamic_cast<MacroMap *>(macroMap.get())) {
            const auto value =
                macroMap->lookup(property_host::debugText(other));
            if (const auto *macro = std::get_if<MacroRef>(&value);
                macro && *macro) {
              NewcommandUtil::addMacro(parser, cs, (*macro)->func(),
                                       (*macro)->args(),
                                       string((*macro)->token()));
              return Property(Missing{});
            }
          }
          if (dynamic_cast<CharacterMapIdentity *>(macroMap.get()) &&
              !dynamic_cast<DelimiterMap *>(macroMap.get())) {
            const auto value =
                macroMap->lookup(property_host::debugText(other));
            if (const auto *token = std::get_if<TokenRef>(&value);
                token && *token) {
              const ParseMethod mapParser = macroMap->parser();
              const TokenRef captured = *token;
              const FunctionRef body =
                  parseFn([mapParser, captured](
                              const FunctionReceiver &receiver,
                              ParserContext &parserContext, const ParseAtom &,
                              ParseRest) -> ParseResult {
                    return invokeParseMethod(mapParser, receiver, parserContext,
                                             ParseAtom{captured});
                  });
              bindDynamicMethod(runtimeFrom(parser), body, u"method", 1);
              NewcommandUtil::addMacro(
                  parser, cs, body,
                  Property(PropertyArray{property_host::debugText(cs),
                                         (*token)->character()}));
              return Property(Missing{});
            }
          }
          const auto delimiters = (*sub)->get("delimiter");
          if (!delimiters)
            property_host::typeError(
                "Cannot read properties of undefined (reading 'lookup')");
          const auto value =
              delimiters->lookup(property_host::debugText(u"\\" + other));
          if (const auto *token = std::get_if<TokenRef>(&value);
              token && *token) {
            NewcommandUtil::addDelimiter(parser, u"\\" + cs,
                                         string((*token)->character()),
                                         (*token)->attributes());
            return Property(Missing{});
          }
          NewcommandUtil::checkProtectedMacros(parser, cs);
          NewcommandUtil::undefineMacro(parser, cs);
          NewcommandUtil::undefineDelimiter(parser, u"\\" + cs);
          return Property(Missing{});
        }
        ++parser.i;
        const auto delimiters = (*sub)->get("delimiter");
        if (!delimiters)
          property_host::typeError(
              "Cannot read properties of undefined (reading 'lookup')");
        const auto value = delimiters->lookup(property_host::debugText(c));
        if (const auto *token = std::get_if<TokenRef>(&value);
            token && *token) {
          NewcommandUtil::addDelimiter(parser, u"\\" + cs,
                                       string((*token)->character()),
                                       (*token)->attributes());
          return Property(Missing{});
        }
        NewcommandUtil::addMacro(
            parser, cs, method(runtime, u"Macro"),
            Property(PropertyArray{property_host::debugText(c)}));
        return Property(Missing{});
      }));
  add("MacroWithTemplate",
      parseFn([](const FunctionReceiver &, ParserContext &context,
                 const ParseAtom &nameAtom, ParseRest rest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string name = string(atom(nameAtom));
        const std::u16string definition = string(arg(rest, 0));
        const std::u16string countText = string(arg(rest, 1));
        const double count =
            js_input::parseInt(countText, js_input::IntegerRadix::Decimal);
        if (rest.size() > 2) {
          parser.GetNext();
          const Property first = arg(rest, 2);
          if (property_host::truthy(first) &&
              !NewcommandUtil::MatchParam(parser, string(first)))
            throw TexError(
                u"MismatchUseDef", u"Use of %1 doesn't match its definition",
                std::array<Property, 1>{property_host::debugText(name)});
          if (count) {
            std::vector<Property> values;
            for (double i = 0; i < count; ++i)
              values.push_back(
                  property_host::debugText(NewcommandUtil::GetParameter(
                      parser, name,
                      arg(rest, static_cast<std::size_t>(i + 3)))));
            const Value list = array(runtimeFrom(parser), values);
            const std::u16string expanded =
                ParseUtil::substituteArgs(parser, list, definition);
            const std::u16string suffix =
                parser.string().substr(static_cast<std::size_t>(
                    std::clamp(parser.i, 0.0,
                               static_cast<double>(parser.string().size()))));
            parser.setString(ParseUtil::addArgs(parser, expanded, suffix));
            parser.i = 0;
            ParseUtil::checkMaxMacros(parser);
            return Property(Missing{});
          }
        }
        const std::u16string suffix =
            parser.string().substr(static_cast<std::size_t>(std::clamp(
                parser.i, 0.0, static_cast<double>(parser.string().size()))));
        parser.setString(ParseUtil::addArgs(parser, definition, suffix));
        parser.i = 0;
        ParseUtil::checkMaxMacros(parser);
        return Property(Missing{});
      }));
  add("BeginEnv",
      parseFn([](const FunctionReceiver &, ParserContext &context,
                 const ParseAtom &beginAtom, ParseRest rest) -> ParseResult {
        auto &parser = parserFrom(context);
        StackItemRef beginRef;
        if (const auto *v = std::get_if<StackItemRef>(&beginAtom))
          beginRef = *v;
        auto *begin =
            beginRef ? dynamic_cast<BaseItem *>(beginRef.get()) : nullptr;
        if (!begin)
          property_host::typeError("Cannot read properties of null or "
                                   "undefined (reading 'getName')");
        const std::u16string name = property_host::string(
            stack_property_host::scalar(begin->getName()));
        Stack &stack = *std::get<std::shared_ptr<Stack>>(parser.stack);
        const Property closing = property_host::get(stack.env(), "closing");
        if (closing == property_host::debugText(name)) {
          eraseProperty(stack.env(), "closing");
          const Property beginN = property_host::get(stack.global, "beginEnv");
          if (property_host::truthy(beginN)) {
            property_host::set(stack.global, "beginEnv",
                               Property(property_host::number(beginN) - 1));
            const std::u16string endDef = string(arg(rest, 1));
            if (!endDef.empty()) {
              const auto &source = parser.string();
              const std::u16string saved = source.substr(
                  stringSliceStart(parser.i, source.size()));
              const std::size_t prefixEnd =
                  stringSubstringEnd(parser.i, source.size());
              parser.setString(ParseUtil::addArgs(
                  parser, source.substr(0, prefixEnd), endDef));
              parser.Parse();
              parser.setString(saved);
              parser.i = 0;
            }
          }
          BaseItem *item = createItem(parser, "end");
          item->setProperty("name", property_host::debugText(name));
          const auto factory = parser.itemFactory();
          return std::get<std::shared_ptr<StackItemFactory>>(factory)
              ->reference(item);
        }
        const Property countValue = arg(rest, 2);
        const double count = js_input::parseInt(
            string(countValue), js_input::IntegerRadix::Decimal);
        std::u16string beginDef = string(arg(rest, 0)),
                       endDef = string(arg(rest, 1));
        if (property_host::truthy(countValue)) {
          std::vector<Property> values;
          const Property def = arg(rest, 3);
          if (!def.missing() &&
              !std::holds_alternative<PropertyNull>(def.value)) {
            const Property optional =
                parser.GetBrackets(u"\\begin{" + name + u"}");
            values.push_back(
                optional.missing() ||
                        std::holds_alternative<PropertyNull>(optional.value)
                    ? def
                    : optional);
          }
          for (double i = static_cast<double>(values.size()); i < count; i += 1)
            values.push_back(parser.GetArgument(u"\\begin{" + name + u"}"));
          const Value list = array(runtimeFrom(parser), values);
          beginDef = ParseUtil::substituteArgs(parser, list, beginDef);
          endDef = ParseUtil::substituteArgs(
              parser, array(runtimeFrom(parser), std::span<const Property>{}),
              endDef);
        }
        const std::u16string suffix =
            parser.string().substr(static_cast<std::size_t>(std::clamp(
                parser.i, 0.0, static_cast<double>(parser.string().size()))));
        parser.setString(ParseUtil::addArgs(parser, beginDef, suffix));
        parser.i = 0;
        const Property old = property_host::get(stack.global, "beginEnv");
        property_host::set(
            stack.global, "beginEnv",
            Property(
                (property_host::truthy(old) ? property_host::number(old) : 0) +
                1));
        BaseItem *item = createItem(parser, "beginEnv");
        item->setProperty("name", property_host::debugText(name));
        const auto factory = parser.itemFactory();
        return std::get<std::shared_ptr<StackItemFactory>>(factory)->reference(
            item);
      }));
  const Property base = BaseMethods::get(runtime, u"Macro");
  if (const auto *fn = std::get_if<FunctionRef>(&base.value); fn && *fn)
    methods.set("Macro", Property(*fn));
  else
    throw Exception("Unimplemented", u"Newcommand requires BaseMethods.Macro");
  runtime.newcommandMethods = Property(std::move(methods));
  return runtime.newcommandMethods;
}
Property NewcommandMethods::get(ParserRuntime &runtime,
                                std::u16string_view name) {
  if (runtime.newcommandMethods.missing())
    initializeModule(runtime);
  return property_host::get(runtime.newcommandMethods,
                            property_host::debugText(name));
}
} // namespace folia::math::tex
