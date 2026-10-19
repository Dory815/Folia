module folia.math.input.tex.ams.methods;
import folia.math.input.tex.parser;
import folia.math.input.tex.ams.items;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.base_parse_methods;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.unit_util;
import folia.math.input.tex.node_util;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.stack_item_factory;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.tex_error;
import folia.math.util.property_host;
import folia.math.util.regexp_binding;
import folia.math.util.js_numeric_input;
import folia.math.core.mml_tree.mml_nodes.munderover;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
TexParser &parserFrom(ParserContext &c) {
  auto *p = dynamic_cast<TexParser *>(&c);
  if (!p)
    property_host::typeError("AMS method requires a TexParser");
  return *p;
}
Property atomValue(const ParseAtom &atom) {
  if (const auto *p = std::get_if<Property>(&atom))
    return *p;
  if (const auto *token = std::get_if<TokenRef>(&atom); token && *token)
    return (*token)->token();
  property_host::typeError("AMS method token is invalid");
}
Property restValue(ParseRest rest, std::size_t index) {
  if (index >= rest.size())
    return Missing{};
  if (const auto *p = std::any_cast<Property>(&rest[index]))
    return *p;
  if (const auto *p = std::any_cast<std::u16string>(&rest[index]))
    return property_host::debugText(*p);
  if (const auto *p = std::any_cast<double>(&rest[index]))
    return *p;
  if (const auto *p = std::any_cast<bool>(&rest[index]))
    return *p;
  return Missing{};
}
std::u16string text(const Property &p) { return property_host::string(p); }
Stack &stack(TexParser &p) {
  const auto *value = std::get_if<std::shared_ptr<Stack>>(&p.stack);
  if (!value || !*value)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'Top')");
  return **value;
}
ParseOptions &config(TexParser &p) {
  const auto *value = std::get_if<ParseOptions *>(&p.configuration);
  if (!value || !*value)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'options')");
  return **value;
}
Tags &tags(TexParser &p) {
  const auto shared = p.tags();
  const auto *value = std::get_if<std::shared_ptr<Tags>>(&shared);
  if (!value || !*value)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'currentTag')");
  return **value;
}
StackItemFactory &factory(TexParser &p) {
  const auto shared = p.itemFactory();
  const auto *value = std::get_if<std::shared_ptr<StackItemFactory>>(&shared);
  if (!value || !*value)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'create')");
  return **value;
}
BaseItem &beginItem(const ParseAtom &atom) {
  const auto *ref = std::get_if<StackItemRef>(&atom);
  if (!ref || !*ref)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'getName')");
  auto *item = dynamic_cast<BaseItem *>(ref->get());
  if (!item)
    property_host::typeError("Environment begin value is not a stack item");
  return *item;
}
FunctionRef method(ParseBody body) {
  return makeParseCallable(std::move(body));
}
BaseItem *create(TexParser &p, std::string_view kind, FunctionArgs args = {}) {
  return factory(p).create(kind, tree::FactoryArgs(args));
}
StackItemRef reference(TexParser &p, BaseItem *value) {
  return factory(p).reference(value);
}
ParseResult pushNull(TexParser &p) {
  p.Push(reference(p, create(p, "null")));
  return Property(Missing{});
}
void pushMml(TexParser &p, mml::NodeValue n) { p.Push(std::move(n)); }
std::u16string nameOf(BaseItem &item) {
  return property_host::string(stack_property_host::scalar(item.getName()));
}
bool sideBase(mml::NodeValue node) {
  const auto *value = node.nodeOrNull();
  if (!value || !value->isKind("msubsup"))
    return false;
  const auto base = NodeUtil::instance().getChildAt(node, 0);
  return base.isNode() && base.requireNode("kind")->isKind("mi") &&
         NodeUtil::instance().getText(base).empty();
}
struct SideParts {
  mml::NodeValue scripts = Missing{}, rest = Missing{};
};
SideParts splitSideSet(mml::NodeValue node) {
  if (!node.isNode())
    return {};
  if (NodeUtil::instance().isInferred(node) &&
      NodeUtil::instance().getChildren(node)->empty())
    return {};
  if (sideBase(node))
    return {node, Missing{}};
  const auto child = NodeUtil::instance().getChildAt(node, 0);
  if (!NodeUtil::instance().isInferred(node) || !child.isNode() ||
      !sideBase(child))
    return {Missing{}, node};
  NodeUtil::instance().getChildren(node)->splice(0, 1);
  return {child, node};
}
std::u16string fromCodePoint(double value) {
  if (!std::isfinite(value) || value < 0 || value > 0x10FFFF)
    property_host::typeError("Invalid code point");
  const auto cp = static_cast<std::uint32_t>(value);
  if (cp <= 0xFFFF)
    return std::u16string(1, static_cast<char16_t>(cp));
  const auto n = cp - 0x10000;
  return {static_cast<char16_t>(0xD800 + (n >> 10)),
          static_cast<char16_t>(0xDC00 + (n & 0x3FF))};
}
FunctionRef baseFunction(ParserRuntime &r, std::u16string_view name) {
  const Property p = BaseMethods::get(r, name);
  if (const auto *f = std::get_if<FunctionRef>(&p.value); f && *f)
    return *f;
  throw Exception("Unimplemented",
                  u"AmsMethods requires a canonical BaseMethods function");
}
double parseInt10(std::u16string_view source) {
  return js_input::parseInt(source, js_input::IntegerRadix::Decimal);
}
Value contextRecord(
    Context &cx,
    std::initializer_list<std::pair<std::u16string_view, Value>> fields) {
  Value result(cx.object());
  for (const auto &[name, value] : fields)
    cx.set(result, Key(std::u16string(name)), value);
  return result;
}
Value childrenArray(Context &cx, std::initializer_list<Value> values) {
  Value result(cx.array(static_cast<std::uint32_t>(values.size())));
  std::size_t i = 0;
  for (const auto &value : values) {
    const std::string n = std::to_string(i++);
    cx.set(result, Key(std::u16string(n.begin(), n.end())), value);
  }
  return result;
}
} // namespace
Property AmsMethods::initializeModule(ParserRuntime &runtime) {
  if (!runtime.amsMethods.missing())
    return runtime.amsMethods;
  PropertyList methods;
  const auto add = [&](std::string name, FunctionRef body) {
    const std::u16string functionName = property_host::text(name);
    const Value bound = BaseMethods::bind(runtime, body, functionName);
    const auto *identity = std::get_if<ObjectRef>(&bound.data);
    if (!identity)
      throw Exception("TypeError", u"AMS method has no Context identity");
    const auto length = [&]() -> std::size_t {
      if (name == "AmsEqnArray") return 8;
      if (name == "AlignAt" || name == "XalignAt") return 4;
      if (name == "Multline" || name == "MultiIntegral" || name == "HandleShove") return 3;
      if (name == "FlalignArray") return 9;
      if (name == "SideSet" || name == "HandleDeclareOp" || name == "HandleOperatorName" ||
          name == "operatorLetter" || name == "CFrac" || name == "HandleTag") return 2;
      if (name == "xArrow") return 6;
      if (name == "Genfrac") return 6;
      throw Exception("Unimplemented", u"Missing pinned AMS method arity");
    }();
    runtime.context().define(*identity, Key(u"length"),
        Descriptor::data(Value(static_cast<double>(length)), false, false, true));
    methods.set(std::move(name), Property(std::move(body)));
  };
  add("AmsEqnArray",
      method([&runtime](const FunctionReceiver &receiver,
                        ParserContext &context, const ParseAtom &begin,
                        ParseRest rest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string name =
            u"\\begin{" + nameOf(beginItem(begin)) + u"}";
        const Property valign = parser.GetBrackets(name);
        const FunctionRef base = baseFunction(runtime, u"EqnArray");
        const ParseResult result = invokeParseMethod(
            ParseMethod{base}, receiver, context, begin, rest);
        const auto *item = std::get_if<StackItemRef>(&result);
        auto *arrayItem =
            item && *item ? dynamic_cast<ArrayItem *>(item->get()) : nullptr;
        if (!arrayItem)
          property_host::typeError(
              "BaseMethods.EqnArray did not return an ArrayItem");
        (void)ParseUtil::setArrayAlign(
            *arrayItem,
            property_host::truthy(valign) ? text(valign) : std::u16string{},
            &parser);
        return *item;
      }));
  add("AlignAt",
      method([&runtime](const FunctionReceiver &receiver,
                        ParserContext &context, const ParseAtom &begin,
                        ParseRest rest) -> ParseResult {
        auto &parser = parserFrom(context);
        auto &item = beginItem(begin);
        const std::u16string name = nameOf(item);
        Property valign = Missing{};
        std::u16string align, balign;
        std::vector<std::u16string> spaces;
        const bool taggable = property_host::truthy(restValue(rest, 1));
        if (!taggable)
          valign = parser.GetBrackets(u"\\begin{" + name + u"}");
        const std::u16string n =
            text(parser.GetArgument(u"\\begin{" + name + u"}"));
        const auto matcher = runtime.regexp().create(u"[^0-9]");
        if (runtime.regexp().exec(Value(matcher), n))
          throw TexError(u"PositiveIntegerArg",
                         u"Argument to %1 must be a positive integer",
                         std::array<Property, 1>{property_host::debugText(
                             u"\\begin{" + name + u"}")});
        double count = parseInt10(n);
        while (count > 0) {
          align += u"rl";
          balign += u"bt";
          spaces.push_back(u"0em 0em");
          count -= 1;
        }
        std::u16string spacing;
        for (std::size_t i = 0; i < spaces.size(); ++i) {
          if (i)
            spacing.push_back(u' ');
          spacing += spaces[i];
        }
        std::vector<std::any> args{std::any(restValue(rest, 0)),
                                   std::any(restValue(rest, 1)),
                                   std::any(Property(property_host::debugText(align))),
                                   std::any(Property(property_host::debugText(balign))),
                                   std::any(Property(property_host::debugText(spacing)))};
        const auto table = runtime.amsMethods;
        const Property eqn =
            property_host::get(table, property_host::debugText(u"EqnArray"));
        const auto *fn = std::get_if<FunctionRef>(&eqn.value);
        if (!fn || !*fn)
          throw Exception("Unimplemented",
                          u"AmsMethods.EqnArray is unavailable");
        const ParseResult result =
            invokeParseMethod(ParseMethod{*fn}, receiver, context, begin, args);
        if (taggable)
          return result;
        const auto *itemRef = std::get_if<StackItemRef>(&result);
        auto *array = itemRef && *itemRef
                          ? dynamic_cast<ArrayItem *>(itemRef->get())
                          : nullptr;
        if (!array)
          property_host::typeError("EqnArray did not return an ArrayItem");
        (void)ParseUtil::setArrayAlign(
            *array,
            property_host::truthy(valign) ? text(valign) : std::u16string{},
            &parser);
        return *itemRef;
      }));
  add("Multline",
      method([&runtime](const FunctionReceiver &, ParserContext &context,
                        const ParseAtom &begin, ParseRest rest) -> ParseResult {
        auto &parser = parserFrom(context);
        ParseUtil::checkEqnEnv(parser);
        parser.Push(std::get<StackItemRef>(begin));
        const Value ams =
            runtime.context().get(config(parser).options, Key(u"ams"));
        const Value padding =
            runtime.context().get(ams, Key(u"multlineIndent"));
        const Value width = runtime.context().get(ams, Key(u"multlineWidth"));
        const Value side =
            runtime.context().get(config(parser).options, Key(u"tagSide"));
        const Value indent =
            runtime.context().get(config(parser).options, Key(u"tagIndent"));
        Value arraydef = runtime.context().object();
        auto &cx = runtime.context();
        cx.set(arraydef, Key(u"displaystyle"), Value(true));
        cx.set(arraydef, Key(u"rowspacing"), Value(u".5em"));
        cx.set(arraydef, Key(u"columnspacing"), Value(u"100%"));
        cx.set(arraydef, Key(u"width"), width);
        cx.set(arraydef, Key(u"side"), side);
        cx.set(arraydef, Key(u"minlabelspacing"), indent);
        cx.set(arraydef, Key(u"data-array-padding"),
               Value(property_host::string(runtime.properties.from(padding)) +
                     u" " +
                     property_host::string(runtime.properties.from(padding))));
        cx.set(arraydef, Key(u"data-width-includes-label"), Value(true));
        const auto *parserStack =
            std::get_if<std::shared_ptr<Stack>>(&parser.stack);
        if (!parserStack || !*parserStack)
          property_host::typeError(
              "Cannot read properties of null or undefined (reading 'stack')");
        const std::array<std::any, 2> args{std::any(restValue(rest, 0)),
                                           std::any(*parserStack)};
        BaseItem *raw = create(parser, "multline", args);
        auto *item = dynamic_cast<MultlineItem *>(raw);
        if (!item)
          property_host::typeError(
              "Multline factory did not return MultlineItem");
        item->arraydef = arraydef;
        return reference(parser, item);
      }));
  add("HandleDeclareOp",
      method([&runtime](const FunctionReceiver &, ParserContext &context,
                        const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string name = text(atomValue(atom));
        const bool star = parser.GetStar();
        const std::u16string cs =
            NewcommandUtil::GetCsNameArgument(parser, name);
        const std::u16string op = text(parser.GetArgument(name));
        std::u16string expansion = u"\\operatorname";
        if (star)
          expansion += u"*";
        expansion += u"{";
        expansion += op;
        expansion += u"}";
        NewcommandUtil::addMacro(
            parser, cs, baseFunction(runtime, u"Macro"),
            Property(PropertyArray{property_host::debugText(expansion)}));
        return pushNull(parser);
      }));
  add("XalignAt",
      method([&runtime](const FunctionReceiver &receiver,
                        ParserContext &context, const ParseAtom &begin,
                        ParseRest rest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string env = nameOf(beginItem(begin));
        const std::u16string n =
            property_host::string(parser.GetArgument(u"\\begin{" + env + u"}"));
        const auto invalid = runtime.regexp().create(u"[^0-9]");
        if (runtime.regexp().exec(Value(invalid), n))
          throw TexError(u"PositiveIntegerArg",
                         u"Argument to %1 must be a positive integer",
                         std::array<Property, 1>{property_host::debugText(
                             u"\\begin{" + env + u"}")});
        const bool padded = property_host::truthy(restValue(rest, 1));
        const std::u16string align = padded ? u"crl" : u"rlc",
                             balign = padded ? u"mbt" : u"btm",
                             width =
                                 padded ? u"fit auto auto" : u"auto auto fit";
        const Property body = property_host::get(
            runtime.amsMethods, property_host::debugText(u"FlalignArray"));
        const auto *fn = std::get_if<FunctionRef>(&body.value);
        if (!fn || !*fn)
          throw Exception("Unimplemented",
                          u"AmsMethods.FlalignArray unavailable");
        const std::array<std::any, 7> args{
            std::any(restValue(rest, 0)),
            std::any(Property(padded)),
            std::any(Property(false)),
            std::any(Property(property_host::debugText(align))),
            std::any(Property(property_host::debugText(balign))),
            std::any(Property(property_host::debugText(width))),
            std::any(Property(true))};
        const auto result =
            invokeParseMethod(ParseMethod{*fn}, receiver, context, begin, args);
        const auto *itemRef = std::get_if<StackItemRef>(&result);
        auto *item = itemRef && *itemRef
                         ? dynamic_cast<FlalignItem *>(itemRef->get())
                         : nullptr;
        if (!item)
          property_host::typeError("FlalignArray did not return FlalignItem");
        item->setProperty("xalignat", Property(2 * parseInt10(n)));
        return *itemRef;
      }));
  add("FlalignArray",
      method([&runtime](const FunctionReceiver &, ParserContext &context,
                        const ParseAtom &begin, ParseRest rest) -> ParseResult {
        auto &parser = parserFrom(context);
        ParseUtil::checkEqnEnv(parser);
        parser.Push(std::get<StackItemRef>(begin));
        std::u16string align = text(restValue(rest, 3));
        std::u16string converted;
        for (char16_t c : align) {
          if (!converted.empty())
            converted.push_back(u' ');
          if (c == u'r')
            converted += u"right";
          else if (c == u'l')
            converted += u"left";
          else if (c == u'c')
            converted += u"center";
          else
            converted.push_back(c);
        }
        const std::u16string breakAlign = property_host::string(
            runtime.properties.from(splitAlignArray(text(restValue(rest, 4)))));
        const bool numbered = property_host::truthy(restValue(rest, 0)),
                   padded = property_host::truthy(restValue(rest, 1)),
                   center = property_host::truthy(restValue(rest, 2)),
                   zero = rest.size() > 6
                              ? property_host::truthy(restValue(rest, 6))
                              : false;
        const auto side =
            runtime.context().get(config(parser).options, Key(u"tagSide"));
        const auto indent =
            runtime.context().get(config(parser).options, Key(u"tagIndent"));
        const Value minLabel = zero ? Value(u"0") : indent;
        Value arraydef = contextRecord(
            runtime.context(),
            {{u"width", Value(u"100%")},
             {u"displaystyle", Value(true)},
             {u"columnalign", Value(converted)},
             {u"columnspacing", Value(u"0em")},
             {u"columnwidth", runtime.properties.to(restValue(rest, 5))},
             {u"rowspacing", Value(u"3pt")},
             {u"data-break-align", Value(breakAlign)},
             {u"side", side},
             {u"minlabelspacing", minLabel},
             {u"data-width-includes-label", Value(true)}});
        const std::u16string env = nameOf(beginItem(begin));
        const auto *stackValue =
            std::get_if<std::shared_ptr<Stack>>(&parser.stack);
        if (!stackValue || !*stackValue)
          property_host::typeError(
              "Cannot read properties of null or undefined (reading 'stack')");
        const std::array<std::any, 5> args{
            std::any(Property(property_host::debugText(env))),
            std::any(Property(numbered)), std::any(Property(padded)),
            std::any(Property(center)), std::any(*stackValue)};
        auto *raw = create(parser, "flalign", args);
        auto *item = dynamic_cast<FlalignItem *>(raw);
        if (!item)
          property_host::typeError(
              "Flalign factory did not return FlalignItem");
        item->arraydef = arraydef;
        item->setProperty("zeroWidthLabel", Property(zero));
        return reference(parser, item);
      }));
  add("HandleOperatorName",
      method([&runtime](const FunctionReceiver &, ParserContext &context,
                        const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &parser = parserFrom(context);
        const bool star = parser.GetStar();
        const std::u16string name = text(atomValue(atom));
        const std::u16string op = property_host::string(
            UnitUtil::instance().trimSpaces(parser.GetArgument(name)));
        Property envCopy = runtime.properties.copyOwnEnumerableProperties(
            stack_property_host::scalar(stack(parser).env()));
        Value env = runtime.properties.to(envCopy);
        const auto ams =
            runtime.context().get(config(parser).options, Key(u"ams"));
        const auto pattern =
            runtime.context().get(ams, Key(u"operatornamePattern"));
        runtime.context().set(env, Key(u"font"), Value(u"normal"));
        runtime.context().set(env, Key(u"multiLetterIdentifiers"), pattern);
        runtime.context().set(env, Key(u"operatorLetters"), Value(true));
        runtime.context().set(env, Key(u"noAutoOP"), Value(true));
        const auto *po = std::get_if<ParseOptions *>(&parser.configuration);
        if (!po || !*po)
          property_host::typeError("Cannot read properties of null or "
                                   "undefined (reading 'nodeFactory')");
        auto mml =
            TexParser::make(op, runtime.properties.from(env), **po)->mml();
        if (mml.requireNode("kind")->isKind("mi")) {
          const std::array<std::string, 1> remove{"autoOP"};
          NodeUtil::instance().removeProperties(mml, remove);
        } else {
          const auto children =
              std::make_shared<tree::ChildArray<mml::MmlNode>>(
                  std::initializer_list<mml::NodeValue>{mml});
          const std::array<std::any, 2> args{
              std::any(Property(property_host::debugText(u"TeXAtom"))),
              std::any(children)};
          mml = parser.create(u"node", args);
        }
        NodeUtil::instance().setProperties(
            mml, Property(PropertyList{
                     {"movesupsub", Property(star)},
                     {"movablelimits", Property(true)},
                     {"texClass", Property(static_cast<double>(
                                      folia::math::mml::TEXCLASS.OP))}}));
        if (!star) {
          const std::u16string c = parser.GetNext();
          const double i = parser.i;
          if (c == u"\\") {
            ++parser.i;
            if (parser.GetCS() != u"limits")
              parser.i = i;
          }
        }
        const std::array<std::any, 1> args{std::any(mml)};
        auto *fn = create(parser, "fn", args);
        parser.Push(reference(parser, fn));
        return Property(Missing{});
      }));
  add("SideSet", method([](const FunctionReceiver &, ParserContext &context,
                           const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string name = text(atomValue(atom));
        const auto pre = splitSideSet(parser.ParseArg(name));
        const auto post = splitSideSet(parser.ParseArg(name));
        const mml::NodeValue base = parser.ParseArg(name);
        mml::NodeValue result = base;
        if (pre.scripts.isNode()) {
          if (pre.rest.isNode()) {
            const auto copy = ParseUtil::copyNode(base, parser);
            auto padded = parser.create(
                u"node",
                std::array<std::any, 3>{
                    std::any(Property(property_host::debugText(u"mpadded"))),
                    std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>(
                        std::initializer_list<mml::NodeValue>{copy})),
                    std::any(Property(PropertyList{{"width", 0}}))});
            auto phantom = parser.create(
                u"node",
                std::array<std::any, 2>{
                    std::any(Property(property_host::debugText(u"mphantom"))),
                    std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>(
                        std::initializer_list<mml::NodeValue>{padded}))});
            (void)pre.scripts.requireNode("replaceChild")
                ->replaceChild(phantom,
                               NodeUtil::instance().getChildAt(pre.scripts, 0));
          } else {
            result = parser.create(
                u"node",
                std::array<std::any, 2>{
                    std::any(Property(property_host::debugText(u"mmultiscripts"))),
                    std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>(
                        std::initializer_list<mml::NodeValue>{base}))});
            if (post.scripts.isNode())
              NodeUtil::instance().appendChildren(
                  result,
                  std::make_shared<tree::ChildArray<
                      mml::MmlNode>>(std::initializer_list<mml::NodeValue>{
                      NodeUtil::instance().getChildAt(post.scripts, 1).isNode()
                          ? NodeUtil::instance().getChildAt(post.scripts, 1)
                          : parser.create(
                                u"node",
                                std::array<std::any, 1>{std::any(
                                    Property(property_host::debugText(u"none")))}),
                      NodeUtil::instance().getChildAt(post.scripts, 2).isNode()
                          ? NodeUtil::instance().getChildAt(post.scripts, 2)
                          : parser.create(
                                u"node",
                                std::array<std::any, 1>{
                                    std::any(Property(
                                        property_host::debugText(u"none")))})}));
            NodeUtil::instance().setProperty(result, "scriptalign",
                                             Property("left"));
            const auto none = parser.create(
                u"node", std::array<std::any, 1>{std::any(
                             Property(property_host::debugText(u"mprescripts")))});
            const auto sub = NodeUtil::instance().getChildAt(pre.scripts, 1);
            const auto sup = NodeUtil::instance().getChildAt(pre.scripts, 2);
            const auto noneNode = [&]() {
              return parser.create(
                  u"node", std::array<std::any, 1>{
                               std::any(Property(property_host::debugText(u"none")))});
            };
            NodeUtil::instance().appendChildren(
                result, std::make_shared<tree::ChildArray<mml::MmlNode>>(
                            std::initializer_list<mml::NodeValue>{
                                none, sub.isNode() ? sub : noneNode(),
                                sup.isNode() ? sup : noneNode()}));
          }
        }
        if (post.scripts.isNode() && base.isNode() && result.isNode() &&
            result.nodeOrNull() == base.nodeOrNull()) {
          post.scripts.requireNode("replaceChild")
              ->replaceChild(base,
                             NodeUtil::instance().getChildAt(post.scripts, 0));
          result = post.scripts;
        }
        const auto row = parser.create(
            u"node",
            std::array<std::any, 3>{
                std::any(Property(property_host::debugText(u"TeXAtom"))),
                std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>()),
                std::any(Property(PropertyList{
                    {"texClass",
                     static_cast<double>(folia::math::mml::TEXCLASS.OP)},
                    {"movesupsub", true},
                    {"movablelimits", true}}))});
        if (pre.rest.isNode()) {
          if (pre.scripts.isNode())
            row.requireNode("appendChild")->appendChild(pre.scripts);
          row.requireNode("appendChild")->appendChild(pre.rest);
        }
        if (result.isNode())
          row.requireNode("appendChild")->appendChild(result);
        if (post.rest.isNode())
          row.requireNode("appendChild")->appendChild(post.rest);
        parser.Push(row);
        return Property(Missing{});
      }));
  add("xArrow",
      method([&runtime](const FunctionReceiver &, ParserContext &context,
                        const ParseAtom &atom, ParseRest rest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string name = text(atomValue(atom));
        const double chr = property_host::number(restValue(rest, 0)),
                     left = property_host::number(restValue(rest, 1)),
                     right = property_host::number(restValue(rest, 2)),
                     minimum = property_host::truthy(restValue(rest, 3))
                                   ? property_host::number(restValue(rest, 3))
                                   : 0;
        const Property bot = parser.GetBrackets(name);
        const mml::NodeValue first = parser.ParseArg(name);
        const std::u16string width =
            u"+" +
            property_host::text(UnitUtil::instance().em((left + right) / 18));
        const std::u16string lspace =
            property_host::text(UnitUtil::instance().em(left / 18));
        const auto empty = std::make_shared<tree::ChildArray<mml::MmlNode>>();
        const auto depth = parser.create(
            u"node",
            std::array<std::any, 3>{
                std::any(Property(property_host::debugText(u"mspace"))), std::any(empty),
                std::any(Property(PropertyList{
                    {"depth", property_host::debugText(u".2em")}}))});
        PropertyList arrowAttrs{
            {"stretchy", true},
            {"texClass", static_cast<double>(folia::math::mml::TEXCLASS.ORD)}};
        const auto arrow = parser.create(
            u"token",
            std::array<std::any, 3>{
                std::any(Property(property_host::debugText(u"mo"))),
                std::any(Property(arrowAttrs)),
                std::any(Property(property_host::debugText(fromCodePoint(chr))))});
        if (minimum)
          NodeUtil::instance().setAttribute(
              arrow, "minsize",
              property_host::debugText(
                  property_host::text(UnitUtil::instance().em(minimum))));
        auto arrowStyle = parser.create(
            u"node",
            std::array<std::any, 3>{
                std::any(Property(property_host::debugText(u"mstyle"))),
                std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>(
                    std::initializer_list<mml::NodeValue>{arrow})),
                std::any(Property(PropertyList{{"scriptlevel", 0}}))});
        auto mml = parser.create(
            u"node",
            std::array<std::any, 2>{
                std::any(Property(property_host::debugText(u"munderover"))),
                std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>(
                    std::initializer_list<mml::NodeValue>{arrowStyle}))});
        auto *scripts =
            dynamic_cast<folia::math::mml::MmlMunderover *>(mml.nodeOrNull());
        if (!scripts)
          property_host::typeError("xArrow factory did not create munderover");
        PropertyList padDef{{"width", property_host::debugText(width)},
                            {"lspace", property_host::debugText(lspace)}};
        auto padded = parser.create(
            u"node",
            std::array<std::any, 3>{
                std::any(Property(property_host::debugText(u"mpadded"))),
                std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>(
                    std::initializer_list<mml::NodeValue>{first, depth})),
                std::any(Property(padDef))});
        NodeUtil::instance().setAttribute(padded, "voffset",
                                          property_host::debugText(u"-.2em"));
        NodeUtil::instance().setAttribute(padded, "height",
                                          property_host::debugText(u"-.2em"));
        NodeUtil::instance().setChild(mml, scripts->over(), padded);
        if (property_host::truthy(bot)) {
          const auto *po = std::get_if<ParseOptions *>(&parser.configuration);
          if (!po || !*po)
            property_host::typeError("Cannot read properties of null or "
                                     "undefined (reading 'nodeFactory')");
          const mml::NodeValue bottom =
              TexParser::make(text(bot),
                              stack_property_host::scalar(stack(parser).env()),
                              **po)
                  ->mml();
          const auto bstrut = parser.create(
              u"node",
              std::array<std::any, 3>{
                  std::any(Property(property_host::debugText(u"mspace"))),
                  std::any(empty),
                  std::any(Property(PropertyList{
                      {"height", property_host::debugText(u".75em")}}))});
          auto lower = parser.create(
              u"node",
              std::array<std::any, 3>{
                  std::any(Property(property_host::debugText(u"mpadded"))),
                  std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>(
                      std::initializer_list<mml::NodeValue>{bottom, bstrut})),
                  std::any(Property(padDef))});
          NodeUtil::instance().setAttribute(lower, "voffset",
                                            property_host::debugText(u".15em"));
          NodeUtil::instance().setAttribute(
              lower, "depth", property_host::debugText(u"-.15em"));
          NodeUtil::instance().setChild(mml, scripts->under(), lower);
        }
        NodeUtil::instance().setProperty(mml, "subsupOK", Property(true));
        const auto none = parser.create(
            u"node",
            std::array<std::any, 3>{
                std::any(Property(property_host::debugText(u"TeXAtom"))),
                std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>()),
                std::any(Property(PropertyList{
                    {"texClass",
                     static_cast<double>(folia::math::mml::TEXCLASS.NONE)}}))});
        const auto outer = parser.create(
            u"node",
            std::array<std::any, 3>{
                std::any(Property(property_host::debugText(u"TeXAtom"))),
                std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>(
                    std::initializer_list<mml::NodeValue>{none, mml})),
                std::any(Property(PropertyList{
                    {"texClass",
                     static_cast<double>(folia::math::mml::TEXCLASS.REL)}}))});
        parser.Push(outer);
        return Property(Missing{});
      }));
  add("operatorLetter",
      method([&runtime](const FunctionReceiver &receiver,
                        ParserContext &context, const ParseAtom &atom,
                        ParseRest rest) -> ParseResult {
        auto &parser = parserFrom(context);
        const auto *stackValue =
            std::get_if<std::shared_ptr<Stack>>(&parser.stack);
        if (!stackValue || !*stackValue)
          property_host::typeError(
              "Cannot read properties of null or undefined (reading 'env')");
        const Property letters =
            property_host::get((*stackValue)->env(), "operatorLetters");
        if (!property_host::truthy(letters))
          return Property(false);
        const Property methodValue = property_host::get(
            runtime.parseMethods, property_host::debugText(u"variable"));
        const auto *fn = std::get_if<FunctionRef>(&methodValue.value);
        if (!fn || !*fn)
          throw Exception("Unimplemented",
                          u"ParseMethods.variable is unavailable");
        return invokeParseMethod(ParseMethod{*fn}, receiver, context, atom,
                                 rest);
      }));
  add("MultiIntegral",
      method([](const FunctionReceiver &, ParserContext &context,
                const ParseAtom &atom, ParseRest rest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string name = text(atomValue(atom));
        std::u16string integral = text(restValue(rest, 0));
        std::u16string next = parser.GetNext();
        if (next == u"\\") {
          const double i = parser.i;
          next = text(parser.GetArgument(name));
          parser.i = i;
          if (next == u"\\limits")
            integral = u"\\!\\!\\mathop{\\,\\," + integral + u"}";
        }
        const auto source = parser.string();
        const std::size_t cursor = static_cast<std::size_t>(
            std::clamp(parser.i, 0.0, static_cast<double>(source.size())));
        parser.setString(integral + u" " + source.substr(cursor));
        parser.i = 0;
        return Property(Missing{});
      }));
  add("HandleShove",
      method([](const FunctionReceiver &, ParserContext &context,
                const ParseAtom &, ParseRest rest) -> ParseResult {
        auto &parser = parserFrom(context);
        const StackEntry top = stack(parser).Top();
        BaseItem &item = stack_host::item(top, "kind");
        auto *raw = &item;
        if (raw->kind() != "multline")
          throw TexError(
              u"CommandOnlyAllowedInEnv", u"%1 only allowed in %2 environment",
              std::array<Property, 2>{parser.currentCS(),
                                      property_host::debugText(u"multline")});
        if (raw->Size())
          throw TexError(u"CommandAtTheBeginingOfLine",
                         u"%1 must come at the beginning of the line",
                         std::array<Property, 1>{parser.currentCS()});
        raw->setProperty("shove", restValue(rest, 0));
        return Property(Missing{});
      }));
  add("CFrac", method([](const FunctionReceiver &, ParserContext &context,
                         const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string name = text(atomValue(atom));
        Property align = UnitUtil::instance().trimSpaces(
            parser.GetBrackets(name, property_host::debugText(u"")));
        const Property numerator = parser.GetArgument(name),
                       denominator = parser.GetArgument(name);
        auto sharedConfig = parser.configuration;
        auto *po = std::get_if<ParseOptions *>(&sharedConfig);
        if (!po || !*po)
          property_host::typeError("Cannot read properties of null or "
                                   "undefined (reading 'nodeFactory')");
        const Property env = stack(parser).env();
        const std::u16string num = u"\\strut\\textstyle{" + text(numerator) +
                                   u"}",
                             den = u"\\strut\\textstyle{" + text(denominator) +
                                   u"}";
        auto n = TexParser::make(num, env, **po)->mml();
        auto d = TexParser::make(den, env, **po)->mml();
        const auto children = std::make_shared<tree::ChildArray<mml::MmlNode>>(
            std::initializer_list<mml::NodeValue>{n, d});
        const std::array<std::any, 2> args{
            std::any(Property(property_host::debugText(u"mfrac"))), std::any(children)};
        auto frac = parser.create(u"node", args);
        std::u16string side;
        if (align == property_host::debugText(u"l"))
          side = u"left";
        else if (align == property_host::debugText(u"r"))
          side = u"right";
        else if (align.missing() || align == property_host::debugText(u"")) {
        } else
          throw TexError(u"IllegalAlign", u"Illegal alignment specified in %1",
                         std::array<Property, 1>{parser.currentCS()});
        if (!side.empty()) {
          PropertyList set{{"numalign", property_host::debugText(side)},
                           {"denomalign", property_host::debugText(side)}};
          NodeUtil::instance().setProperties(frac, Property(set));
        }
        parser.Push(frac);
        return Property(Missing{});
      }));
  add("Genfrac",
      method([](const FunctionReceiver &, ParserContext &context,
                const ParseAtom &atom, ParseRest rest) -> ParseResult {
        auto &parser = parserFrom(context);
        const std::u16string name = text(atomValue(atom));
        Property left = restValue(rest, 0), right = restValue(rest, 1),
                 thick = restValue(rest, 2), style = restValue(rest, 3);
        if (left.missing() || std::holds_alternative<PropertyNull>(left.value))
          left = parser.GetDelimiterArg(name);
        if (right.missing() ||
            std::holds_alternative<PropertyNull>(right.value))
          right = parser.GetDelimiterArg(name);
        if (thick.missing() ||
            std::holds_alternative<PropertyNull>(thick.value))
          thick = parser.GetArgument(name);
        if (style.missing() ||
            std::holds_alternative<PropertyNull>(style.value))
          style = UnitUtil::instance().trimSpaces(parser.GetArgument(name));
        const auto num = parser.ParseArg(name), den = parser.ParseArg(name);
        std::array<std::any, 3> nodeArgs{
            std::any(Property(property_host::debugText(u"mfrac"))),
            std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>(
                std::initializer_list<mml::NodeValue>{num, den})),
            std::any(Property(PropertyList{}))};
        mml::NodeValue frac = parser.create(u"node", nodeArgs);
        if (!text(thick).empty())
          NodeUtil::instance().setAttribute(frac, "linethickness", thick);
        if (property_host::truthy(left) || property_host::truthy(right)) {
          NodeUtil::instance().setProperty(frac, "withDelims", Property(true));
          frac = ParseUtil::fixedFence(config(parser), text(left), frac,
                                       text(right));
        }
        if (!text(style).empty()) {
          const double parsed =
              js_input::parseInt(text(style), js_input::IntegerRadix::Decimal);
          if (!std::isfinite(parsed) || parsed < 0 || parsed > 3)
            throw TexError(u"BadMathStyleFor", u"Bad math style for %1",
                           std::array<Property, 1>{parser.currentCS()});
          const int index = static_cast<int>(parsed);
          std::array<std::any, 2> styleArgs{
              std::any(Property(property_host::debugText(u"mstyle"))),
              std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>(
                  std::initializer_list<mml::NodeValue>{frac}))};
          frac = parser.create(u"node", styleArgs);
          if (index == 0)
            NodeUtil::instance().setProperties(
                frac, Property(PropertyList{{"displaystyle", true},
                                            {"scriptlevel", 0}}));
          else
            NodeUtil::instance().setProperties(
                frac, Property(PropertyList{{"displaystyle", false},
                                            {"scriptlevel", index - 1}}));
        }
        parser.Push(frac);
        return Property(Missing{});
      }));
  add("HandleTag",
      method([&runtime](const FunctionReceiver &, ParserContext &context,
                        const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &parser = parserFrom(context);
        Tags &tagger = tags(parser);
        const Value identity(tagger.sourceIdentity());
        const Value currentForTaggable =
            runtime.context().get(identity, Key(u"currentTag"));
        const Value taggable =
            runtime.context().get(currentForTaggable, Key(u"taggable"));
        const Value env = tagger.env();
        if (!truthy(taggable) && truthy(env))
          throw TexError(u"CommandNotAllowedInEnv",
                         u"%1 not allowed in %2 environment",
                         std::array<Property, 2>{parser.currentCS(),
                                                 runtime.properties.from(env)});
        const Value currentForTag =
            runtime.context().get(identity, Key(u"currentTag"));
        const Value tag = runtime.context().get(currentForTag, Key(u"tag"));
        if (truthy(tag))
          throw TexError(u"MultipleCommand", u"Multiple %1",
                         std::array<Property, 1>{parser.currentCS()});
        const bool star = parser.GetStar();
        const std::u16string name = text(atomValue(atom));
        const std::u16string id = property_host::string(
            UnitUtil::instance().trimSpaces(parser.GetArgument(name)));
        tagger.tag(Value(id), Value(star));
        return pushNull(parser);
      }));
  for (const auto name :
       {u"HandleNoTag", u"HandleRef", u"Macro", u"Accent", u"Tilde", u"Array",
        u"Spacer", u"NamedOp", u"EqnArray", u"Equation"}) {
    const Property body = BaseMethods::get(runtime, name);
    if (body.missing())
      throw Exception(
          "Unimplemented",
          u"AmsMethods alias requires an unavailable BaseMethods function");
    methods.set(property_host::debugText(std::u16string_view(name)), body);
  }
  runtime.amsMethods = Property(std::move(methods));
  return runtime.amsMethods;
}
Property AmsMethods::get(ParserRuntime &runtime, std::u16string_view name) {
  if (runtime.amsMethods.missing())
    initializeModule(runtime);
  return property_host::get(runtime.amsMethods, property_host::debugText(name));
}
} // namespace folia::math::tex
