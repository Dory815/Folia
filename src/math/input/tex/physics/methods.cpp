// Native port of MathJax 4.1.3 input/tex/physics/PhysicsMethods.ts.
module folia.math.input.tex.physics.methods;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.node_util;
import folia.math.input.tex.macro;
import folia.math.input.tex.node_factory;
import folia.math.input.tex.parser;
import folia.math.input.tex.stack_item_factory;
import folia.math.input.tex.stack_item_host;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.tex_error;
import folia.math.util.property_host;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view v) { return Key(std::u16string(v)); }
Property val(ParseRest rest, std::size_t i) {
  if (i >= rest.size())
    return Missing{};
  if (const auto *p = std::any_cast<Property>(&rest[i]))
    return *p;
  if (const auto *s = std::any_cast<std::u16string>(&rest[i]))
    return Property(property_host::debugText(*s));
  if (const auto *s = std::any_cast<std::string>(&rest[i]))
    return Property(*s);
  if (const auto *b = std::any_cast<bool>(&rest[i]))
    return Property(*b);
  if (const auto *n = std::any_cast<double>(&rest[i]))
    return Property(*n);
  return Missing{};
}
TexParser &parser(ParserContext &c) {
  auto *p = dynamic_cast<TexParser *>(&c);
  if (!p)
    property_host::typeError("Physics requires a TexParser");
  return *p;
}
std::u16string atom(const ParseAtom &a) {
  if (const auto *p = std::get_if<Property>(&a))
    return property_host::string(*p);
  if (const auto *t = std::get_if<TokenRef>(&a); t && *t)
    return property_host::string((*t)->token());
  property_host::typeError("Physics parse method requires a token name");
}
ParseOptions &config(TexParser &p) {
  auto *c = std::get_if<ParseOptions *>(&p.configuration);
  if (!c || !*c)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'nodeFactory')");
  return **c;
}
Stack &stack(TexParser &p) {
  auto *s = std::get_if<std::shared_ptr<Stack>>(&p.stack);
  if (!s || !*s)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'env')");
  return **s;
}
StackItemFactory &itemFactory(TexParser &p) {
  auto f = p.itemFactory();
  auto *s = std::get_if<std::shared_ptr<StackItemFactory>>(&f);
  if (!s || !*s)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'create')");
  return **s;
}
std::u16string str(ParseRest r, std::size_t i) {
  return property_host::string(val(r, i));
}
std::u16string numberText(double value) {
  const auto s = js_number::toString(value);
  std::u16string out;
  out.reserve(s.size());
  for (unsigned char c : s)
    out.push_back(static_cast<char16_t>(c));
  return out;
}
bool truth(ParseRest r, std::size_t i) {
  return property_host::truthy(val(r, i));
}
mml::NodeValue node(TexParser &p, std::u16string_view kind) {
  std::array<std::any, 1> a{std::any(Property(property_host::debugText(kind)))};
  return p.create(u"node", a);
}
mml::NodeValue node(TexParser &p, std::u16string_view kind, mml::Children c,
                    Property d = PropertyList{}) {
  std::array<std::any, 3> a{std::any(Property(property_host::debugText(kind))),
                            std::any(std::move(c)), std::any(std::move(d))};
  return p.create(u"node", a);
}
mml::NodeValue token(TexParser &p, std::u16string_view kind, Property d,
                     Property t) {
  std::array<std::any, 3> a{std::any(Property(property_host::debugText(kind))),
                            std::any(std::move(d)), std::any(std::move(t))};
  return p.create(u"token", a);
}
mml::Children children(std::initializer_list<mml::NodeValue> nodes) {
  auto a = std::make_shared<tree::ChildArray<mml::MmlNode>>();
  for (auto n : nodes)
    a->push_back(n);
  return a;
}
TexParserRef nested(TexParser &p, std::u16string s) {
  return TexParser::make(std::move(s), stack(p).env(), config(p));
}
ParseResult push(TexParser &p, mml::NodeValue n) {
  p.Push(std::move(n));
  return Property(Missing{});
}
ParseResult pushNested(TexParser &p, std::u16string s) {
  return push(p, nested(p, std::move(s))->mml());
}
BaseItem *createItem(TexParser &p, std::string_view kind) {
  return itemFactory(p).create(kind, {});
}
ParseResult pushAutoOpen(TexParser &p, std::u16string open,
                         std::u16string close, PropertyList extra = {}) {
  extra.setOwn("open", Property(property_host::debugText(open)));
  extra.setOwn("close", Property(property_host::debugText(close)));
  auto *item = createItem(p, "auto open");
  item->setProperties(StackProperty(std::move(extra)));
  p.Push(itemFactory(p).reference(item));
  return Property(Missing{});
}
void rewrite(TexParser &p, std::u16string replacement) {
  auto s = p.string();
  const auto i = static_cast<std::size_t>(
      std::clamp(p.i, 0.0, static_cast<double>(s.size())));
  s.replace(i, 0, replacement);
  p.setString(std::move(s));
}
FunctionRef method(ParseBody body) {
  return makeParseCallable(std::move(body));
}
bool inRange(std::uint32_t n, std::uint32_t a, std::uint32_t b) {
  return n >= a && n <= b;
}
const std::array<std::pair<char16_t, char16_t>, 5> pairs{
    {{u'(', u')'}, {u'[', u']'}, {u'{', u'}'}, {u'|', u'|'}, {u'\\', 0}}};
