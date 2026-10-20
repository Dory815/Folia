// MathJax 4.1.3 input/tex/bussproofs/BussproofsMethods.ts; Apache-2.0.
module folia.math.input.tex.bussproofs.methods;
import folia.math.input.tex.parse_util;
import folia.math.input.tex.unit_util;
import std;
namespace folia::math::tex {
namespace {
using mml::MmlNode;
using mml::NodeKind;
using Args = std::span<TexArgument const>;
Args arguments(MacroInvocation const &invocation) {
    return std::get<Args>(invocation.arguments);
}
ProofTreeItem &proofTree(TexParser &parser) {
    auto *item = dynamic_cast<ProofTreeItem *>(parser.stack.Top());
    if (!item)
        throw TexError("IllegalProofCommand",
                       u"Proof commands only allowed in prooftree environment.");
    return *item;
}
NodeDefinition attrs(std::initializer_list<mml::AttributeSet::Entry> entries) {
    NodeDefinition definition;
    for (auto const &[key, value] : entries)
        definition.attributes.set(key, value);
    return definition;
}
MmlNode &node(TexParser &parser, NodeKind kind, std::initializer_list<MmlNode *> children = {},
              NodeDefinition definition = {}) {
    return parser.create(kind, {children.begin(), children.size()}, std::move(definition));
}
MmlNode &paddedContent(TexParser &parser, std::u16string_view source) {
    auto content =
        parse_util::internalMath(parser, UnitUtil::instance().trimSpaces(source), ScriptLevel{0.0});
    if (content.empty() || content.front()->children().empty() ||
        !content.front()->children().front())
        throw std::out_of_range("Bussproofs padded content has no inner row");
    if (content.front()->children().front()->children().empty())
        return node(parser, NodeKind::mrow);
    auto &left = node(parser, NodeKind::mspace, {}, attrs({{u"width", std::u16string(u".5ex")}}));
    auto &right = node(parser, NodeKind::mspace, {}, attrs({{u"width", std::u16string(u".5ex")}}));
    content.insert(content.begin(), &left);
    content.push_back(&right);
    return parser.create(NodeKind::mrow, content);
}
MmlNode &createRule(TexParser &parser, MmlNode &premise, MmlNode &conclusion, MmlNode *left,
                    MmlNode *right, std::u16string const &style, bool rootAtTop) {
    auto &upper = node(parser, NodeKind::mtr, {&node(parser, NodeKind::mtd, {&premise})});
    auto &lower = node(parser, NodeKind::mtr, {&node(parser, NodeKind::mtd, {&conclusion})});
    auto &rule = node(parser, NodeKind::mtable,
                      rootAtTop ? std::initializer_list<MmlNode *>{&lower, &upper}
                                : std::initializer_list<MmlNode *>{&upper, &lower},
                      attrs({{u"align", std::u16string(u"top 2")},
                             {u"rowlines", style},
                             {u"framespacing", std::u16string(u"0 0")}}));
    rule.properties().proof.setRuleDirection(rootAtTop ? mml::ProofRuleDirection::up
                                                       : mml::ProofRuleDirection::down);
    MmlNode *leftLabel{}, *rightLabel{};
    if (left) {
        leftLabel = &node(parser, NodeKind::mpadded, {left},
                          attrs({{u"height", std::u16string(u".25em")},
                                 {u"depth", std::u16string(u"+.25em")},
                                 {u"width", std::u16string(u"+.5ex")},
                                 {u"voffset", std::u16string(u"-.25em")}}));
        leftLabel->properties().proof.setProofLabel(mml::ProofSide::left);
    }
    if (right) {
        rightLabel = &node(parser, NodeKind::mpadded, {right},
                           attrs({{u"height", std::u16string(u"-.25em")},
                                  {u"depth", std::u16string(u"+.25em")},
                                  {u"width", std::u16string(u"+.5ex")},
                                  {u"voffset", std::u16string(u"-.25em")},
                                  {u"lspace", std::u16string(u".5ex")}}));
        rightLabel->properties().proof.setProofLabel(mml::ProofSide::right);
    }
    if (!left && !right)
        return rule;
    std::vector<MmlNode *> children;
    if (leftLabel)
        children.push_back(leftLabel);
    children.push_back(&rule);
    if (rightLabel)
        children.push_back(rightLabel);
    auto &result = parser.create(NodeKind::mrow, children);
    result.properties().proof.setLabelledRule(left && right ? mml::ProofLabels::both
                                              : left        ? mml::ProofLabels::left
                                                            : mml::ProofLabels::right);
    return result;
}
MmlNode &parseFCenterLine(TexParser &parser, std::u16string_view name) {
    if (parser.GetNext() != u"$") {
        const std::array<std::u16string, 1> args{std::u16string(name)};
        throw TexError("IllegalUseOfCommand", u"Use of %1 does not match its definition.", args);
    }
    parser.setPosition(parser.position() + 1);
    auto source = parser.GetUpTo(name, u"$");
    const auto split = source.find(u"\\fCenter");
    if (split == std::u16string::npos) {
        const std::array<std::u16string, 2> args{u"\\fCenter", std::u16string(name)};
        throw TexError("MissingProofCommand", u"Missing %1 in %2.", args);
    }
    const auto end = source.find(u"\\fCenter", split + 8);
    auto *premise = TexParser(source.substr(0, split), ParserEnvironmentSeed{parser.stack.env()},
                              parser.configuration)
                        .mml();
    auto *conclusion =
        TexParser(source.substr(split + 8, end == std::u16string::npos ? end : end - split - 8),
                  ParserEnvironmentSeed{parser.stack.env()}, parser.configuration)
            .mml();
    auto *fcenter =
        TexParser(u"\\fCenter", ParserEnvironmentSeed{parser.stack.env()}, parser.configuration)
            .mml();
    auto &left = node(parser, NodeKind::mtd, {premise});
    auto &middle = node(parser, NodeKind::mtd, {fcenter});
    auto &right = node(parser, NodeKind::mtd, {conclusion});
    auto &row = node(parser, NodeKind::mtr, {&left, &middle, &right});
    auto &table = node(parser, NodeKind::mtable, {&row},
                       attrs({{u"columnspacing", std::u16string(u".5ex")},
                              {u"columnalign", std::u16string(u"center 2")}}));
    table.properties().proof.setSequent(true);
    parser.configuration.addNode(u"sequent", row);
    return table;
}
DispatchOutcome inference(TexParser &parser, MacroInvocation const &invocation, bool sequent) {
    auto &top = proofTree(parser);
    auto count = static_cast<std::size_t>(std::get<double>(arguments(invocation)[0]));
    if (top.Size() < count)
        throw TexError("BadProofTree", u"Proof tree badly specified.");
    const auto childCount = count == 1 && top.Peek().front()->children().empty() ? 0 : count;
    std::vector<MmlNode *> children;
    do {
        if (!children.empty())
            children.insert(children.begin(), &node(parser, NodeKind::mtd));
        auto *premise = top.Pop();
        auto &cell =
            node(parser, NodeKind::mtd, {premise},
                 attrs({{u"rowalign", std::u16string(top.rootAtTop ? u"top" : u"bottom")}}));
        children.insert(children.begin(), &cell);
    } while (--count > 0);
    auto &row = parser.create(NodeKind::mtr, children);
    auto &table =
        node(parser, NodeKind::mtable, {&row}, attrs({{u"framespacing", std::u16string(u"0 0")}}));
    auto &conclusion =
        sequent ? parseFCenterLine(parser, invocation.dispatch_token)
                : paddedContent(parser, parser.GetArgument(invocation.dispatch_token).value());
    auto style = top.currentLine;
    if (style != top.line)
        top.currentLine = top.line;
    auto &rule = createRule(parser, table, conclusion, top.left, top.right, style, top.rootAtTop);
    top.left = nullptr;
    top.right = nullptr;
    rule.properties().proof.setPremiseCount(childCount);
    parser.configuration.addNode(u"inference", rule);
    const std::array<MmlNode *, 1> result{&rule};
    top.Push(result);
    return NoParseResult{};
}
} // namespace
StackEntry BussproofsMethods::Prooftree(TexParser &parser, FrameHandle const &begin,
                                        std::span<TexArgument const>) {
    parser.Push(begin);
    auto item = parser.itemFactory().create(StackKind::proof_tree);
    auto &proof = dynamic_cast<ProofTreeItem &>(*item);
    if (auto name = begin->getName())
        proof.name = std::u16string(*name);
    proof.line = u"solid";
    proof.currentLine = u"solid";
    proof.rootAtTop = false;
    return item;
}
DispatchOutcome BussproofsMethods::Axiom(TexParser &parser, MacroInvocation const &invocation) {
    auto &top = proofTree(parser);
    auto &content = paddedContent(parser, parser.GetArgument(invocation.dispatch_token).value());
    content.properties().proof.setAxiom(true);
    const std::array<MmlNode *, 1> result{&content};
    top.Push(result);
    return NoParseResult{};
}
DispatchOutcome BussproofsMethods::Inference(TexParser &p, MacroInvocation const &i) {
    return inference(p, i, false);
}
DispatchOutcome BussproofsMethods::InferenceF(TexParser &p, MacroInvocation const &i) {
    return inference(p, i, true);
}
DispatchOutcome BussproofsMethods::Label(TexParser &parser, MacroInvocation const &invocation) {
    auto &top = proofTree(parser);
    auto content = parse_util::internalMath(
        parser, parser.GetArgument(invocation.dispatch_token).value(), ScriptLevel{0.0});
    auto *label = content.size() > 1 ? &parser.create(NodeKind::mrow, content)
                  : content.empty()  ? nullptr
                                     : content.front();
    auto const &side = std::get<std::u16string>(arguments(invocation)[0]);
    if (side == u"left")
        top.left = label;
    else if (side == u"right")
        top.right = label;
    else
        throw std::invalid_argument("Bussproofs label side must be left or right");
    return NoParseResult{};
}
DispatchOutcome BussproofsMethods::SetLine(TexParser &parser, MacroInvocation const &invocation) {
    auto &top = proofTree(parser);
    auto args = arguments(invocation);
    top.currentLine = std::get<std::u16string>(args[0]);
    if (std::get<bool>(args[1]))
        top.line = top.currentLine;
    return NoParseResult{};
}
DispatchOutcome BussproofsMethods::RootAtTop(TexParser &parser, MacroInvocation const &invocation) {
    proofTree(parser).rootAtTop = std::get<bool>(arguments(invocation)[0]);
    return NoParseResult{};
}
DispatchOutcome BussproofsMethods::AxiomF(TexParser &parser, MacroInvocation const &invocation) {
    auto &top = proofTree(parser);
    auto &line = parseFCenterLine(parser, invocation.dispatch_token);
    line.properties().proof.setAxiom(true);
    const std::array<MmlNode *, 1> result{&line};
    top.Push(result);
    return NoParseResult{};
}
DispatchOutcome BussproofsMethods::FCenter(TexParser &, MacroInvocation const &) {
    return NoParseResult{};
}
} // namespace folia::math::tex
