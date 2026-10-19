// Native source port of MathtoolsMethods.ts. Methods retain the shared
// ParserRuntime and use the actual Base/AMS/Newcommand method owners.
module folia.math.input.tex.mathtools.methods;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.ams.methods;
import folia.math.input.tex.newcommand.methods;
import folia.math.input.tex.parser;
import folia.math.input.tex.node_util;
import folia.math.input.tex.stack;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.unit_util;
import folia.math.input.tex.mathtools.items;
import folia.math.input.tex.mathtools.util;
import folia.math.input.tex.mathtools.tags;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.macro;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.map_handler;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
TexParser &parser(ParserContext &value) {
  auto *result = dynamic_cast<TexParser *>(&value);
  if (!result)
    property_host::typeError("Mathtools method requires TexParser");
  return *result;
}
std::u16string text(const Property &value) {
  return property_host::string(value);
}
void eraseProperty(const Property &receiver, std::string_view key) {
  auto object = PropertyList::record(receiver);
  if (!object)
    property_host::typeError("Cannot convert undefined or null to object");
  object->erase(key);
}
Property arg(const ParseAtom &value) {
  if (const auto *item = std::get_if<Property>(&value))
    return *item;
  if (const auto *token = std::get_if<TokenRef>(&value); token && *token)
    return (*token)->token();
  if (const auto *item = std::get_if<StackItemRef>(&value); item && *item)
    return stack_property_host::scalar(
        stack_host::item(StackEntry(*item), "getName").getName());
  return Missing{};
}
std::u16string name(const ParseAtom &value) {
  if (const auto *item = std::get_if<Property>(&value))
    return text(*item);
  if (const auto *token = std::get_if<TokenRef>(&value); token && *token)
    return text((*token)->character());
  return u"";
}
Property restProperty(ParseRest values, std::size_t index) {
  if (index >= values.size())
    return Missing{};
  if (const auto *v = std::any_cast<Property>(&values[index]))
    return *v;
  if (const auto *v = std::any_cast<std::u16string>(&values[index]))
    return property_host::debugText(*v);
  if (const auto *v = std::any_cast<double>(&values[index]))
    return *v;
  if (const auto *v = std::any_cast<bool>(&values[index]))
    return *v;
  return Missing{};
}
std::u16string restText(ParseRest values, std::size_t index,
                        std::u16string fallback = {}) {
  const Property value = restProperty(values, index);
  return value.missing() ? fallback : text(value);
}
bool restBool(ParseRest values, std::size_t index, bool fallback = false) {
  const Property value = restProperty(values, index);
  return value.missing() ? fallback : property_host::truthy(value);
}
double restNumber(ParseRest values, std::size_t index, double fallback = 0) {
  const Property value = restProperty(values, index);
  return value.missing() ? fallback : property_host::number(value);
}
Stack &stack(TexParser &p) {
  const auto *value = std::get_if<std::shared_ptr<Stack>>(&p.stack);
  if (!value || !*value)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'Top')");
  return **value;
}
StackItemFactory &factory(TexParser &p) {
  const auto value = p.itemFactory();
  const auto *ref = std::get_if<std::shared_ptr<StackItemFactory>>(&value);
  if (!ref || !*ref)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'create')");
  return **ref;
}
ParseOptions &configuration(TexParser &p) {
  const auto *value = std::get_if<ParseOptions *>(&p.configuration);
  if (!value || !*value)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'options')");
  return **value;
}
template <class T>
std::shared_ptr<tree::ChildArray<T>>
array(const ParserShared<tree::ChildArray<T>> &value, std::string_view member) {
  if (const auto *result =
          std::get_if<std::shared_ptr<tree::ChildArray<T>>>(&value);
      result && *result)
    return *result;
  property_host::typeError(
      std::string("Cannot read properties of undefined (reading '") +
      std::string(member) + "')");
}
mml::Children children(std::initializer_list<mml::NodeValue> nodes = {}) {
  auto result = std::make_shared<tree::ChildArray<mml::MmlNode>>();
  for (const auto &node : nodes)
    result->push_back(node);
  return result;
}
mml::NodeValue node(TexParser &p, std::u16string kind, mml::Children kids,
                    Property attrs = PropertyList{}) {
  const std::array<std::any, 3> args{
      std::any(Property(property_host::debugText(kind))),
      std::any(std::move(kids)), std::any(std::move(attrs))};
  return p.create(u"node", FunctionArgs(args));
}
mml::NodeValue node(TexParser &p, std::u16string kind,
                    std::initializer_list<mml::NodeValue> kids,
                    Property attrs = PropertyList{}) {
  return node(p, std::move(kind), children(kids), std::move(attrs));
}
mml::NodeValue token(TexParser &p, std::u16string kind,
                     Property attrs = PropertyList{},
                     Property value = Missing{}) {
  const std::array<std::any, 3> args{
      std::any(Property(property_host::debugText(kind))),
      std::any(std::move(attrs)), std::any(std::move(value))};
  return p.create(u"token", FunctionArgs(args));
}
ParseMethod base(ParserRuntime &runtime, std::u16string_view member) {
  return functions::fromProperty(BaseMethods::get(runtime, member));
}
ParseResult call(ParseMethod method, TexParser &p, const ParseAtom &atom,
                 ParseRest rest = {}) {
  return invokeParseMethod(method, FunctionReceiver{Missing{}},
                           static_cast<ParserContext &>(p), atom, rest);
}
Property option(TexParser &p, std::u16string_view member) {
  const Value value = p.options();
  const auto *object = std::get_if<ObjectRef>(&value.data);
  if (!object)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'mathtools')");
  const Value mt =
      configuration(p).runtime().context().get(*object, Key(u"mathtools"));
  const auto *options = std::get_if<ObjectRef>(&mt.data);
  if (!options)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'mathtools')");
  return configuration(p).runtime().properties.from(
      configuration(p).runtime().context().get(*options,
                                               Key(std::u16string(member))));
}
FunctionRef bound(ParserRuntime &runtime, std::u16string_view label,
                  double length, ParseBody body) {
  auto function = makeParseCallable(std::move(body));
  const auto identity =
      BaseMethods::bind(runtime, function, std::u16string(label));
  const auto *object = std::get_if<ObjectRef>(&identity.data);
  if (!object)
    property_host::typeError(
        "Mathtools method binding did not return a function identity");
  runtime.context().define(*object, Key(u"length"),
                           Descriptor::data(Value(length), false, false, true));
  return function;
}
} // namespace
Property MathtoolsMethods::initializeModule(ParserRuntime &runtime) {
  if (!runtime.mathtoolsMethods.missing())
    return runtime.mathtoolsMethods;
  PropertyList methods;
  const Property baseMethods = BaseMethods::initializeModule(runtime);
  const Property amsMethods = AmsMethods::initializeModule(runtime);
  const Property newcommandMethods =
      NewcommandMethods::initializeModule(runtime);
  auto putForward = [&](std::string_view target, const Property &source,
                        std::u16string_view member) {
    methods.set(target,
                property_host::get(source, property_host::debugText(member)));
  };
  auto own = [&](std::string_view key, std::u16string_view label, double length,
                 ParseBody fn) {
    methods.set(key, Property(bound(runtime, label, length, std::move(fn))));
  };
  // Exact Source delegates remain the same canonical FunctionRef objects.
  putForward("Array", baseMethods, u"Array");
  putForward("Macro", baseMethods, u"Macro");
  putForward("xArrow", amsMethods, u"xArrow");
  putForward("HandleRef", amsMethods, u"HandleRef");
  putForward("AmsEqnArray", amsMethods, u"AmsEqnArray");
  putForward("MacroWithTemplate", newcommandMethods, u"MacroWithTemplate");
  own("MtMatrix", u"MtMatrix", 4,
      [&runtime](const FunctionReceiver &, ParserContext &source,
                 const ParseAtom &atom, ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        auto &begin = stack_host::item(
            StackEntry(std::get<StackItemRef>(atom)), "getName");
        const auto beginName =
            text(stack_property_host::scalar(begin.getName()));
        const Property align =
            p.GetBrackets(u"\\begin{" + beginName + u"}", Property("c"));
        std::vector<std::any> args{std::any(restProperty(rest, 0)),
                                   std::any(restProperty(rest, 1)),
                                   std::any(align)};
        return call(base(runtime, u"Array"), p, atom, args);
      });
  own("MtSmallMatrix", u"MtSmallMatrix", 5,
      [&runtime](const FunctionReceiver &, ParserContext &source,
                 const ParseAtom &atom, ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        Property align = restProperty(rest, 2);
        if (align.missing() || !property_host::truthy(align)) {
          const auto &begin = stack_host::item(
              StackEntry(std::get<StackItemRef>(atom)), "getName");
          align = p.GetBrackets(
              u"\\begin{" + text(stack_property_host::scalar(begin.getName())) +
                  u"}",
              option(p, u"smallmatrix-align"));
        }
        std::vector<std::any> args{
            std::any(restProperty(rest, 0)),
            std::any(restProperty(rest, 1)),
            std::any(align),
            std::any(Property(UnitUtil::instance().em(1.0 / 3))),
            std::any(Property(".2em")),
            std::any(Property("S")),
            std::any(Property(1))};
        return call(base(runtime, u"Array"), p, atom, args);
      });
  own("MtMultlined", u"MtMultlined", 2,
      [&runtime](const FunctionReceiver &, ParserContext &source,
                 const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &p = parser(source);
        const auto &begin = stack_host::item(
            StackEntry(std::get<StackItemRef>(atom)), "getName");
        const std::u16string env = text(
                                 stack_property_host::scalar(begin.getName())),
                             label = u"\\begin{" + env + u"}";
        Property pos = option(p, u"multlined-pos"),
                 width = option(p, u"multlined-width");
        if (!property_host::truthy(pos))
          pos = Property("c");
        if (!property_host::truthy(width))
          width = Property("");
        if (!p.nextIsSpace()) {
          Property argValue = p.GetBrackets(label, pos);
          const auto value = text(argValue);
          if (value.size() == 1 &&
              (value[0] == u'c' || value[0] == u't' || value[0] == u'b')) {
            pos = argValue;
            width = p.nextIsSpace() ? Property("")
                                    : p.GetBrackets(label, Property(""));
          } else
            width = argValue;
          if (property_host::truthy(width) &&
              !property_host::truthy(UnitUtil::instance().matchDimen(width)[0]))
            throw TexError(
                u"BadWidth", u"Width for %1 must be a dimension",
                std::array<Property, 1>{property_host::debugText(label)});
        }
        p.Push(std::get<StackItemRef>(atom));
        BaseItem *raw = factory(p).create("multlined", {});
        auto *item = dynamic_cast<MultlinedItem *>(raw);
        if (!item)
          property_host::typeError("MultlinedItem constructor is unavailable");
        Context &cx = runtime.context();
        ObjectRef definition = cx.object();
        cx.set(Value(definition), Key(u"displaystyle"), Value(true));
        cx.set(Value(definition), Key(u"rowspacing"), Value(u".5em"));
        cx.set(Value(definition), Key(u"width"),
               property_host::truthy(width) ? runtime.properties.to(width)
                                            : Value(u"auto"));
        cx.set(Value(definition), Key(u"columnwidth"), Value(u"100%"));
        item->arraydef = Value(definition);
        ParseUtil::setArrayAlign(*item, text(pos), &p);
        return factory(p).reference(item);
      });
  own("HandleShove", u"HandleShove", 3,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        const auto name = text(arg(atom));
        const auto shove = restText(rest, 0);
        const StackEntry top = stack(p).Top();
        BaseItem &value = stack_host::item(top, "kind");
        if (value.kind() != "multline" && value.kind() != "multlined")
          throw TexError(
              u"CommandInMultlined",
              u"%1 can only appear within the multline or multlined "
              u"environments",
              std::array<Property, 1>{property_host::debugText(name)});
        if (value.Size())
          throw TexError(
              u"CommandAtTheBeginingOfLine",
              u"%1 must come at the beginning of the line",
              std::array<Property, 1>{property_host::debugText(name)});
        (void)value.setProperty(
            "shove", StackProperty(Property(property_host::debugText(shove))));
        const Property shift = p.GetBrackets(name);
        mml::NodeValue mml = p.ParseArg(name);
        if (property_host::truthy(shift)) {
          auto row = node(p, u"mrow", {});
          auto space =
              node(p, u"mspace", {}, Property(PropertyList{{"width", shift}}));
          if (shove == u"left")
            NodeUtil::instance().appendChildren(row, children({space, mml}));
          else
            NodeUtil::instance().appendChildren(row, children({mml, space}));
          mml = row;
        }
        p.Push(mml);
        return Property(Missing{});
      });
  own("SpreadLines", u"SpreadLines", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        auto &begin = stack_host::item(
            StackEntry(std::get<StackItemRef>(atom)), "getName");
        Property env = stack(p).env();
        const Property closing = property_host::get(env, "closing");
        if (closing == stack_property_host::scalar(begin.getName())) {
          eraseProperty(env, "closing");
          const StackEntry entry = stack(p).Pop();
          BaseItem &item = stack_host::item(entry, "toMml");
          const mml::NodeValue mml = item.toMml();
          const std::u16string spread =
              text(stack_property_host::scalar(item.getProperty("spread")));
          if (auto *node = mml.nodeOrNull(); node && node->isInferred()) {
            const auto values = NodeUtil::instance().getChildren(mml);
            if (values)
              for (std::size_t i = 0; i < values->size(); ++i)
                if (values->has(i))
                  MathtoolsUtil::spreadLines(values->get(i), spread);
          } else
            MathtoolsUtil::spreadLines(mml, spread);
          p.Push(mml);
        } else {
          const std::u16string spread = p.GetDimen(
              u"\\begin{" + text(stack_property_host::scalar(begin.getName())) +
              u"}");
          (void)begin.setProperty(
              "spread", StackProperty(Property(property_host::debugText(spread))));
          (void)begin.setProperty("nestStart", StackProperty(Property(true)));
          ParseUtil::checkEqnEnv(p);
          p.Push(std::get<StackItemRef>(atom));
        }
        return Property(Missing{});
      });
  own("Cases", u"Cases", 5,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        auto *arrayItem =
            dynamic_cast<ArrayItem *>(factory(p).create("array", {}));
        if (!arrayItem)
          property_host::typeError("ArrayItem constructor is unavailable");
        arrayItem->setProperty("casesEnv", StackProperty(arg(atom)));
        Context &cx = configuration(p).runtime().context();
        ObjectRef def = cx.object();
        cx.set(Value(def), Key(u"rowspacing"), Value(u".2em"));
        cx.set(Value(def), Key(u"columnspacing"), Value(u"1em"));
        cx.set(Value(def), Key(u"columnalign"), Value(u"left"));
        if (restText(rest, 2) == u"D")
          cx.set(Value(def), Key(u"displaystyle"), Value(true));
        arrayItem->arraydef = Value(def);
        arrayItem->setProperties(StackProperty(
            Property(PropertyList{{"open", restProperty(rest, 0)},
                                  {"close", restProperty(rest, 1)}})));
        p.Push(std::get<StackItemRef>(atom));
        return factory(p).reference(arrayItem);
      });
  own("MathLap", u"MathLap", 4,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        const auto style = UnitUtil::instance().trimSpaces(
            p.GetBrackets(text(arg(atom)), Property("")));
        const auto pos = restText(rest, 0);
        const bool cramped = restBool(rest, 1);
        const mml::NodeValue argNode = p.ParseArg(text(arg(atom)));
        Property attrs(PropertyList{{"width", Property(0)}});
        if (pos != u"r")
          property_host::set(attrs, "lspace",
                             pos == u"l" ? Property("-1width")
                                         : Property("-.5width"));
        const auto padded = node(p, u"mpadded", {argNode}, attrs);
        const auto styled =
            node(p, u"mstyle", {padded},
                 Property(PropertyList{{"data-cramped", Property(cramped)}}));
        MathtoolsUtil::setDisplayLevel(styled, text(style));
        p.Push(node(p, u"TeXAtom", {styled}));
        return Property(Missing{});
      });
  own("Cramped", u"Cramped", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        const auto style = UnitUtil::instance().trimSpaces(
            p.GetBrackets(text(arg(atom)), Property("")));
        const auto styled =
            node(p, u"mstyle", {p.ParseArg(text(arg(atom)))},
                 Property(PropertyList{{"data-cramped", Property(true)}}));
        MathtoolsUtil::setDisplayLevel(styled, text(style));
        p.Push(styled);
        return Property(Missing{});
      });
  own("MtLap", u"MtLap", 3,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        const auto pos = restText(rest, 0);
        const std::u16string content =
            property_host::string(p.GetArgument(text(arg(atom))));
        const auto values = ParseUtil::internalMath(p, content, Property(0));
        const auto mml = node(p, u"mpadded", values,
                              Property(PropertyList{{"width", Property(0)}}));
        if (pos != u"r")
          NodeUtil::instance().setAttribute(
              mml, "lspace", Property(pos == u"l" ? "-1width" : "-.5width"));
        p.Push(mml);
        return Property(Missing{});
      });
  own("MathMakeBox", u"MathMakeBox", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        const auto macro = text(arg(atom));
        const Property width = p.GetBrackets(macro);
        const auto pos =
            property_host::string(p.GetBrackets(macro, Property("c")));
        const auto mml = node(p, u"mpadded", {p.ParseArg(macro)});
        if (property_host::truthy(width))
          NodeUtil::instance().setAttribute(mml, "width", width);
        std::u16string lower = pos;
        for (auto &c : lower)
          if (c >= u'A' && c <= u'Z')
            c = static_cast<char16_t>(c - u'A' + u'a');
        const std::u16string align = lower == u"c"   ? u"center"
                                     : lower == u"r" ? u"right"
                                                     : u"";
        if (!align.empty())
          NodeUtil::instance().setAttribute(
              mml, "data-align", Property(property_host::debugText(align)));
        if (lower != pos)
          NodeUtil::instance().setAttribute(mml, "data-overflow",
                                            Property("linebreak"));
        p.Push(mml);
        return Property(Missing{});
      });
  own("MathMBox", u"MathMBox", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        const auto macro = text(arg(atom));
        p.Push(node(p, u"mrow", {p.ParseArg(macro)}));
        return Property(Missing{});
      });
  own("UnderOverBracket", u"UnderOverBracket", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        const std::u16string macro = text(arg(atom));
        const double thickness = UnitUtil::instance().dimen2em(
            p.GetBrackets(macro, Property(".1em")));
        const Property height = p.GetBrackets(macro, Property(".2em"));
        const Property argument = p.GetArgument(macro);
        const bool over = macro.size() > 1 && macro[1] == u'o';
        const std::u16string position = over ? u"over" : u"under",
                             border = over ? u"bottom" : u"top";
        const std::string em = UnitUtil::instance().em(thickness);
        const std::u16string style = property_host::text("border: ") +
                                     property_host::text(em) +
                                     u" solid; border-" + border + u": none";
        const auto env = stack(p).env();
        auto parsed = TexParser::make(text(argument), env, configuration(p));
        const auto copy = TexParser::make(text(argument), env, configuration(p));
        const auto mphantom = node(p, u"mphantom", {copy->mml()});
        const auto script =
            node(p, u"mpadded", {mphantom},
                 Property(PropertyList{
                     {"style", Property(property_host::debugText(style))},
                     {"height", height},
                     {"depth", Property(0)}}));
        const auto result = ParseUtil::underOver(p, parsed->mml(), script,
                                                 position, true);
        const auto row = NodeUtil::instance().getChildAt(result, 0);
        const auto munderover = NodeUtil::instance().getChildAt(row, 0);
        NodeUtil::instance().setAttribute(
            munderover, over ? "accent" : "accentunder", Property(true));
        p.Push(result);
        return Property(Missing{});
      });
  own("ArrowBetweenLines", u"ArrowBetweenLines", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        const auto macro = text(arg(atom));
        auto &top = MathtoolsUtil::checkAlignment(p, macro);
        if (top.Size() || array(top.row, "length")->size())
          throw TexError(
              u"BetweenLines", u"%1 must be on a row by itself",
              std::array<Property, 1>{property_host::debugText(macro)});
        const bool star = p.GetStar();
        const std::u16string symbol = property_host::string(
            p.GetBrackets(macro, Property("\\Updownarrow")));
        if (star) {
          top.EndEntry();
          top.EndEntry();
        }
        const std::u16string tex =
            star ? u"\\quad" + symbol : symbol + u"\\quad";
        auto parsed = TexParser::make(tex, stack(p).env(), configuration(p));
        p.Push(parsed->mml());
        top.EndEntry();
        top.EndRow();
        return Property(Missing{});
      });
  own("VDotsWithin", u"VDotsWithin", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        const std::u16string macro = text(arg(atom));
        const std::u16string tex = u"\\mmlToken{mi}{}" +
                                   property_host::string(p.GetArgument(macro)) +
                                   u"\\mmlToken{mi}{}";
        auto parsed = TexParser::make(tex, stack(p).env(), configuration(p));
        const mml::NodeValue base = parsed->mml();
        const auto dots =
            node(p, u"mpadded",
                 {token(p, u"mo", PropertyList{},
                        Property(property_host::debugText(u"⋮")))},
                 Property(PropertyList{{"width", Property(0)},
                                       {"lspace", Property("-.5width")}}));
        const auto mphantom = node(p, u"mphantom", {base});
        p.Push(node(p, u"mpadded", {dots, mphantom},
                    Property(PropertyList{{"lspace", Property(".5width")}})));
        return Property(Missing{});
      });
  own("FlushSpaceAbove", u"FlushSpaceAbove", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        auto &top = MathtoolsUtil::checkAlignment(p, text(arg(atom)));
        const auto *table =
            std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(
                &top.table);
        if (table && *table) {
          top.setProperty("flushspaceabove",
                          StackProperty(static_cast<double>((*table)->size())));
          top.addRowSpacing(u"-" + property_host::string(
                                       option(p, u"shortvdotsadjustabove")));
        }
        return Property(Missing{});
      });
  own("FlushSpaceBelow", u"FlushSpaceBelow", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        auto &top = MathtoolsUtil::checkAlignment(p, text(arg(atom)));
        const auto *table =
            std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(
                &top.table);
        if (table && *table) {
          if (top.Size())
            top.EndEntry();
          top.EndRow();
          top.addRowSpacing(u"-" + property_host::string(
                                       option(p, u"shortvdotsadjustbelow")));
        }
        return Property(Missing{});
      });
  own("ShortVDotsWithin", u"ShortVDotsWithin", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        auto *top = dynamic_cast<ArrayItem *>(
            &stack_host::item(stack(p).Top(), "EndEntry"));
        const bool star = p.GetStar();
        if (top) {
          auto &aligned = MathtoolsUtil::checkAlignment(p, u"\\MTFlushSpaceAbove");
          const auto *table = std::get_if<
              std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(
              &aligned.table);
          if (table && *table) {
            aligned.setProperty("flushspaceabove", StackProperty(
                static_cast<double>((*table)->size())));
            aligned.addRowSpacing(
                u"-" + property_host::string(
                           option(p, u"shortvdotsadjustabove")));
          }
        }
        if (top && !star)
          top->EndEntry();
        const std::u16string name = text(arg(atom));
        const std::u16string tex = u"\\mmlToken{mi}{}" +
                                   property_host::string(p.GetArgument(name)) +
                                   u"\\mmlToken{mi}{}";
        const auto parsed =
            TexParser::make(tex, stack(p).env(), configuration(p));
        const auto base = parsed->mml();
        const auto dot = token(p, u"mo", PropertyList{},
                               Property(property_host::debugText(u"⋮")));
        const auto paddedDot =
            node(p, u"mpadded", {dot},
                 Property(PropertyList{{"width", Property(0)},
                                       {"lspace", Property("-.5width")}}));
        p.Push(node(p, u"mpadded", {paddedDot, node(p, u"mphantom", {base})},
                    Property(PropertyList{{"lspace", Property(".5width")}})));
        if (top) {
          if (star)
            top->EndEntry();
          auto &aligned = MathtoolsUtil::checkAlignment(p, u"\\MTFlushSpaceBelow");
          const auto *table = std::get_if<
              std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(
              &aligned.table);
          if (table && *table) {
            if (aligned.Size())
              aligned.EndEntry();
            aligned.EndRow();
            aligned.addRowSpacing(
                u"-" + property_host::string(
                           option(p, u"shortvdotsadjustbelow")));
          }
        }
        return Property(Missing{});
      });
  own("Aboxed", u"Aboxed", 4,
      [&runtime](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        auto &top = MathtoolsUtil::checkAlignment(p, text(arg(atom)));
        if (array(top.row, "length")->size() % 2 == 1)
          array(top.row, "length")->push_back(node(p, u"mtd", {}));
        const std::u16string name = text(arg(atom));
        const std::u16string argument = text(p.GetArgument(name));
        const std::u16string tail =
            p.string().substr(static_cast<std::size_t>(p.i));
        p.setString(argument + u"&&\\endAboxed");
        p.i = 0;
        const std::u16string left = p.GetUpTo(name, u"&"),
                             right = p.GetUpTo(name, u"&");
        (void)p.GetUpTo(name, u"\\endAboxed");
        const bool math = restBool(rest, 1, true);
        const std::u16string box = restText(rest, 0, u"boxed");
        const std::u16string begin = math ? u"" : u"$\\displaystyle{";
        const std::u16string end = math ? u"" : u"}$";
        const std::u16string templ =
            u"\\rlap{\\" + box + u"{" + begin + u"#1{}#2" + end +
            u"}}\\kern.267em\\phantom{#1}&\\phantom{{}#2}\\kern.267em";
        const Value subArgs = runtime.properties.to(Property(PropertyArray{
            Property(property_host::debugText(left)),
            Property(property_host::debugText(right))}));
        const auto tex = ParseUtil::substituteArgs(p, subArgs, templ);
        p.setString(ParseUtil::addArgs(p, tex, tail));
        p.i = 0;
        return Property(Missing{});
      });
  own("MakeAboxedCommand", u"MakeAboxedCommand", 2,
      [&runtime](const FunctionReceiver &, ParserContext &source,
                 const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &p = parser(source);
        const std::u16string name = text(arg(atom));
        const bool star = p.GetStar();
        const std::u16string cs = NewcommandUtil::GetCSname(p, name);
        const std::u16string box =
            NewcommandUtil::GetCSname(p, name + u"\\" + cs);
        const auto handlers = std::get_if<std::shared_ptr<SubHandlers>>(
            &configuration(p).handlers);
        if (!handlers || !*handlers)
          property_host::typeError(
              "Cannot read properties of undefined (reading 'get')");
        const auto macros = (**handlers).get("macro");
        if (macros) {
          const TokenLookup existing =
              macros->lookup(Property(property_host::debugText(cs)));
          const bool found = std::visit(
              [](const auto &value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::same_as<T, Property>)
                  return property_host::truthy(value);
                else
                  return static_cast<bool>(value);
              },
              existing);
          if (found)
            throw TexError(
                u"AlreadyDefined", u"%1 is already defined",
                std::array<Property, 1>{property_host::debugText(u"\\" + cs)});
        }
        NewcommandUtil::addMacro(
            p, cs,
            functions::fromProperty(
                property_host::get(initializeModule(runtime),
                                   property_host::debugText(u"Aboxed"))),
            Property(PropertyArray{Property(property_host::debugText(box)),
                                   Property(star)}));
        BaseItem *item = factory(p).create("null", {});
        if (!item)
          property_host::typeError(
              "Cannot read properties of undefined (reading 'null')");
        return factory(p).reference(item);
      });
  own("DeclarePairedDelimiter", u"DeclarePairedDelimiter", 2,
      [&runtime](const FunctionReceiver &, ParserContext &source,
                 const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &p = parser(source);
        const auto name = text(arg(atom)),
                   cs = NewcommandUtil::GetCsNameArgument(p, name);
        const auto left = text(p.GetArgument(name)),
                   right = text(p.GetArgument(name));
        MathtoolsUtil::addPairedDelims(
            p, cs, runtime.properties.to(Property(PropertyArray{
                      Property(property_host::debugText(left)),
                      Property(property_host::debugText(right))})));
        BaseItem *item = factory(p).create("null", {});
        return factory(p).reference(item);
      });
  own("DeclarePairedDelimiterX", u"DeclarePairedDelimiterX", 2,
      [&runtime](const FunctionReceiver &, ParserContext &source,
                 const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &p = parser(source);
        const auto name = text(arg(atom)),
                   cs = NewcommandUtil::GetCsNameArgument(p, name);
        const Property n = NewcommandUtil::GetArgCount(p, name);
        const auto left = text(p.GetArgument(name)),
                   right = text(p.GetArgument(name)),
                   body = text(p.GetArgument(name));
        MathtoolsUtil::addPairedDelims(
            p, cs, runtime.properties.to(Property(PropertyArray{
                      Property(property_host::debugText(left)),
                      Property(property_host::debugText(right)),
                      Property(property_host::debugText(body)), n})));
        BaseItem *item = factory(p).create("null", {});
        return factory(p).reference(item);
      });
  own("DeclarePairedDelimiterXPP", u"DeclarePairedDelimiterXPP", 2,
      [&runtime](const FunctionReceiver &, ParserContext &source,
                 const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &p = parser(source);
        const auto name = text(arg(atom)),
                   cs = NewcommandUtil::GetCsNameArgument(p, name);
        const Property n = NewcommandUtil::GetArgCount(p, name);
        const auto pre = text(p.GetArgument(name)),
                   left = text(p.GetArgument(name)),
                   right = text(p.GetArgument(name)),
                   post = text(p.GetArgument(name)),
                   body = text(p.GetArgument(name));
        MathtoolsUtil::addPairedDelims(
            p, cs, runtime.properties.to(Property(PropertyArray{
                      Property(property_host::debugText(left)),
                      Property(property_host::debugText(right)),
                      Property(property_host::debugText(body)), n,
                      Property(property_host::debugText(pre)),
                      Property(property_host::debugText(post))})));
        BaseItem *item = factory(p).create("null", {});
        return factory(p).reference(item);
      });
  own("PairedDelimiters", u"PairedDelimiters", 8,
      [&runtime](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        const auto name = text(arg(atom));
        const bool star = p.GetStar();
        const Property size = star ? Property("") : p.GetBrackets(name);
        const std::u16string open = restText(rest, 0),
                             close = restText(rest, 1);
        std::u16string body = restText(rest, 2, u"#1");
        const double count = restNumber(rest, 3, 1);
        std::u16string pre = restText(rest, 4), post = restText(rest, 5);
        std::u16string left, right, after, delim;
        if (star) {
          left = u"\\mathopen{\\left";
          right = u"\\right";
          after = u"}\\mathclose{}";
          delim = u"\\middle";
        } else if (property_host::truthy(size)) {
          left = text(size) + u"l";
          right = text(size) + u"r";
          delim = text(size);
        }
        std::vector<std::u16string> args;
        if (count != 0)
          for (double i = 0; i < count; ++i)
            args.push_back(text(p.GetArgument(name)));
        if (count != 0) {
          PropertyArray values;
          for (const auto &item : args)
            values.push(property_host::debugText(item));
          const Value argValues = runtime.properties.to(Property(values));
          pre = ParseUtil::substituteArgs(p, argValues, pre);
          body = ParseUtil::substituteArgs(p, argValues, body);
          post = ParseUtil::substituteArgs(p, argValues, post);
        }
        for (std::size_t pos = 0;
             (pos = body.find(u"\\delimsize", pos)) != std::u16string::npos;)
          body.replace(pos, 10, delim);
        std::u16string tex;
        for (const auto &part : std::array<std::u16string, 9>{
                 pre, left, open, body, right, close, after, post,
                 p.string().substr(static_cast<std::size_t>(p.i))})
          tex = ParseUtil::addArgs(p, tex, part);
        p.setString(std::move(tex));
        p.i = 0;
        ParseUtil::checkMaxMacros(p);
        return Property(Missing{});
      });
  own("CenterColon", u"CenterColon", 5,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &,
         ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        const bool center = restBool(rest, 0), force = restBool(rest, 1),
                   thin = restBool(rest, 2);
        const mml::NodeValue colon =
            token(p, u"mo", PropertyList{}, Property(":"));
        if (center &&
            (property_host::truthy(option(p, u"centercolon")) || force)) {
          const Property dy = option(p, u"centercolon-offset");
          PropertyList attrs{{"voffset", dy},
                             {"height", property_host::debugText(
                                            property_host::text("+") +
                                            property_host::string(dy))},
                             {"depth", property_host::debugText(
                                           property_host::text("-") +
                                           property_host::string(dy))}};
          if (thin) {
            attrs.set("width", option(p, u"thincolon-dw"));
            attrs.set("lspace", option(p, u"thincolon-dx"));
          }
          p.Push(node(p, u"mpadded", {colon}, Property(attrs)));
        } else
          p.Push(colon);
        return Property(Missing{});
      });
  own("Relation", u"Relation", 4,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &,
         ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        std::u16string tex = restText(rest, 0);
        const auto unicode = restText(rest, 1);
        if (property_host::truthy(option(p, u"use-unicode")) &&
            !unicode.empty()) {
          p.Push(token(
              p, u"mo",
              Property(PropertyList{{"texClass", mml::TEXCLASS.REL}}),
              Property(property_host::debugText(unicode))));
          return Property(Missing{});
        }
        std::u16string encoded = u"\\mathrel{";
        for (const char16_t c : tex) {
          if (c == u':')
            encoded += u"\\MTThinColon";
          else if (c == u'-')
            encoded += u"\\mathrel{-}";
          else
            encoded.push_back(c);
        }
        encoded += u"}";
        p.setString(ParseUtil::addArgs(
            p, encoded, p.string().substr(static_cast<std::size_t>(p.i))));
        p.i = 0;
        return Property(Missing{});
      });
  own("NArrow", u"NArrow", 4,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &,
         ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        const std::u16string arrow = restText(rest, 0), dy = restText(rest, 1);
        const auto base = token(p, u"mtext", PropertyList{},
                                Property(property_host::debugText(arrow)));
        const auto slashspace =
            node(p, u"mspace", {},
                 Property(PropertyList{{"height", Property(".2em")},
                                       {"depth", Property(0)},
                                       {"width", Property(".4em")}}));
        const auto slash = node(
            p, u"menclose", {slashspace},
            Property(PropertyList{{"notation", Property("updiagonalstrike")},
                                  {"data-thickness", Property(".05em")},
                                  {"data-padding", Property(0)}}));
        const auto raised =
            node(p, u"mpadded", {slash},
                 Property(PropertyList{{"width", Property(0)},
                                       {"lspace", Property("-.5width")},
                                       {"voffset", property_host::debugText(dy)}}));
        const auto overlay = node(
            p, u"mpadded",
            {raised, node(p, u"mphantom",
                          {token(p, u"mtext", PropertyList{},
                                 Property(property_host::debugText(arrow)))})},
            Property(PropertyList{{"width", Property(0)},
                                  {"lspace", Property("-.5width")}}));
        p.Push(node(
            p, u"TeXAtom", {base, overlay},
            Property(PropertyList{{"texClass", mml::TEXCLASS.REL}})));
        return Property(Missing{});
      });
  own("SplitFrac", u"SplitFrac", 3,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        const auto name = text(arg(atom));
        const auto numerator = p.ParseArg(name), denominator = p.ParseArg(name);
        auto blank = token(p, u"mi");
        auto space =
            token(p, u"mspace", PropertyList{{"width", Property("1em")}});
        const auto top =
            node(p, u"mstyle", {numerator, blank, space},
                 Property(PropertyList{{"scriptlevel", Property(0)}}));
        const auto bottom =
            node(p, u"mstyle", {space, token(p, u"mi"), denominator},
                 Property(PropertyList{{"scriptlevel", Property(0)}}));
        const auto frac =
            node(p, u"mfrac", {top, bottom},
                 Property(PropertyList{{"linethickness", Property(0)},
                                       {"numalign", Property("left")},
                                       {"denomalign", Property("right")}}));
        p.Push(node(
            p, u"mstyle", {frac},
            Property(PropertyList{{"displaystyle", Property(restBool(rest, 0))},
                                  {"scriptlevel", Property(0)}})));
        return Property(Missing{});
      });
  own("XMathStrut", u"XMathStrut", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        const std::u16string name = text(arg(atom));
        std::u16string depth = text(p.GetBrackets(name));
        std::u16string height = text(p.GetArgument(name));
        height = MathtoolsUtil::plusOrMinus(name, height);
        depth =
            MathtoolsUtil::plusOrMinus(name, depth.empty() ? height : depth);
        const auto paren = token(
            p, u"mo", Property(PropertyList{{"stretchy", Property(false)}}),
            Property("("));
        const auto phantom = node(p, u"mphantom", {paren});
        const auto padded =
            node(p, u"mpadded", {phantom},
                 Property(PropertyList{
                     {"width", Property(0)},
                     {"height",
                      Property(property_host::debugText(height + u"height"))},
                     {"depth",
                      Property(property_host::debugText(depth + u"depth"))}}));
        p.Push(node(
            p, u"TeXAtom", {padded},
            Property(PropertyList{{"texClass", Property(mml::TEXCLASS.ORD)}})));
        return Property(Missing{});
      });
  own("Prescript", u"Prescript", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        const std::u16string name = text(arg(atom));
        const auto sup = MathtoolsUtil::getScript(p, name, u"sup"),
                   sub = MathtoolsUtil::getScript(p, name, u"sub"),
                   base = MathtoolsUtil::getScript(p, name, u"arg");
        if (sup.isNode() && sup.requireNode()->isKind("none") && sub.isNode() &&
            sub.requireNode()->isKind("none")) {
          p.Push(base);
          return Property(Missing{});
        }
        const auto result = node(p, u"mmultiscripts", {base});
        const auto kids = NodeUtil::instance().getChildren(result);
        kids->push_back(PropertyNull{});
        kids->push_back(PropertyNull{});
        kids->push_back(node(p, u"mprescripts", {}));
        kids->push_back(sub);
        kids->push_back(sup);
        NodeUtil::instance().setProperty(result, "fixPrescript",
                                         Property(true));
        p.Push(result);
        return Property(Missing{});
      });
  own("NewTagForm", u"NewTagForm", 3,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest rest) -> ParseResult {
        auto &p = parser(source);
        const auto macro = text(arg(atom));
        const auto tags = p.tags();
        const auto *ref = std::get_if<TagsRef>(&tags);
        if (!ref || !*ref ||
            !configuration(p).runtime().context().has(
                Value((*ref)->sourceIdentity()), Key(u"mtFormats")))
          throw TexError(
              u"TagsNotMT", u"%1 can only be used with ams or mathtools tags",
              std::array<Property, 1>{property_host::debugText(macro)});
        auto &runtime = configuration(p).runtime();
        Tags &tagOwner = **ref;
        const std::u16string id =
            text(UnitUtil::instance().trimSpaces(p.GetArgument(macro)));
        if (id.empty())
          throw TexError(u"InvalidTagFormID", u"Tag form name can't be empty");
        const std::u16string format = text(p.GetBrackets(macro, Property("")));
        const std::u16string left = text(p.GetArgument(macro)),
                             right = text(p.GetArgument(macro));
        if (!restBool(rest, 0) &&
            MathtoolsTags::hasFormat(runtime, tagOwner, id))
          throw TexError(u"DuplicateTagForm", u"Duplicate tag form: %1",
                         std::array<Property, 1>{property_host::debugText(id)});
        Context &cx = runtime.context();
        const ObjectRef values = cx.array(3);
        cx.set(Value(values), Key(u"0"), Value(left));
        cx.set(Value(values), Key(u"1"), Value(right));
        cx.set(Value(values), Key(u"2"), Value(format));
        MathtoolsTags::setFormat(runtime, tagOwner, id, Value(values));
        BaseItem *item = factory(p).create("null", {});
        return factory(p).reference(item);
      });
  own("UseTagForm", u"UseTagForm", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        const auto macro = text(arg(atom));
        const auto tags = p.tags();
        const auto *ref = std::get_if<TagsRef>(&tags);
        if (!ref || !*ref ||
            !configuration(p).runtime().context().has(
                Value((*ref)->sourceIdentity()), Key(u"mtFormats")))
          throw TexError(
              u"TagsNotMT", u"%1 can only be used with ams or mathtools tags",
              std::array<Property, 1>{property_host::debugText(macro)});
        auto &runtime = configuration(p).runtime();
        Tags &tagOwner = **ref;
        const std::u16string id =
            text(UnitUtil::instance().trimSpaces(p.GetArgument(macro)));
        if (id.empty())
          MathtoolsTags::setCurrent(runtime, tagOwner, Value(Null{}));
        else {
          if (!MathtoolsTags::hasFormat(runtime, tagOwner, id))
            throw TexError(
                u"UndefinedTagForm", u"Undefined tag form: %1",
                std::array<Property, 1>{property_host::debugText(id)});
          MathtoolsTags::setCurrent(
              runtime, tagOwner, MathtoolsTags::format(runtime, tagOwner, id));
        }
        BaseItem *item = factory(p).create("null", {});
        return factory(p).reference(item);
      });
  own("SetOptions", u"SetOptions", 2,
      [](const FunctionReceiver &, ParserContext &source, const ParseAtom &atom,
         ParseRest) -> ParseResult {
        auto &p = parser(source);
        const std::u16string name = text(arg(atom));
        if (!property_host::truthy(option(p, u"allow-mathtoolsset")))
          throw TexError(
              u"ForbiddenMathtoolsSet", u"%1 is disabled",
              std::array<Property, 1>{property_host::debugText(name)});
        const Value options = p.options();
        const auto *object = std::get_if<ObjectRef>(&options.data);
        if (!object)
          property_host::typeError(
              "Cannot read properties of undefined (reading 'mathtools')");
        Context &cx = configuration(p).runtime().context();
        const Value mt = cx.get(*object, Key(u"mathtools"));
        const auto *mtObject = std::get_if<ObjectRef>(&mt.data);
        if (!mtObject)
          property_host::typeError(
              "Cannot read properties of undefined (reading 'mathtools')");
        ObjectRef allowed = cx.object();
        for (const auto &key : cx.ownKeys(mt)) {
          const auto id = cx.keyString(key);
          if (id == u"pariedDelimiters" || id == u"tagforms" ||
              id == u"allow-mathtoolsset")
            continue;
          cx.set(allowed, key, Value(1));
        }
        const auto args = text(p.GetArgument(name));
        const Property parsedProperties = ParseUtil::keyvalOptions(
            configuration(p).runtime(), args, Value(allowed), true);
        const Value parsed =
            configuration(p).runtime().properties.to(parsedProperties);
        const auto *changes = std::get_if<ObjectRef>(&parsed.data);
        if (changes)
          for (const auto &key : cx.ownKeys(parsed)) {
            const auto id = cx.keyString(key);
            const Value next = cx.get(parsed, key);
            if (id == u"legacycolonsymbols" && cx.get(*mtObject, key) != next) {
              const auto handlers = std::get_if<std::shared_ptr<SubHandlers>>(
                  &configuration(p).handlers);
              if (!handlers || !*handlers)
                property_host::typeError(
                    "Cannot read properties of undefined (reading 'add')");
              HandlerConfig config;
              config.setOwn("macro", Property(PropertyArray{Property(
                                         "mathtools-legacycolonsymbols")}));
              if (options::truthy(cx.get(*mtObject, key)))
                (**handlers).remove(config, FallbackConfig{});
              else
                (**handlers)
                    .add(config, FallbackConfig{},
                         MathtoolsMethods::LEGACYPRIORITY);
            }
            cx.set(*mtObject, key, next);
          }
        BaseItem *item = factory(p).create("null", {});
        return factory(p).reference(item);
      });
  constexpr std::array<std::string_view, 38> sourceOrder{
      {"MtMatrix",
       "MtSmallMatrix",
       "MtMultlined",
       "HandleShove",
       "SpreadLines",
       "Cases",
       "MathLap",
       "Cramped",
       "MtLap",
       "MathMakeBox",
       "MathMBox",
       "UnderOverBracket",
       "Aboxed",
       "MakeAboxedCommand",
       "ArrowBetweenLines",
       "VDotsWithin",
       "ShortVDotsWithin",
       "FlushSpaceAbove",
       "FlushSpaceBelow",
       "PairedDelimiters",
       "DeclarePairedDelimiter",
       "DeclarePairedDelimiterX",
       "DeclarePairedDelimiterXPP",
       "CenterColon",
       "Relation",
       "NArrow",
       "SplitFrac",
       "XMathStrut",
       "Prescript",
       "NewTagForm",
       "UseTagForm",
       "SetOptions",
       "Array",
       "Macro",
       "xArrow",
       "HandleRef",
       "AmsEqnArray",
       "MacroWithTemplate"}};
  PropertyList ordered;
  const Property all(methods);
  for (const auto member : sourceOrder)
    ordered.set(
        std::string(member),
         property_host::get(
             all, property_host::debugText(std::u16string(member.begin(), member.end()))));
  Property result(std::move(ordered));
  runtime.mathtoolsMethods = result;
  return result;
}
} // namespace folia::math::tex
