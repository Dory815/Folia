// MathJax 4.1.3 input/tex/amscd/AmsCdMethods.ts; Apache-2.0.
module folia.math.input.tex.amscd.methods;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.node_util;
import folia.math.core.mml_tree.mml_nodes.munderover;
import std;

namespace folia::math::tex {
namespace {
using mml::MmlNode;
using mml::NodeKind;
NodeDefinition attributes(std::initializer_list<mml::AttributeSet::Entry> entries) {
    NodeDefinition definition;
    for (auto const &[key, value] : entries)
        definition.attributes.set(key, value);
    return definition;
}
MmlNode &node(TexParser &parser, NodeKind kind, std::initializer_list<MmlNode *> children = {},
              NodeDefinition definition = {}) {
    return parser.create(kind, {children.begin(), children.size()}, std::move(definition));
}
MmlNode *label(TexParser &parser, std::u16string source) {
    return TexParser(std::move(source), ParserEnvironmentSeed{parser.stack.env()},
                     parser.configuration)
        .mml();
}
} // namespace

StackEntry AmsCdMethods::CD(TexParser &parser, FrameHandle const &begin,
                            std::span<TexArgument const>) {
    parser.Push(begin);
    auto item = parser.itemFactory().create(StackKind::array);
    auto &array = dynamic_cast<ArrayItem &>(*item);
    auto const &options = parser.configuration.options.amscd.value();
    auto const &env = parser.stack.env();
    array.cd_min_width =
        env.cd_min_width && !env.cd_min_width->empty() ? env.cd_min_width : options.harrowsize;
    array.cd_min_height =
        env.cd_min_height && !env.cd_min_height->empty() ? env.cd_min_height : options.varrowsize;
    array.arraydef.set(u"columnalign", u"center");
    if (options.colspace)
        array.arraydef.set(u"columnspacing", *options.colspace);
    if (options.rowspace)
        array.arraydef.set(u"rowspacing", *options.rowspace);
    array.arraydef.set(u"displaystyle", true);
    return item;
}

DispatchOutcome AmsCdMethods::arrow(TexParser &parser, MacroInvocation const &invocation) {
    const auto position = parser.position();
    const auto c = parser.GetNext();
    if (c.size() != 1 || std::u16string_view(u"><VA.|=").find(c) == std::u16string_view::npos) {
        parser.setPosition(position);
        Other(parser, invocation.dispatch_token);
        return NoParseResult{};
    }
    parser.setPosition(parser.position() + 1);
    auto first = parser.stack.Top();
    if (!first->isKind(StackKind::array) || first->Size()) {
        cell(parser, invocation);
        first = parser.stack.Top();
    }
    auto &top = dynamic_cast<ArrayItem &>(*first);
    const bool arrowRow = top.table.size() % 2 == 1;
    if ((top.row.size() + (arrowRow ? 0 : 1)) % 2)
        cell(parser, invocation);

    auto horizontal = attributes({{u"stretchy", true}});
    // A generic array can use @ without a CD dimension. Retain an explicit
    // unset minimum: omitting it would select mo's default instead.
    horizontal.attributes.set(u"minsize", top.cd_min_width
                                              ? mml::AttributeValue{*top.cd_min_width}
                                              : mml::AttributeValue{mml::AttributeNull{}});
    auto vertical =
        attributes({{u"stretchy", true}, {u"symmetric", true}, {u"lspace", 0.0}, {u"rspace", 0.0}});
    vertical.attributes.set(u"minsize", top.cd_min_height
                                            ? mml::AttributeValue{*top.cd_min_height}
                                            : mml::AttributeValue{mml::AttributeNull{}});
    MmlNode *output = nullptr;
    if (c == u"|") {
        output = &parser.createToken(NodeKind::mo, u"\u2225", std::move(vertical));
    } else if (c == u"=") {
        output = &parser.createToken(NodeKind::mo, u"=", std::move(horizontal));
    } else if (c != u".") {
        auto arrow = c == u">"   ? u"\u2192"
                     : c == u"<" ? u"\u2190"
                     : c == u"V" ? u"\u2193"
                                 : u"\u2191";
        const auto name = std::u16string(invocation.dispatch_token) + c;
        auto a = parser.GetUpTo(name, c);
        const auto b = parser.GetUpTo(name, c);
        if (c == u">" || c == u"<") {
            auto &arrowNode = parser.createToken(NodeKind::mo, arrow, std::move(horizontal));
            if (a.empty())
                a = u"\\kern " + top.cd_min_width.value_or(u"undefined");
            const auto pad = attributes(
                {{u"width", std::u16string(u"+.67em")}, {u"lspace", std::u16string(u".33em")}});
            auto &scripts = dynamic_cast<mml::MmlMunderover &>(
                node(parser, NodeKind::munderover, {&arrowNode}));
            auto &above = node(parser, NodeKind::mpadded, {label(parser, std::move(a))}, pad);
            above.attributes().set(u"voffset", u".1em");
            NodeUtil::setChild(scripts, scripts.over(), above);
            if (!b.empty())
                NodeUtil::setChild(scripts, scripts.under(),
                                   node(parser, NodeKind::mpadded, {label(parser, b)}, pad));
            output = &scripts;
            if (parser.configuration.options.amscd->hideHorizontalLabels.value_or(false))
                output =
                    &node(parser, NodeKind::mpadded, {output},
                          attributes({{u"depth", 0.0}, {u"height", std::u16string(u".67em")}}));
        } else {
            auto &arrowNode = parser.createToken(NodeKind::mo, arrow, std::move(vertical));
            output = &arrowNode;
            if (!a.empty() || !b.empty()) {
                output = &node(parser, NodeKind::mrow);
                if (!a.empty())
                    output->appendChild(*label(
                        parser, u"\\scriptstyle\\raise.125em{\\vcenter{\\llap{" + a + u"}}}"));
                arrowNode.setTexClass(mml::TexClass::ord);
                output->appendChild(arrowNode);
                if (!b.empty())
                    output->appendChild(*label(
                        parser, u"\\scriptstyle\\raise.125em{\\vcenter{\\rlap{" + b + u"}}}"));
            }
        }
    }
    if (output)
        parser.Push(output);
    return cell(parser, invocation);
}

DispatchOutcome AmsCdMethods::cell(TexParser &parser, MacroInvocation const &invocation) {
    auto top = parser.stack.Top();
    auto array = dynamic_cast<ArrayItem *>(top);
    if ((!array || array->table.size() % 2 == 0) && (!array || array->row.empty()))
        parser.Push(&node(parser, NodeKind::mpadded, {},
                          attributes({{u"height", std::u16string(u"8.5pt")},
                                      {u"depth", std::u16string(u"2pt")}})));
    auto item = parser.itemFactory().create(StackKind::cell);
    auto &cell = dynamic_cast<CellItem &>(*item);
    cell.isEntry = true;
    cell.name = invocation.dispatch_token;
    parser.Push(item);
    return NoParseResult{};
}

DispatchOutcome AmsCdMethods::minCDarrowwidth(TexParser &parser,
                                              MacroInvocation const &invocation) {
    parser.stack.env().cd_min_width = parser.GetDimen(invocation.dispatch_token);
    return NoParseResult{};
}
DispatchOutcome AmsCdMethods::minCDarrowheight(TexParser &parser,
                                               MacroInvocation const &invocation) {
    parser.stack.env().cd_min_height = parser.GetDimen(invocation.dispatch_token);
    return NoParseResult{};
}
} // namespace folia::math::tex
