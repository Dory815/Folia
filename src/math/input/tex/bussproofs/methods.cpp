// Native port of MathJax 4.1.3 input/tex/bussproofs/BussproofsMethods.ts.
module folia.math.input.tex.bussproofs.methods;
import folia.math.input.tex.bussproofs.util;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.unit_util;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.node_util;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.stack_item_factory;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
using namespace options;
namespace {
using namespace options;
TexParser &parser(ParserContext &value) {
  auto *result = dynamic_cast<TexParser *>(&value);
  if (!result)
    property_host::typeError("Bussproofs requires a TexParser");
  return *result;
}
std::u16string atomText(const ParseAtom &atom) {
  if (const auto *value = std::get_if<Property>(&atom))
    return property_host::string(*value);
  if (const auto *token = std::get_if<TokenRef>(&atom); token && *token)
    return property_host::string((*token)->character());
  property_host::typeError("Bussproofs parse method requires a string token");
}
Stack &stack(TexParser &p) {
  const auto *result = std::get_if<std::shared_ptr<Stack>>(&p.stack);
  if (!result || !*result)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'Top')");
  return **result;
}
ParseOptions &configuration(TexParser &p) {
  const auto *result = std::get_if<ParseOptions *>(&p.configuration);
  if (!result || !*result)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'configuration')");
  return **result;
}
BaseItem &proofTree(TexParser &p) {
  auto &item = stack_host::item(stack(p).Top(), "kind");
  if (item.kind() != "proofTree")
    throw TexError(u"IllegalProofCommand",
                   u"Proof commands only allowed in prooftree environment.");
  return item;
}
mml::NodeValue node(TexParser &p, std::string_view kind,
                    std::span<const mml::NodeValue> children,
                    Property attrs = PropertyList{}) {
  auto array = std::make_shared<tree::ChildArray<mml::MmlNode>>();
  for (const auto &child : children)
    array->push_back(child);
  const std::array<std::any, 3> args{std::any(Property(std::string(kind))),
                                     std::any(array),
                                     std::any(std::move(attrs))};
  return p.create(u"node", FunctionArgs(args));
}
mml::NodeValue padded(TexParser &p, std::u16string_view source) {
  const auto text = UnitUtil::instance().trimSpaces(
      Property(property_host::debugText(source)));
  const auto body = ParseUtil::internalMath(p, property_host::string(text), 0);
  if (!body || body->empty())
    property_host::typeError(
        "Cannot read properties of undefined (reading 'childNodes')");
  const auto firstChildren = NodeUtil::instance().getChildren(body->get(0));
  if (!firstChildren || firstChildren->empty())
    property_host::typeError(
        "Cannot read properties of undefined (reading 'childNodes')");
  const auto contentChildren =
      NodeUtil::instance().getChildren(firstChildren->get(0));
  if (!contentChildren)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'length')");
  if (contentChildren->empty())
    return node(p, "mrow", {});
  std::vector<mml::NodeValue> children;
  const Property attrs(PropertyList{{"width", Property(".5ex")}});
  children.push_back(node(p, "mspace", {}, attrs));
  for (std::size_t i = 0; i < body->size(); ++i)
    children.push_back(body->get(i));
  children.push_back(node(p, "mspace", {}, attrs));
  return node(p, "mrow", children);
}
void set(mml::NodeValue value, std::string_view name, Property data) {
  bussproofs_util::setProperty(value, name, std::move(data));
}
Property prop(BaseItem &item, std::string_view name) {
  return stack_property_host::scalar(item.getProperty(name));
}
StackItemFactory &itemFactory(TexParser &p) {
  const auto value = p.itemFactory();
  const auto *factory = std::get_if<std::shared_ptr<StackItemFactory>>(&value);
  if (!factory || !*factory)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'create')");
  return **factory;
}
std::u16string currentName(TexParser &p) {
  return property_host::string(p.currentCS());
}
mml::NodeValue parseSequent(TexParser &p, std::u16string_view name) {
  if (p.GetNext() != u"$")
    throw TexError(
        u"IllegalUseOfCommand", u"Use of %1 does not match its definition.",
        std::array<Property, 1>{Property(property_host::debugText(name))});
  ++p.i;
  const std::u16string source = p.GetUpTo(std::u16string(name), u"$");
  const auto delimiter = source.find(u"\\fCenter");
  if (delimiter == std::u16string::npos)
    throw TexError(
        u"MissingProofCommand", u"Missing %1 in %2.",
        std::array<Property, 2>{Property("\\fCenter"),
                                Property(property_host::debugText(name))});
  const auto secondDelimiter = source.find(u"\\fCenter", delimiter + 8);
  const auto env = stack(p).env();
  auto &config = configuration(p);
  const auto left =
      TexParser::make(source.substr(0, delimiter), env, config)->mml();
  const auto right =
      TexParser::make(
          source.substr(delimiter + 8,
                        secondDelimiter == std::u16string::npos
                            ? std::u16string::npos
                            : secondDelimiter - delimiter - 8),
          env, config)
          ->mml();
  const auto center = TexParser::make(u"\\fCenter", env, config)->mml();
  const std::array<mml::NodeValue, 1> l{left}, c{center}, r{right};
  const std::array<mml::NodeValue, 3> cells{
      node(p, "mtd", l), node(p, "mtd", c), node(p, "mtd", r)};
  const auto row = node(p, "mtr", cells);
  const std::array<mml::NodeValue, 1> rows{row};
  auto table =
      node(p, "mtable", rows,
           Property(PropertyList{{"columnspacing", Property(".5ex")},
                                 {"columnalign", Property("center 2")}}));
  set(table, "sequent", true);
  config.addNode(Property("sequent"), row);
  return table;
}
void addInference(TexParser &p, BaseItem &top, unsigned arity, bool sequent) {
  if (top.Size() < arity)
    throw TexError(u"BadProofTree", u"Proof tree badly specified.");
  const bool rootAtTop = property_host::truthy(prop(top, "rootAtTop"));
  bool unaryEmpty = false;
  if (arity == 1) {
    if (top.hasSourceNodesOwner()) {
      auto& access=top.sourceAccess();const auto receiver=access.receiver();const auto peek=access.getReceiver(u"Peek");
      const auto raw=access.callMember(peek,receiver,{});const auto first=access.getIndex(raw,0);
      const auto childNodes=access.get(first,options::Key(u"childNodes"));
      unaryEmpty=!access.truthy(access.get(childNodes,options::Key(u"length")));
    } else {
      const auto peek = top.Peek();
      const auto first = peek->get(0);
      unaryEmpty = first.isNode() && NodeUtil::instance().getChildren(first)->empty();
    }
  }
  const unsigned childCount = unaryEmpty ? 0 : arity;
  std::vector<mml::NodeValue> columns;
  for (unsigned i = 0; i < arity; ++i) {
    if (i)
      columns.insert(columns.begin(), node(p, "mtd", {}));
    const auto premise = top.Pop();
    const std::array<mml::NodeValue, 1> one{premise};
    columns.insert(
        columns.begin(),
        node(p, "mtd", one,
             Property(PropertyList{
                 {"rowalign", Property(rootAtTop ? "top" : "bottom")}})));
  }
  const auto row = node(p, "mtr", columns);
  const std::array<mml::NodeValue, 1> rowChild{row};
  const auto premise =
      node(p, "mtable", rowChild,
           Property(PropertyList{{"framespacing", Property("0 0")}}));
  mml::NodeValue conclusion;
  std::u16string style = property_host::string(prop(top, "currentLine"));
  if (sequent)
    conclusion = parseSequent(p, currentName(p));
  else
    conclusion =
        padded(p, property_host::string(p.GetArgument(currentName(p))));
  const auto previousLine = prop(top, "line");
  if (previousLine != prop(top, "currentLine"))
    top.setProperty("currentLine", StackProperty(previousLine));
  std::vector<mml::NodeValue> ruleRows;
  const std::array<mml::NodeValue, 1> upperCell{premise};
  const auto upper = node(p, "mtd", upperCell);
  const std::array<mml::NodeValue, 1> lowerCell{conclusion};
  const auto lower = node(p, "mtd", lowerCell);
  const std::array<mml::NodeValue, 1> upperRowChild{upper},
      lowerRowChild{lower};
  const auto upperRow = node(p, "mtr", upperRowChild),
             lowerRow = node(p, "mtr", lowerRowChild);
  ruleRows = rootAtTop ? std::vector<mml::NodeValue>{lowerRow, upperRow}
                       : std::vector<mml::NodeValue>{upperRow, lowerRow};
  auto rule = node(p, "mtable", ruleRows,
                   Property(PropertyList{
                       {"align", Property("top 2")},
                       {"rowlines", Property(property_host::debugText(style))},
                       {"framespacing", Property("0 0")}}));
  set(rule, "inferenceRule", Property(rootAtTop ? "up" : "down"));
  const mml::NodeValue leftLabel =
                           stack_property_host::node(top.getProperty("left")),
                       rightLabel =
                           stack_property_host::node(top.getProperty("right"));
  std::vector<mml::NodeValue> labelled;
  auto wrapLabel = [&](mml::NodeValue value, bool right) {
    if (!value.isNode())
      return mml::NodeValue(PropertyNull{});
    const std::array<mml::NodeValue, 1> child{value};
    PropertyList attrs{{"height", Property(right ? "-.25em" : ".25em")},
                       {"depth", Property("+.25em")},
                       {"width", Property("+.5ex")},
                       {"voffset", Property("-.25em")}};
    if (right)
      attrs.setOwn("lspace", Property(".5ex"));
    return node(p, "mpadded", child, Property(attrs));
  };
  const auto left = wrapLabel(leftLabel, false),
             right = wrapLabel(rightLabel, true);
  if (left.isNode()) {
    set(left, "prooflabel", Property("left"));
    labelled.push_back(left);
  }
  labelled.push_back(rule);
  if (right.isNode()) {
    set(right, "prooflabel", Property("right"));
    labelled.push_back(right);
  }
  if (labelled.size() > 1) {
    const std::string label =
        left.isNode() ? (right.isNode() ? "both" : "left") : "right";
    rule = node(p, "mrow", labelled);
    set(rule, "labelledRule", Property(label));
  }
  top.setProperty("left", StackProperty(PropertyNull{}));
  top.setProperty("right", StackProperty(PropertyNull{}));
  set(rule, "inference", Property(static_cast<double>(childCount)));
  configuration(p).addNode(Property("inference"), rule);
  const std::array<mml::NodeValue, 1> result{rule};
  top.Push(result);
}
FunctionRef
callable(std::function<ParseResult(const FunctionReceiver &, ParserContext &,
                                   const ParseAtom &, ParseRest)>
             fn) {
  return makeParseCallable(std::move(fn));
}
} // namespace
BussproofsMethodSet BussproofsMethods::create(ParserRuntime &runtime) {
  Context &cx = runtime.context();
  PropertyList methods;
  auto add = [&](std::u16string_view name, FunctionRef fn, double length) {
    const Value bound = BaseMethods::bind(runtime, fn, std::u16string(name));
    const auto *identity = std::get_if<ObjectRef>(&bound.data);
    if (!identity)
      throw Exception("TypeError",
                      u"Bussproofs method has no Context identity");
    cx.define(*identity, Key(u"length"),
              Descriptor::data(Value(length), false, false, true));
    methods.setOwn(std::string(name.begin(), name.end()),
                   Property(std::move(fn)));
  };
  add(u"Prooftree",
      callable([](const FunctionReceiver &, ParserContext &context,
                  const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &p = parser(context);
        StackItemRef begin;
        if (const auto *ref = std::get_if<StackItemRef>(&atom))
          begin = *ref;
        else {
          const auto current = stack(p).Top();
          if (const auto *ref = std::get_if<StackItemRef>(&current))
            begin = *ref;
        }
        if (begin) {
          const std::array<StackEntry, 1> entry{begin};
          stack(p).Push(entry);
        }
        auto *item = itemFactory(p).create("proofTree", {});
        if (!item)
          property_host::typeError(
              "ProofTreeItem constructor is not registered");
        Property name = Missing{};
        if (begin)
          if (auto *base = dynamic_cast<BaseItem *>(begin.get()))
            name = stack_property_host::scalar(base->getName());
        item->setProperties(
            StackProperty(PropertyList{{"name", name},
                                       {"line", Property("solid")},
                                       {"currentLine", Property("solid")},
                                       {"rootAtTop", false}}));
        return itemFactory(p).reference(item);
      }),
      2);
  add(u"Axiom",
      callable([](const FunctionReceiver &, ParserContext &context,
                  const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &p = parser(context);
        auto &top = proofTree(p);
        auto content = padded(
            p, property_host::string(p.GetArgument(atomText(atom))));
        set(content, "axiom", true);
        const std::array<mml::NodeValue, 1> values{content};
        top.Push(values);
        return Property(Missing{});
      }),
      2);
  auto inference = [&](bool sequent) {
    return callable([sequent](const FunctionReceiver &, ParserContext &context,
                              const ParseAtom &atom,
                              ParseRest rest) -> ParseResult {
      auto &p = parser(context);
      auto &top = proofTree(p);
      unsigned n = 1;
      if (!rest.empty())
        if (const auto *value = std::any_cast<Property>(&rest[0]))
          n = static_cast<unsigned>(property_host::number(*value));
      addInference(p, top, n, sequent);
      return Property(Missing{});
    });
  };
  add(u"Inference", inference(false), 3);
  add(u"InferenceF", inference(true), 3);
  add(u"Label",
      callable([](const FunctionReceiver &, ParserContext &context,
                  const ParseAtom &atom, ParseRest rest) -> ParseResult {
        auto &p = parser(context);
        auto &top = proofTree(p);
        const auto content = ParseUtil::internalMath(
            p, property_host::string(p.GetArgument(atomText(atom))), 0);
        mml::NodeValue label(PropertyNull{});
        if (content && !content->empty())
          label = content->get(0);
        if (content && content->size() > 1) {
          std::vector<mml::NodeValue> values;
          for (std::size_t i = 0; i < content->size(); ++i)
            values.push_back(content->get(i));
          label = node(p, "mrow", values);
        }
        std::u16string side = u"right";
        if (!rest.empty())
          if (const auto *v = std::any_cast<Property>(&rest[0]))
            side = property_host::string(*v);
        top.setProperty(std::string(side.begin(), side.end()),
                        content && !content->empty()
                            ? StackProperty(label)
                            : StackProperty(Missing{}));
        return Property(Missing{});
      }),
      3);
  add(u"SetLine",
      callable([](const FunctionReceiver &, ParserContext &context,
                  const ParseAtom &, ParseRest rest) -> ParseResult {
        auto &p = parser(context);
        auto &top = proofTree(p);
        const auto style = std::any_cast<Property>(rest[0]);
        const bool always =
            property_host::truthy(std::any_cast<Property>(rest[1]));
        top.setProperty("currentLine", StackProperty(style));
        if (always)
          top.setProperty("line", StackProperty(style));
        return Property(Missing{});
      }),
      4);
  add(u"RootAtTop",
      callable([](const FunctionReceiver &, ParserContext &context,
                  const ParseAtom &, ParseRest rest) -> ParseResult {
        auto &top = proofTree(parser(context));
        top.setProperty("rootAtTop",
                        StackProperty(std::any_cast<Property>(rest[0])));
        return Property(Missing{});
      }),
      3);
  add(u"AxiomF",
      callable([](const FunctionReceiver &, ParserContext &context,
                  const ParseAtom &atom, ParseRest) -> ParseResult {
        auto &p = parser(context);
        auto &top = proofTree(p);
        auto line = parseSequent(p, atomText(atom));
        set(line, "axiom", true);
        const std::array<mml::NodeValue, 1> values{line};
        top.Push(values);
        return Property(Missing{});
      }),
      2);
  add(u"FCenter",
      callable([](const FunctionReceiver &, ParserContext &, const ParseAtom &,
                  ParseRest) -> ParseResult { return Property(Missing{}); }),
      2);
  return {Property(methods)};
}
} // namespace folia::math::tex
