module folia.math.input.tex.newcommand.util;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.token;
import folia.math.input.tex.unit_util;
import folia.math.input.tex.base_methods;
import folia.math.util.property_host;
import folia.math.input.tex.parser;
import folia.math.cancellation;
import folia.math.util.js_numeric_input;
import folia.math.util.regexp_binding;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
[[noreturn]] void error(std::u16string id, std::u16string_view message,
                        std::span<const Property> args = {}) {
  throw TexError(std::move(id), message, args);
}
std::u16string text(const Property &p) { return property_host::string(p); }
void eraseProperty(const Property &receiver, std::string_view key) {
  auto object = PropertyList::record(receiver);
  if (!object)
    property_host::typeError("Cannot convert undefined or null to object");
  object->erase(key);
}
ParseOptions &config(TexParser &p) {
  if (const auto *c = std::get_if<ParseOptions *>(&p.configuration); c && *c)
    return **c;
  property_host::typeError(
      "Cannot read properties of undefined (reading 'handlers')");
}
SubHandlers &handlers(TexParser &p) {
  auto &c = config(p);
  if (const auto *h = std::get_if<std::shared_ptr<SubHandlers>>(&c.handlers);
      h && *h)
    return **h;
  property_host::typeError(
      "Cannot read properties of undefined (reading 'get')");
}
AbstractParseMap &dynamicMap(TexParser &p, std::u16string_view name,
                             std::string_view handler) {
  auto h = handlers(p).get(handler);
  if (!h)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'retrieve')");
  const TokenMapRef map = h->retrieve(Property(property_host::debugText(name)));
  auto *result = dynamic_cast<AbstractParseMap *>(map.get());
  if (!result)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'add')");
  return *result;
}
Property parserStackEnv(TexParser &parser);
std::vector<std::shared_ptr<AbstractParseMap>>
selectMaps(TexParser &parser, std::span<const std::u16string> tokens,
           std::span<const std::u16string_view> names,
           std::span<const std::string_view> handlerNames) {
  const Property env = parserStackEnv(parser);
  if (property_host::truthy(property_host::get(env, "isGlobal"))) {
    auto &options = config(parser);
    const auto *packageData =
        std::get_if<std::shared_ptr<SourceStringMap<Value>>>(
            &options.packageData);
    if (!packageData || !*packageData)
      property_host::typeError(
          "Cannot read properties of undefined (reading 'get')");
    const auto package = (*packageData)->get(u"begingroup");
    if (!package)
      property_host::typeError(
          "Cannot read properties of undefined (reading 'stack')");
    auto &rt = options.runtime();
    const Value stack = rt.context().get(*package, Key(u"stack"));
    const auto *identity = std::get_if<ObjectRef>(&stack.data);
    if (!identity)
      property_host::typeError(
          "Cannot read properties of undefined (reading 'checkGlobal')");
    const GlobalDefinitionScopeRef scope =
        rt.globalDefinitionScopeIdentities.resolve(*identity);
    PropertyArray tokenValues;
    for (const auto token : tokens)
      tokenValues.push(property_host::debugText(token));
    PropertyArray mapValues;
    for (const auto name : names)
      mapValues.push(property_host::debugText(name));
    const auto selected =
        scope->checkGlobal(std::move(tokenValues), std::move(mapValues));
    std::vector<std::shared_ptr<AbstractParseMap>> result;
    result.reserve(selected.size());
    for (const auto &candidate : selected) {
      auto map = std::dynamic_pointer_cast<AbstractParseMap>(candidate);
      if (!map)
        property_host::typeError(
            "Cannot read properties of undefined (reading 'add')");
      result.push_back(std::move(map));
    }
    return result;
  }
  if (names.size() != handlerNames.size())
    property_host::typeError("Invalid token-map selection");
  auto &sub = handlers(parser);
  std::vector<std::shared_ptr<AbstractParseMap>> result;
  result.reserve(names.size());
  for (std::size_t i = 0; i < names.size(); ++i) {
    auto handler = sub.get(handlerNames[i]);
    if (!handler)
      property_host::typeError(
          "Cannot read properties of undefined (reading 'retrieve')");
    const auto map =
        handler->retrieve(Property(property_host::debugText(names[i])));
    auto parsed = std::dynamic_pointer_cast<AbstractParseMap>(map);
    if (!parsed)
      property_host::typeError(
          "Cannot read properties of undefined (reading 'add')");
    result.push_back(std::move(parsed));
  }
  return result;
}
Property parseProperty(const ParseMethod &method) {
  if (const auto *p = std::get_if<Property>(&method))
    return *p;
  return Property(std::get<FunctionRef>(method));
}
Property parserStackEnv(TexParser &parser) {
  if (const auto *stack = std::get_if<std::shared_ptr<Stack>>(&parser.stack);
      stack && *stack)
    return (*stack)->env();
  property_host::typeError(
      "Cannot read properties of undefined (reading 'env')");
}
ParserRuntime &runtime(TexParser &parser) {
  const auto *po = std::get_if<ParseOptions *>(&parser.configuration);
  if (!po || !*po)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'runtime')");
  return (**po).runtime();
}
std::optional<regexp_host::Match>
match(TexParser &parser, std::u16string pattern, std::u16string input) {
  auto &rt = runtime(parser);
  const auto object = rt.regexp().create(std::move(pattern), u"i");
  return rt.regexp().exec(options::Value(object), input);
}
} // namespace
std::u16string NewcommandUtil::GetCSname(TexParser &parser,
                                         std::u16string cmd) {
  const std::u16string c = parser.GetNext();
  if (c != u"\\") {
    const std::array<Property, 1> args{Property(property_host::debugText(cmd))};
    error(u"MissingCS", u"%1 must be followed by a control sequence", args);
  }
  const Property argument = parser.GetArgument(cmd);
  const std::u16string cs =
      property_host::string(UnitUtil::instance().trimSpaces(argument));
  const std::u16string result = cs.empty() ? cs : cs.substr(1);
  checkProtectedMacros(parser, result);
  return result;
}
std::u16string NewcommandUtil::GetCsNameArgument(TexParser &parser,
                                                 std::u16string name) {
  std::u16string cs = property_host::string(
      UnitUtil::instance().trimSpaces(parser.GetArgument(name)));
  if (!cs.empty() && cs.front() == u'\\')
    cs.erase(cs.begin());
  const bool valid = match(parser, u"^(.|[a-z]+)$", cs).has_value();
  if (!valid) {
    const std::array<Property, 1> args{
        Property(property_host::debugText(name))};
    error(u"IllegalControlSequenceName",
          u"Illegal control sequence name for %1", args);
  }
  checkProtectedMacros(parser, cs);
  return cs;
}
Property NewcommandUtil::GetArgCount(TexParser &parser,
                                     std::u16string name) {
  Property n = parser.GetBrackets(name);
  if (property_host::truthy(n)) {
    const std::u16string value =
        property_host::string(UnitUtil::instance().trimSpaces(n));
    if (!match(parser, u"^[0-9]+$", value)) {
      const std::array<Property, 1> args{
          Property(property_host::debugText(name))};
      error(u"IllegalParamNumber",
            u"Illegal number of parameters specified in %1", args);
    }
    return property_host::debugText(value);
  }
  return n;
}
std::variant<double, PropertyArray>
NewcommandUtil::GetTemplate(TexParser &parser, std::u16string cmd,
                            std::u16string cs) {
  std::u16string c = parser.GetNext();
  std::vector<std::optional<std::u16string>> params;
  double n = 0;
  double i = parser.i;
  const std::u16string source = parser.string();
  while (parser.i < static_cast<double>(source.size())) {
    cancellation::checkpoint(cancellation::CheckpointStage::TeX);
    c = parser.GetNext();
    if (c == u"#") {
      if (i != parser.i)
        params.resize(static_cast<std::size_t>(n) + 1),
            params[static_cast<std::size_t>(n)] =
                source.substr(static_cast<std::size_t>(i),
                              static_cast<std::size_t>(parser.i - i));
      ++parser.i;
      const std::u16string digit =
          source.substr(static_cast<std::size_t>(std::max(0.0, parser.i)), 1);
      if (!match(parser, u"^[1-9]$", digit)) {
        const std::array<Property, 1> args{
            Property(property_host::debugText(cs))};
        error(u"CantUseHash2", u"Illegal use of # in template for %1", args);
      }
      const double parsed = std::stod(property_host::debugText(digit));
      ++n;
      if (parsed != n) {
        const std::array<Property, 1> args{
            Property(property_host::debugText(cs))};
        error(u"SequentialParam",
              u"Parameters for %1 must be numbered sequentially", args);
      }
      i = parser.i + 1;
    } else if (c == u"{") {
      if (i != parser.i) {
        if (params.size() <= static_cast<std::size_t>(n))
          params.resize(static_cast<std::size_t>(n) + 1);
        params[static_cast<std::size_t>(n)] =
            source.substr(static_cast<std::size_t>(i),
                          static_cast<std::size_t>(parser.i - i));
        const auto &current = *params[static_cast<std::size_t>(n)];
        std::size_t leadingIndex = 0;
        while (leadingIndex < current.size() && current[leadingIndex] == u' ')
          ++leadingIndex;
        std::u16string previous;
        for (std::size_t j = 0;
             j < static_cast<std::size_t>(n) && j < params.size(); ++j)
          if (params[j])
            previous += *params[j];
        if (leadingIndex == current.size() && previous.empty())
          return n;
      }
      if (!params.empty()) {
        PropertyArray result;
        result.push(property_host::debugText(
            property_host::text(std::to_string(static_cast<int>(n)))));
        for (const auto &entry : params) {
          if (entry)
            result.push(property_host::debugText(*entry));
          else
            result.resize(result.size() + 1);
        }
        return result;
      }
      return n;
    }
    ++parser.i;
  }
  const std::array<Property, 1> args{Property(property_host::debugText(cmd))};
  error(u"MissingReplacementString",
        u"Missing replacement string for definition of %1", args);
}
std::u16string NewcommandUtil::GetParameter(TexParser &parser,
                                            std::u16string name,
                                            Property paramValue) {
  if (paramValue.missing() ||
      std::holds_alternative<PropertyNull>(paramValue.value))
    return property_host::string(parser.GetArgument(name));
  const std::u16string param = property_host::string(paramValue);
  double i = parser.i, j = 0;
  bool hasBraces = false;
  const std::u16string source = parser.string();
  while (parser.i < static_cast<double>(source.size())) {
    cancellation::checkpoint(cancellation::CheckpointStage::TeX);
    const char16_t c = source[static_cast<std::size_t>(parser.i)];
    if (c == u'{') {
      hasBraces = parser.i == i;
      (void)parser.GetArgument(name);
      j = parser.i - i;
    } else if (MatchParam(parser, param)) {
      if (hasBraces) {
        ++i;
        j -= 2;
      }
      return source.substr(static_cast<std::size_t>(i),
                           static_cast<std::size_t>(j));
    } else if (c == u'\\') {
      ++parser.i;
      ++j;
      hasBraces = false;
      const auto start = static_cast<std::size_t>(std::max(0.0, parser.i));
      if (start < source.size()) {
        const char16_t first = source[start];
        std::size_t end = start + 1;
        const auto matched = match(parser, u"[a-z]+|.", source.substr(start));
        if (matched && matched->at(0))
          end = start + matched->at(0)->size();
        parser.i = static_cast<double>(end);
        j = parser.i - i;
      }
    } else {
      ++parser.i;
      ++j;
      hasBraces = false;
    }
  }
  const std::array<Property, 1> args{Property(property_host::debugText(name))};
  error(u"RunawayArgument", u"Runaway argument for %1?", args);
}
double NewcommandUtil::MatchParam(TexParser &parser,
                                  std::u16string_view param) {
  const std::u16string source = parser.string();
  const auto i = static_cast<std::size_t>(std::max(0.0, parser.i));
  if (i + param.size() > source.size() ||
      std::u16string_view(source).substr(i, param.size()) != param)
    return 0;
  const bool endsControl =
      match(parser, u"\\\\[a-z]+$", std::u16string(param)).has_value();
  if (endsControl && i + param.size() < source.size()) {
    const std::u16string next(1, source[i + param.size()]);
    if (match(parser, u"[a-z]", next))
      return 0;
  }
  parser.i += static_cast<double>(param.size());
  return 1;
}
void NewcommandUtil::checkProtectedMacros(TexParser &parser,
                                          std::u16string_view cs) {
  auto &config = *std::get<ParseOptions *>(parser.configuration);
  Context &context = config.runtime().context();
  const Value protectedMacros =
      context.get(config.options, Key(u"protectedMacros"));
  if (const auto *identity = std::get_if<ObjectRef>(&protectedMacros.data)) {
    const Value includes = context.get(protectedMacros, Key(u"includes"));
    const auto *fn = std::get_if<ObjectRef>(&includes.data);
    if (fn) {
      const std::array<Value, 1> args{Value(std::u16string(cs))};
      if (property_host::truthy(config.runtime().properties.from(
              context.call(*fn, protectedMacros, args)))) {
        const std::array<Property, 1> values{
            Property(property_host::debugText(u"\\" + std::u16string(cs)))};
        error(u"ProtectedMacro", u"The control sequence %1 can't be redefined",
              values);
      }
    }
  }
}
void NewcommandUtil::addDelimiter(TexParser &parser, std::u16string cs,
                                  std::u16string character,
                                  Property attributes) {
  const std::u16string name = cs.size() > 1 ? cs.substr(1) : std::u16string{};
  checkProtectedMacros(parser, name);
  const std::array<std::u16string, 2> tokens{name, cs};
  const std::array<std::u16string_view, 2> maps{NEW_COMMAND, NEW_DELIMITER};
  const std::array<std::string_view, 2> types{"macro", "delimiter"};
  auto selected = selectMaps(parser, tokens, maps, types);
  if (name != cs)
    selected[0]->remove(Property(property_host::debugText(name)));
  selected[1]->add(Property(property_host::debugText(cs)),
                   TokenRef(std::make_shared<Token>(
                       Property(property_host::debugText(cs)),
                       Property(property_host::debugText(character)),
                       std::move(attributes))));
  eraseProperty(parserStackEnv(parser), "isGlobal");
}
void NewcommandUtil::addMacro(TexParser &parser, std::u16string cs,
                              ParseMethod method, Property args,
                              std::u16string token) {
  checkProtectedMacros(parser, cs);
  const std::array<std::u16string, 1> tokens{cs};
  const std::array<std::u16string_view, 1> maps{NEW_COMMAND};
  const std::array<std::string_view, 1> types{"macro"};
  auto selected = selectMaps(parser, tokens, maps, types);
  auto &macros = *selected[0];
  undefineDelimiter(parser, u"\\" + cs);
  if (token.empty())
    token = cs;
  macros.add(Property(property_host::debugText(cs)),
             MacroRef(std::make_shared<Macro>(
                 Property(property_host::debugText(token)), std::move(method),
                 std::move(args))));
  eraseProperty(parserStackEnv(parser), "isGlobal");
}
void NewcommandUtil::addEnvironment(TexParser &parser, std::u16string env,
                                    ParseMethod method, Property args) {
  const std::array<std::u16string, 1> tokens{env};
  const std::array<std::u16string_view, 1> maps{NEW_ENVIRONMENT};
  const std::array<std::string_view, 1> types{"environment"};
  auto selected = selectMaps(parser, tokens, maps, types);
  auto &environments = *selected[0];
  environments.add(
      Property(property_host::debugText(env)),
      MacroRef(std::make_shared<Macro>(Property(property_host::debugText(env)),
                                       std::move(method), std::move(args))));
  eraseProperty(parserStackEnv(parser), "isGlobal");
}
void NewcommandUtil::undefineMacro(TexParser &parser, std::u16string cs) {
  const std::array<std::u16string, 1> tokens{cs};
  const std::array<std::u16string_view, 1> maps{NEW_COMMAND};
  const std::array<std::string_view, 1> types{"macro"};
  auto selected = selectMaps(parser, tokens, maps, types);
  auto &macros = *selected[0];
  macros.remove(Property(property_host::debugText(cs)));
  auto &handler = handlers(parser);
  const auto macroHandler = handler.get("macro");
  if (!macroHandler)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'applicable')");
  if (macroHandler->applicable(Property(property_host::debugText(cs)))) {
    auto &rt = config(parser).runtime();
    // Source's anonymous fallback is `function () { return FALLBACK; }`.
    // Keep its real zero-argument behavior while the typed parser path can
    // still invoke the same retained FunctionRef with its parser/atom slots.
    const FunctionRef fallback = std::make_shared<Function>();
    fallback->call = [](const FunctionReceiver &, FunctionArgs) -> std::any {
      return std::any(std::get<ParseSymbolRef>(SubHandler::FALLBACK));
    };
    OptionCallBinding binding;
    binding.receiver = [](options::Context &, options::Value) {
      return FunctionReceiver{Missing{}};
    };
    binding.arguments = [](options::Context &, std::span<const options::Value>) {
      return std::vector<std::any>{};
    };
    binding.result = [&rt](options::Context &, const std::any &result) -> options::Value {
      if (const auto *symbol = std::any_cast<ParseSymbolRef>(&result);
          symbol && *symbol)
        return options::Value(rt.bindParseSymbol(*symbol));
      if (const auto *property = std::any_cast<Property>(&result))
        return rt.properties.to(*property);
      throw options::Exception("TypeError", u"Fallback function returned an invalid result");
    };
    const options::Value bound = rt.functions().bind(fallback, std::move(binding), u"");
    const auto *identity = std::get_if<options::ObjectRef>(&bound.data);
    if (!identity)
      throw options::Exception("TypeError",
                      u"Fallback macro has no Context function identity");
    config(parser).runtime().context().define(
        *identity, Key(u"length"),
        Descriptor::data(Value(0.0), false, false, true));
    macros.add(Property(property_host::debugText(cs)),
               MacroRef(std::make_shared<Macro>(
                   Property(property_host::debugText(cs)),
                   ParseMethod{fallback}, Property(PropertyArray{}))));
    undefineDelimiter(parser, u"\\" + cs);
  }
  eraseProperty(parserStackEnv(parser), "isGlobal");
}
void NewcommandUtil::undefineDelimiter(TexParser &parser, std::u16string cs) {
  const std::array<std::u16string, 1> tokens{cs};
  const std::array<std::u16string_view, 1> maps{NEW_DELIMITER};
  const std::array<std::string_view, 1> types{"delimiter"};
  auto selected = selectMaps(parser, tokens, maps, types);
  auto &delimiters = *selected[0];
  delimiters.remove(Property(property_host::debugText(cs)));
  auto &handler = handlers(parser);
  const auto delims = handler.get("delimiter");
  if (!delims)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'applicable')");
  if (delims->applicable(Property(property_host::debugText(cs))))
    delimiters.add(Property(property_host::debugText(cs)),
                   TokenRef(std::make_shared<Token>(
                       Property(property_host::debugText(cs)),
                       Property(PropertyNull{}), Property(PropertyList{}))));
  eraseProperty(parserStackEnv(parser), "isGlobal");
}
} // namespace folia::math::tex