char16_t pairFor(std::u16string_view s) {
  if (s.empty())
    return 0;
  for (auto [a, b] : pairs)
    if (s.front() == a && b)
      return b;
  return 0;
}
bool isBigFence(std::u16string_view value) {
  return value == u"big" || value == u"bigg" || value == u"Big" ||
         value == u"Bigg";
}
std::u16string outputBraket(const std::array<std::u16string, 3> &a, bool star1,
                            bool star2) {
  if (star1 && star2)
    return u"\\left\\langle{" + a[0] + u"}\\middle\\vert{" + a[1] +
           u"}\\middle\\vert{" + a[2] + u"}\\right\\rangle";
  if (star1)
    return u"\\langle{" + a[0] + u"}\\vert{" + a[1] + u"}\\vert{" + a[2] +
           u"}\\rangle";
  return u"\\left\\langle{" + a[0] + u"}\\right\\vert{" + a[1] +
         u"}\\left\\vert{" + a[2] + u"}\\right\\rangle";
}
FunctionRef quantity() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest r) -> ParseResult {
    auto &p = parser(c);
    const auto name = atom(a);
    const auto open =
        property_host::string(val(r, 0).missing() ? Property("(") : val(r, 0));
    const auto close =
        property_host::string(val(r, 1).missing() ? Property(")") : val(r, 1));
    const bool arg = property_host::truthy(val(r, 2));
    const auto namedValue = val(r, 3);
    const auto variantValue = val(r, 4);
    // TypeScript default parameters apply when the argument is undefined.
    // Converting Missing first would turn it into the literal "undefined".
    const auto named = namedValue.missing()
                           ? std::u16string{}
                           : property_host::string(namedValue);
    const auto variant = variantValue.missing()
                             ? std::u16string{}
                             : property_host::string(variantValue);
    const bool star = arg ? p.GetStar() : false;
    auto next = p.GetNext();
    const auto position = p.i;
    std::u16string big;
    if (next == u"\\") {
      p.i++;
      big = p.GetCS();
      if (!isBigFence(big)) {
        auto empty = node(p, u"mrow");
        p.Push(ParseUtil::fenced(config(p), open, empty, close));
        p.i = position;
        return Property(Missing{});
      }
      next = p.GetNext();
    }
    const auto right = pairFor(next);
    if (arg && next != u"{")
      throw TexError(u"MissingArgFor", u"Missing argument for %1",
                     std::array<Property, 1>{p.currentCS()});
    if (!right) {
      auto empty = node(p, u"mrow");
      p.Push(ParseUtil::fenced(config(p), open, empty, close));
      p.i = position;
      return Property(Missing{});
    }
    if (!named.empty()) {
      auto m = token(p, u"mi",
                     Property(PropertyList{{"texClass", mml::TEXCLASS.OP}}),
                     Property(property_host::debugText(named)));
      if (!variant.empty())
        NodeUtil::instance().setAttribute(
            m, "mathvariant", Property(property_host::debugText(variant)));
      const std::any x(m);
      auto *fn = itemFactory(p).create("fn", tree::FactoryArgs(&x, 1));
      p.Push(itemFactory(p).reference(fn));
    }
    if (next == u"{") {
      auto argument = property_host::string(p.GetArgument(name));
      const auto left = arg ? open : std::u16string(u"\\{");
      const auto rightText = arg ? close : std::u16string(u"\\}");
      if (star)
        argument = left + u" " + argument + u" " + rightText;
      else if (!big.empty())
        argument = u"\\" + big + u"l" + left + u" " + argument + u" \\" + big +
                   u"r" + rightText;
      else
        argument = u"\\left" + left + u" " + argument + u" \\right" + rightText;
      return pushNested(p, std::move(argument));
    }
    ++p.i;
    return pushAutoOpen(
        p, next, std::u16string(1, right),
        PropertyList{{"big", big.empty()
                                 ? Property(PropertyNull{})
                                 : Property(property_host::debugText(big))}});
  });
}
FunctionRef eval() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest) -> ParseResult {
    auto &p = parser(c);
    const auto name = atom(a);
    const bool star = p.GetStar();
    const auto next = p.GetNext();
    if (next == u"(" || next == u"[") {
      ++p.i;
      PropertyList x{{"smash", star}, {"right", Property("\\vphantom{\\int}")}};
      return pushAutoOpen(p, next, u"|", std::move(x));
    }
    std::u16string replacement = u"\\left.\\vphantom{\\int}\\right|";
    if (next == u"{") {
      auto arg = property_host::string(p.GetArgument(name));
      replacement = u"\\left." + (star ? u"\\smash{" + arg + u"}" : arg) +
                    u"\\vphantom{\\int}\\right|";
    }
    rewrite(p, std::move(replacement));
    return Property(Missing{});
  });
}
FunctionRef commutator() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest r) -> ParseResult {
    auto &p = parser(c);
    const auto name = atom(a);
    const auto open =
        property_host::string(val(r, 0).missing() ? Property("[") : val(r, 0));
    const auto close =
        property_host::string(val(r, 1).missing() ? Property("]") : val(r, 1));
    const bool star = p.GetStar();
    auto next = p.GetNext();
    std::u16string big;
    if (next == u"\\") {
      ++p.i;
      big = p.GetCS();
      if (!isBigFence(big))
        throw TexError(u"MissingArgFor", u"Missing argument for %1",
                       std::array<Property, 1>{p.currentCS()});
      next = p.GetNext();
    }
    if (next != u"{")
      throw TexError(u"MissingArgFor", u"Missing argument for %1",
                     std::array<Property, 1>{p.currentCS()});
    auto a1 = property_host::string(p.GetArgument(name));
    auto a2 = property_host::string(p.GetArgument(name));
    auto body = a1 + u"," + a2;
    auto s = star
                 ? open + u" " + body + u" " + close
                 : (!big.empty()
                        ? u"\\" + big + u"l" + open + u" " + body + u" \\" +
                              big + u"r" + close
                        : u"\\left" + open + u" " + body + u" \\right" + close);
    return pushNested(p, std::move(s));
  });
}
FunctionRef starMacro() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest r) -> ParseResult {
    auto &p = parser(c);
    const auto name = atom(a);
    const bool star = p.GetStar();
    const double n = property_host::number(val(r, 0));
    std::vector<std::u16string> parts;
    for (std::size_t i = 1; i < r.size(); ++i)
      parts.push_back(property_host::string(val(r, i)));
    std::u16string macro;
    for (std::size_t i = 0; i < parts.size(); ++i) {
      if (i)
        macro += star ? u"*" : u"";
      macro += parts[i];
    }
    PropertyArray arguments;
    for (int i = 0; i < static_cast<int>(n); ++i)
      arguments.push(Property(property_host::debugText(
          property_host::string(p.GetArgument(name)))));
    auto &runtime = config(p).runtime();
    macro = ParseUtil::substituteArgs(
        p, runtime.properties.to(Property(arguments)), std::move(macro));
    p.setString(ParseUtil::addArgs(
        p, std::move(macro), p.string().substr(static_cast<std::size_t>(p.i))));
    p.i = 0;
    ParseUtil::checkMaxMacros(p);
    return Property(Missing{});
  });
}
FunctionRef operatorApplication(bool vector) {
  return method([vector](const FunctionReceiver &, ParserContext &c,
                         const ParseAtom &a, ParseRest r) -> ParseResult {
    auto &p = parser(c);
    const auto name = atom(a), op = str(r, 0);
    auto operation = nested(p, op)->mml();
    const std::any arg(operation);
    auto *item = itemFactory(p).create(vector ? "mml" : "fn",
                                       tree::FactoryArgs(&arg, 1));
    p.Push(itemFactory(p).reference(item));
    const auto left = p.GetNext();
    const auto right = pairFor(left);
    if (!right)
      return Property(Missing{});
    std::vector<std::u16string> fences;
    for (std::size_t i = 1; i < r.size(); ++i)
      fences.push_back(str(r, i));
    const bool enlarge = std::ranges::find(fences, left) != fences.end();
    if (left == u"{") {
      auto value = property_host::string(p.GetArgument(name));
      std::u16string macro = enlarge ? u"\\left\\{" : u"";
      macro += u" " + value + u" ";
      if (enlarge)
        macro += u"\\right\\}";
      p.setString(ParseUtil::addArgs(
          p, macro, p.string().substr(static_cast<std::size_t>(p.i))));
      p.i = 0;
      return Property(Missing{});
    }
    if (enlarge) {
      ++p.i;
      return pushAutoOpen(p, left, std::u16string(1, right));
    }
    return Property(Missing{});
  });
}
FunctionRef expression() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest r) -> ParseResult {
    auto &p = parser(c);
    auto name = atom(a);
    const bool opt = val(r, 0).missing() || property_host::truthy(val(r, 0));
    const auto idValue = val(r, 1);
    auto id = property_host::string(idValue);
    if (!property_host::truthy(idValue))
      id = name.size() > 1 ? name.substr(1) : name;
    const auto exp = opt ? p.GetBrackets(name) : Property(PropertyNull{});
    auto op =
        token(p, u"mi", Property(PropertyList{{"texClass", mml::TEXCLASS.OP}}),
              Property(property_host::debugText(id)));
    if (property_host::truthy(exp)) {
      auto sup = nested(p, property_host::string(exp))->mml();
      op = node(p, u"msup", children({op, sup}));
    }
    const std::any arg(op);
    auto *fn = itemFactory(p).create("fn", tree::FactoryArgs(&arg, 1));
    p.Push(itemFactory(p).reference(fn));
    if (p.GetNext() == u"(") {
      ++p.i;
      return pushAutoOpen(p, u"(", u")");
    }
    return Property(Missing{});
  });
}
FunctionRef vectorBold() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest) -> ParseResult {
    auto &p = parser(c);
    const auto name = atom(a);
    const bool star = p.GetStar();
    const auto argument = property_host::string(p.GetArgument(name));
    auto &parseOptions = config(p);
    auto factory = parseOptions.nodeFactoryOwnerRef();
    const Property oldToken = factory->get("token");
    const Property env = stack(p).env();
    const Property oldFont = property_host::get(env, "font");
    if (auto record = PropertyList::record(env))
      record->erase("font");
    const auto vectorFont =
        Property(property_host::debugText(star ? u"bold-italic" : u"bold"));
    property_host::set(env, "vectorFont", vectorFont);
    property_host::set(env, "vectorStar", Property(star));
    auto custom = std::make_shared<Function>();
    custom->call = [factory, env](const FunctionReceiver &,
                                  FunctionArgs args) -> std::any {
      const auto read = [&](std::size_t index) -> Property {
        if (index >= args.size())
          return Missing{};
        if (const auto *value = std::any_cast<Property>(&args[index]))
          return *value;
        property_host::typeError(
            "NodeFactory token argument is not a Property");
      };
      auto result =
          NodeFactory::createToken(*factory, read(1), read(2), read(3));
      const auto text = property_host::string(read(3));
      if (text.size() == 1) {
        const auto ch = static_cast<std::uint32_t>(text[0]);
        const bool ordinary =
            inRange(ch, 0x41, 0x5a) || inRange(ch, 0x61, 0x7a) ||
            inRange(ch, 0x391, 0x3a9) || inRange(ch, 0x30, 0x39) ||
            (inRange(ch, 0x3b1, 0x3c9) &&
             property_host::truthy(property_host::get(env, "vectorStar")));
        if (!property_host::truthy(property_host::get(env, "font")) &&
            property_host::truthy(property_host::get(env, "vectorFont")) &&
            (ordinary ||
             property_host::truthy(
                 NodeUtil::instance().getAttribute(result, "accent"))))
          NodeUtil::instance().setAttribute(
              result, "mathvariant", property_host::get(env, "vectorFont"));
      }
      return result;
    };
    factory->set("token", Property(FunctionRef(custom)));
    auto result = nested(p, argument)->mml();
    if (property_host::truthy(oldFont))
      property_host::set(env, "font", oldFont);
    else if (auto record = PropertyList::record(env))
      record->erase("font");
    if (auto record = PropertyList::record(env)) {
      record->erase("vectorFont");
      record->erase("vectorStar");
    }
    factory->set("token", oldToken);
    return push(p, result);
  });
}
FunctionRef qqtext() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest r) -> ParseResult {
    auto &p = parser(c);
    auto name = atom(a);
    const bool star = p.GetStar();
    const auto textValue = val(r, 0);
    auto text = property_host::string(textValue);
    if (!property_host::truthy(textValue))
      text = property_host::string(p.GetArgument(name));
    std::u16string replacement = star ? u"" : u"\\quad";
    replacement += u"\\text{" + text + u"}\\quad ";
    rewrite(p, std::move(replacement));
    return Property(Missing{});
  });
}
FunctionRef differential() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest r) -> ParseResult {
    auto &p = parser(c);
    auto name = atom(a), op = str(r, 0);
    const auto exponent = p.GetBrackets(name);
    const auto power = property_host::nullish(exponent)
                           ? std::u16string(u" ")
                           : u"^{" + property_host::string(exponent) + u"}";
    const bool parens = p.GetNext() == u"(", braces = p.GetNext() == u"{";
    std::u16string macro = op + power;
    if (!parens && !braces) {
      const auto argument = p.GetArgument(name, true);
      if (property_host::truthy(argument))
        macro += property_host::string(argument);
      return pushNested(p, std::move(macro));
    }
    if (braces) {
      macro += property_host::string(p.GetArgument(name));
      auto inner = nested(p, macro)->mml();
      return push(p,
                  node(p, u"TeXAtom", children({inner}),
                       Property(PropertyList{{"texClass", mml::TEXCLASS.OP}})));
    }
    pushNested(p, std::move(macro));
    ++p.i;
    return pushAutoOpen(p, u"(", u")");
  });
}
FunctionRef derivative() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest r) -> ParseResult {
    auto &p = parser(c);
    auto name = atom(a);
    const auto max = property_host::number(val(r, 0));
    const auto op = str(r, 1);
    const bool star = p.GetStar();
    const auto opt = p.GetBrackets(name);
    std::vector<std::u16string> args{
        property_host::string(p.GetArgument(name))};
    while (p.GetNext() == u"{" && args.size() < max)
      args.push_back(property_host::string(p.GetArgument(name)));
    bool ignore = false;
    std::u16string pow1 = u" ", pow2 = u" ";
    if (max > 2 && args.size() > 2) {
      pow1 = u"^{" +
             std::u16string(1, static_cast<char16_t>(u'0' + args.size() - 1)) +
             u"}";
      ignore = true;
    } else if (!property_host::nullish(opt)) {
      if (max > 2 && args.size() > 1)
        ignore = true;
      pow1 = pow2 = u"^{" + property_host::string(opt) + u"}";
    }
    const auto first = args.size() > 1 ? args[0] : u"";
    const auto second = args.size() > 1 ? args[1] : args[0];
    std::u16string rest;
    for (std::size_t i = 2; i < args.size() && !args[i].empty(); ++i)
      rest += op + u" " + args[i];
    std::u16string fraction = star ? u"\\flatfrac" : u"\\frac";
    fraction += u"{" + op + pow1 + first + u"}{" + op + u" " + second + pow2 +
                u" " + rest + u"}";
    pushNested(p, std::move(fraction));
    if (p.GetNext() == u"(") {
      ++p.i;
      return pushAutoOpen(p, u"(", u")", PropertyList{{"ignore", ignore}});
    }
    return Property(Missing{});
  });
}
FunctionRef simpleBraKet(std::string kind) {
  return method([kind = std::move(kind)](const FunctionReceiver &,
                                         ParserContext &c, const ParseAtom &a,
                                         ParseRest) -> ParseResult {
    auto &p = parser(c);
    auto name = atom(a);
    const bool star = p.GetStar();
    auto x = property_host::string(p.GetArgument(name));
    std::u16string macro;
    if (kind == "ket")
      macro = star ? u"\\vert{" + x + u"}\\rangle"
                   : u"\\left\\vert{" + x + u"}\\right\\rangle";
    else if (kind == "bra")
      macro = star ? u"\\langle{" + x + u"}\\vert"
                   : u"\\left\\langle{" + x + u"}\\right\\vert";
    else {
      std::optional<std::u16string> y;
      if (p.GetNext() == u"{")
        y = property_host::string(p.GetArgument(name, true));
      if (kind == "braket") {
        const auto ket = y ? *y : x;
        macro = star ? u"\\langle{" + x + u"}\\vert{" + ket + u"}\\rangle"
                     : u"\\left\\langle{" + x + u"}\\middle\\vert{" + ket +
                           u"}\\right\\rangle";
      } else {
        const auto bra = y ? *y : x;
        macro =
            star ? u"\\vert{" + x + u"}\\rangle\\!\\langle{" + bra + u"}\\vert"
                 : u"\\left\\vert{" + x +
                       u"}\\middle\\rangle\\!\\middle\\langle{" + bra +
                       u"}\\right\\vert";
      }
    }
    return pushNested(p, std::move(macro));
  });
}
FunctionRef matrixElement() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest) -> ParseResult {
    auto &p = parser(c);
    auto name = atom(a);
    const bool s1 = p.GetStar(), s2 = s1 && p.GetStar();
    std::array<std::u16string, 3> x{property_host::string(p.GetArgument(name)),
                                    property_host::string(p.GetArgument(name)),
                                    property_host::string(p.GetArgument(name))};
    return pushNested(p, outputBraket(x, s1, s2));
  });
}
FunctionRef expectation() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest) -> ParseResult {
    auto &p = parser(c);
    auto name = atom(a);
    const bool s1 = p.GetStar(), s2 = s1 && p.GetStar();
    auto op = property_host::string(p.GetArgument(name));
    std::u16string state;
    if (p.GetNext() == u"{")
      state = property_host::string(p.GetArgument(name, true));
    const auto m = (!op.empty() && !state.empty())
                       ? outputBraket({state, op, state}, s1, s2)
                       : (s1 ? u"\\langle {" + op + u"} \\rangle"
                             : u"\\left\\langle {" + op + u"} \\right\\rangle");
    return pushNested(p, m);
  });
}
FunctionRef matrixQuantity() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest r) -> ParseResult {
    auto &p = parser(c);
    auto name = atom(a);
    const bool star = p.GetStar();
    const auto next = p.GetNext();
    const std::u16string array = truth(r, 0) ? u"smallmatrix" : u"array";
    std::u16string arg, open, close;
    if (next == u"{")
      arg = property_host::string(p.GetArgument(name));
    else if (next == u"(" || next == u"[" || next == u"|") {
      ++p.i;
      open = next == u"(" ? (star ? u"\\lgroup" : u"(") : next;
      close = next == u"("   ? (star ? u"\\rgroup" : u")")
              : next == u"[" ? u"]"
                             : u"|";
      arg = p.GetUpTo(name, next == u"(" ? u")" : next);
    } else {
      open = u"(";
      close = u")";
    }
    std::u16string macro = open.empty() ? u"" : u"\\left";
    macro += open + u"\\begin{" + array + u"}{} " + arg + u"\\end{" + array +
             u"}" + (open.empty() ? u"" : u"\\right") + close;
    return pushNested(p, std::move(macro));
  });
}
FunctionRef identityMatrix() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest) -> ParseResult {
    auto &p = parser(c);
    const auto source = property_host::string(p.GetArgument(atom(a)));
    const double parsed =
        js_input::parseInt(source, js_input::IntegerRadix::Decimal);
    if (std::isnan(parsed))
      throw TexError(u"InvalidNumber", u"Invalid number");
    if (parsed <= 1) {
      p.setString(u"1" + p.string().substr(static_cast<std::size_t>(p.i)));
      p.i = 0;
      return Property(Missing{});
    }
    const auto size = static_cast<std::size_t>(parsed);
    std::u16string matrix;
    for (std::size_t i = 0; i < size; ++i) {
      if (i)
        matrix += u"\\\\ ";
      for (std::size_t j = 0; j < size; ++j) {
        if (j)
          matrix += u" & ";
        matrix += i == j ? u"1" : u"0";
      }
    }
    p.setString(matrix + p.string().substr(static_cast<std::size_t>(p.i)));
    p.i = 0;
    return Property(Missing{});
  });
}
FunctionRef xmatrix() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest) -> ParseResult {
    auto &p = parser(c);
    auto name = atom(a);
    const bool star = p.GetStar();
    const auto fill = property_host::string(p.GetArgument(name));
    const auto ns = property_host::string(p.GetArgument(name)),
               ms = property_host::string(p.GetArgument(name));
    double nd = js_input::parseInt(ns, js_input::IntegerRadix::Decimal),
           md = js_input::parseInt(ms, js_input::IntegerRadix::Decimal);
    if (std::isnan(nd) || std::isnan(md) || numberText(nd) != ns ||
        numberText(md) != ms)
      throw TexError(u"InvalidNumber", u"Invalid number");
    const auto n = static_cast<std::size_t>(std::max(nd, 1.0)),
               m = static_cast<std::size_t>(std::max(md, 1.0));
    std::u16string matrix;
    if (!star) {
      for (std::size_t i = 0; i < n; ++i) {
        if (i)
          matrix += u"\\\\ ";
        for (std::size_t j = 0; j < m; ++j) {
          if (j)
            matrix += u" & ";
          matrix += fill;
        }
      }
    } else if (n == 1 && m == 1)
      matrix = fill;
    else if (n == 1) {
      for (std::size_t j = 1; j <= m; ++j) {
        if (j > 1)
          matrix += u" & ";
        matrix += fill + u"_{" + numberText(static_cast<double>(j)) + u"}";
      }
    } else if (m == 1) {
      for (std::size_t i = 1; i <= n; ++i) {
        if (i > 1)
          matrix += u"\\\\ ";
        matrix += fill + u"_{" + numberText(static_cast<double>(i)) + u"}";
      }
    } else {
      for (std::size_t i = 1; i <= n; ++i) {
        if (i > 1)
          matrix += u"\\\\ ";
        for (std::size_t j = 1; j <= m; ++j) {
          if (j > 1)
            matrix += u" & ";
          matrix += fill + u"_{{" + numberText(static_cast<double>(i)) + u"}{" +
                    numberText(static_cast<double>(j)) + u"}}";
        }
      }
    }
    p.setString(matrix + p.string().substr(static_cast<std::size_t>(p.i)));
    p.i = 0;
    return Property(Missing{});
  });
}
FunctionRef pauli() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest) -> ParseResult {
    auto &p = parser(c);
    auto arg = property_host::string(p.GetArgument(atom(a)));
    std::u16string matrix = arg.size() > 1 ? arg.substr(1) : u"";
    if (!arg.empty())
      switch (arg[0]) {
      case u'0':
        matrix += u" 1 & 0\\\\ 0 & 1";
        break;
      case u'1':
      case u'x':
        matrix += u" 0 & 1\\\\ 1 & 0";
        break;
      case u'2':
      case u'y':
        matrix += u" 0 & -i\\\\ i & 0";
        break;
      case u'3':
      case u'z':
        matrix += u" 1 & 0\\\\ 0 & -1";
        break;
      }
    p.setString(matrix + p.string().substr(static_cast<std::size_t>(p.i)));
    p.i = 0;
    return Property(Missing{});
  });
}
FunctionRef diagonal() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest r) -> ParseResult {
    auto &p = parser(c);
    const auto name = atom(a);
    const bool anti = truth(r, 0);
    if (p.GetNext() != u"{")
      return Property(Missing{});
    const auto start = static_cast<std::size_t>(p.i);
    (void)p.GetArgument(name);
    const auto end = static_cast<std::size_t>(p.i);
    p.i = static_cast<double>(start + 1);
    std::vector<std::u16string> entries;
    std::size_t current = static_cast<std::size_t>(p.i);
    while (current < end) {
      try {
        entries.push_back(p.GetUpTo(name, u","));
        current = static_cast<std::size_t>(p.i);
      } catch (...) {
        p.i = static_cast<double>(end);
        entries.push_back(
            p.string().substr(current, end > 0 ? end - 1 - current : 0));
        break;
      }
    }
    std::u16string matrix;
    for (std::size_t i = 0; i < entries.size(); ++i) {
      if (i)
        matrix += u"\\\\ ";
      for (std::size_t j = 0; j < (anti ? entries.size() - i - 1 : i); ++j)
        matrix += u"&";
      matrix += u"\\mqty{" + entries[i] + u"}";
    }
    p.setString(matrix + p.string().substr(end));
    p.i = 0;
    return Property(Missing{});
  });
}
FunctionRef autoClose() {
  return method([](const FunctionReceiver &, ParserContext &c,
                   const ParseAtom &a, ParseRest r) -> ParseResult {
    auto &p = parser(c);
    const auto fence = std::get_if<Property>(&a)
                           ? property_host::string(std::get<Property>(a))
                           : u"";
    const auto cls = val(r, 0);
    auto top = stack(p).Top();
    auto *ref = std::get_if<StackItemRef>(&top);
    if (ref && *ref) {
      auto *base = dynamic_cast<BaseItem *>(ref->get());
      if (base && base->isKind("over"))
        top = stack(p).Top(Property(2));
    }
    ref = std::get_if<StackItemRef>(&top);
    auto *item = ref && *ref ? dynamic_cast<AutoOpen *>(ref->get()) : nullptr;
    if (!item || !item->closing(fence))
      return Property(false);
    auto mo = token(p, u"mo", Property(PropertyList{{"texClass", cls}}),
                    Property(property_host::debugText(fence)));
    auto *close = createItem(p, "close");
    close->setProperty("pre-autoclose", StackProperty(true));
    p.Push(itemFactory(p).reference(close));
    auto *mmlitem = createItem(p, "mml");
    mmlitem->Push(std::array<mml::NodeValue, 1>{mo});
    mmlitem->setProperty("autoclose", StackProperty(true));
    p.Push(itemFactory(p).reference(mmlitem));
    return Property(true);
  });
}
FunctionRef togglePush(bool arrow) {
  return method([arrow](const FunctionReceiver &, ParserContext &c,
                        const ParseAtom &, ParseRest) -> ParseResult {
    auto &p = parser(c);
    auto &runtime = config(p).runtime();
    const Property sourceOptions = runtime.properties.from(config(p).options);
    const Property physics = property_host::get(sourceOptions, "physics");
    const Property option =
        property_host::get(physics, arrow ? "arrowdel" : "italicdiff");
    const auto enabled = property_host::truthy(option);
    return pushNested(
        p, arrow ? (enabled ? u"\\vec{\\gradientnabla}" : u"{\\gradientnabla}")
                 : (enabled ? u"d" : u"{\\rm d}"));
  });
}
} // namespace
Property PhysicsMethodSet::get(std::u16string_view name) const {
  return property_host::get(methods, property_host::debugText(name));
}
PhysicsMethodSet PhysicsMethods::create(ParserRuntime &runtime) {
  PropertyList table;
  const auto add = [&](std::u16string_view name, FunctionRef f,
                       std::size_t length) {
    const auto bound = BaseMethods::bind(runtime, f, std::u16string(name));
    const auto *id = std::get_if<options::ObjectRef>(&bound.data);
    if (!id)
      throw options::Exception("TypeError",
                               u"Physics method has no Context identity");
    runtime.context().define(
        *id, options::Key(u"length"),
        options::Descriptor::data(options::Value(static_cast<double>(length)),
                                  false, false, true));
    table.setOwn(std::string(name.begin(), name.end()), Property(std::move(f)));
  };
  add(u"Quantity", quantity(), 7);
  add(u"Eval", eval(), 2);
  add(u"Commutator", commutator(), 4);
  add(u"VectorBold", vectorBold(), 2);
  add(u"StarMacro", starMacro(), 3);
  add(u"OperatorApplication", operatorApplication(false), 3);
  add(u"VectorOperator", operatorApplication(true), 3);
  add(u"Expression", expression(), 4);
  add(u"Qqtext", qqtext(), 3);
  add(u"Differential", differential(), 3);
  add(u"Derivative", derivative(), 4);
  const auto bra = method([](const FunctionReceiver &, ParserContext &context,
                             const ParseAtom &atomValue,
                             ParseRest) -> ParseResult {
    auto &active = parser(context);
    const auto name = atom(atomValue);
    const bool starBra = active.GetStar();
    const auto braText = property_host::string(active.GetArgument(name));
    std::u16string ketText;
    bool hasKet = false;
    bool starKet = false;
    if (active.GetNext() == u"\\") {
      const auto savedIndex = active.i;
      ++active.i;
      const auto command = active.GetCS();
      const auto found = active.lookup(HandlerType.get("MACRO"), command);
      if (const auto *macro = std::get_if<MacroRef>(&found);
          macro && *macro &&
          property_host::string((*macro)->token()) == u"ket") {
        hasKet = true;
        const auto afterCommand = active.i;
        starKet = active.GetStar();
        if (active.GetNext() == u"{") {
          ketText = property_host::string(active.GetArgument(command, true));
        } else {
          active.i = afterCommand;
          starKet = false;
        }
      } else {
        active.i = savedIndex;
      }
    }
    std::u16string expanded;
    if (hasKet) {
      expanded =
          (starBra || starKet)
              ? u"\\langle{" + braText + u"}\\vert{" + ketText + u"}\\rangle"
              : u"\\left\\langle{" + braText + u"}\\middle\\vert{" + ketText +
                    u"}\\right\\rangle";
    } else {
      expanded = starBra ? u"\\langle{" + braText + u"}\\vert"
                         : u"\\left\\langle{" + braText + u"}\\right\\vert";
    }
    return pushNested(active, std::move(expanded));
  });
  add(u"Bra", bra, 2);
  add(u"Ket", simpleBraKet("ket"), 2);
  add(u"BraKet", simpleBraKet("braket"), 2);
  add(u"KetBra", simpleBraKet("ketbra"), 2);
  add(u"Expectation", expectation(), 2);
  add(u"MatrixElement", matrixElement(), 2);
  add(u"MatrixQuantity", matrixQuantity(), 3);
  add(u"IdentityMatrix", identityMatrix(), 2);
  add(u"XMatrix", xmatrix(), 2);
  add(u"PauliMatrix", pauli(), 2);
  add(u"DiagonalMatrix", diagonal(), 3);
  add(u"AutoClose", autoClose(), 3);
  add(u"Vnabla", togglePush(true), 2);
  add(u"DiffD", togglePush(false), 2);
  table.setOwn("Macro", BaseMethods::get(runtime, u"Macro"));
  table.setOwn("NamedFn", BaseMethods::get(runtime, u"NamedFn"));
  table.setOwn("Array", BaseMethods::get(runtime, u"Array"));
  return {Property(std::move(table))};
}
} // namespace folia::math::tex
