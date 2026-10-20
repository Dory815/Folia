// MathJax 4.1.3 input/tex/empheq/EmpheqUtil.ts; Apache-2.0.
module folia.math.input.tex.empheq.util;
import folia.math.util.utf16;
import std;

namespace folia::math::tex {
namespace {
using mml::MmlNode;
using mml::NodeKind;
MmlNode &node(TexParser &parser, NodeKind kind, std::initializer_list<MmlNode *> children = {},
              std::initializer_list<mml::AttributeSet::Entry> attributes = {}) {
    NodeDefinition definition;
    for (auto const &[name, value] : attributes)
        definition.attributes.set(name, value);
    return parser.create(kind, {children.begin(), children.size()}, std::move(definition));
}
std::u16string attribute(MmlNode const &node, std::u16string_view name,
                         std::u16string_view missing = {}) {
    auto value = node.attributes().lookup(name);
    return value ? std::get<std::u16string>(*value) : std::u16string(missing);
}
// Preserve Source's literal-space split/slice/join, including empty entries.
std::vector<std::u16string_view> words(std::u16string_view text) {
    std::vector<std::u16string_view> result;
    std::size_t at{};
    for (;;) {
        auto end = text.find(u' ', at);
        result.push_back(text.substr(at, end == text.npos ? text.size() - at : end - at));
        if (end == text.npos)
            return result;
        at = end + 1;
    }
}
std::u16string prefix(std::u16string_view text, std::optional<std::size_t> count) {
    auto parts = words(text);
    // Missing count denotes slice(0, -1), used for a zero-column empty row.
    parts.resize(count ? std::min(*count, parts.size()) : parts.size() - 1);
    std::u16string result;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i)
            result += u' ';
        result += parts[i];
    }
    return result;
}
} // namespace
mml::AttributeSet EmpheqUtil::splitOptions(std::u16string_view text,
                                           std::optional<std::span<KeyValueOption const>> allowed) {
    return parse_util::keyvalOptions(text, allowed, true);
}
std::size_t EmpheqUtil::columnCount(MmlNode const &table) {
    std::size_t count{};
    for (auto row : table.children()) {
        const auto labels = row->isKind(NodeKind::mlabeledtr) ? 1U : 0U;
        if (row->children().size() > labels)
            count = std::max(count, row->children().size() - labels);
    }
    return count;
}
MmlNode &EmpheqUtil::cellBlock(std::u16string_view tex, MmlNode &table, TexParser &parser,
                               std::u16string_view env) {
    auto &padded = node(parser, NodeKind::mpadded, {},
                        {{u"height", 0.0}, {u"depth", 0.0}, {u"voffset", u"-1height"}});
    TexParser nested(std::u16string(tex), ParserEnvironmentSeed{parser.stack.env(), {}},
                     parser.configuration);
    auto result = nested.mml();
    if (!result)
        throw std::logic_error("Empheq side material produced no MML");
    if (!env.empty() && !nested.tags().label().empty()) {
        nested.configuration.equation.current_tag.environment = utf16::encode(env);
        nested.tags().getTag(true);
    }
    if (result->isInferred()) {
        // Reparenting removes children from the old row. Retain the original order.
        auto children = std::vector(result->children().begin(), result->children().end());
        for (auto child : children)
            padded.appendChild(*child);
    } else
        padded.appendChild(*result);
    padded.appendChild(node(parser, NodeKind::mphantom,
                            {&node(parser, NodeKind::mpadded, {&table}, {{u"width", 0.0}})}));
    return padded;
}
MmlNode &EmpheqUtil::topRowTable(MmlNode const &original, TexParser &parser) {
    auto &table = parse_util::copyNode(original, parser);
    auto first = std::vector(table.children().begin(), table.children().end());
    first.resize(std::min<std::size_t>(1, first.size()));
    table.setChildren(first);
    table.attributes().set(u"align", u"baseline 1");
    auto &padded = node(parser, NodeKind::mpadded, {&table}, {{u"width", 0.0}});
    const std::array children{&padded};
    // Source deliberately bypasses parser.create for this one phantom.
    return original.document().factory().make(NodeKind::mphantom, {}, children);
}
void EmpheqUtil::rowspanCell(MmlNode &cell, std::u16string_view tex, MmlNode const &table,
                             TexParser &parser, std::u16string_view env) {
    auto &content = cellBlock(tex, parse_util::copyNode(table, parser), parser, env);
    auto &top = topRowTable(table, parser);
    cell.appendChild(node(parser, NodeKind::mpadded, {&content, &top},
                          {{u"height", 0.0}, {u"depth", 0.0}, {u"voffset", u"height"}}));
}
void EmpheqUtil::left(MmlNode &table, MmlNode const &original, std::u16string_view tex,
                      TexParser &parser, std::u16string_view env) {
    table.attributes().set(u"columnalign",
                           u"right " + attribute(table, u"columnalign", u"undefined"));
    table.attributes().set(u"columnspacing",
                           u"0em " + attribute(table, u"columnspacing", u"undefined"));
    if (table.children().empty())
        table.appendChild(node(parser, NodeKind::mtr));
    MmlNode *firstCell{};
    for (std::size_t i = table.children().size(); i > 0; --i) {
        auto &row = *table.children()[i - 1];
        firstCell = &node(parser, NodeKind::mtd);
        auto children = std::vector(row.children().begin(), row.children().end());
        const auto label = row.isKind(NodeKind::mlabeledtr) && !children.empty() ? 1U : 0U;
        children.insert(children.begin() + label, firstCell);
        row.setChildren(children);
    }
    rowspanCell(*firstCell, tex, original, parser, env);
}
void EmpheqUtil::right(MmlNode &table, MmlNode const &original, std::u16string_view tex,
                       TexParser &parser, std::u16string_view env) {
    if (table.children().empty())
        table.appendChild(node(parser, NodeKind::mtr));
    auto &row = *table.children().front();
    const auto count = columnCount(table) + (row.isKind(NodeKind::mlabeledtr) ? 1U : 0U);
    while (row.children().size() < count)
        row.appendChild(node(parser, NodeKind::mtd));
    auto &cell = row.appendChild(node(parser, NodeKind::mtd));
    rowspanCell(cell, tex, original, parser, env);
    table.attributes().set(u"columnalign",
                           prefix(attribute(table, u"columnalign"), count) + u" left");
    auto spacing = table.attributes().lookup(u"columnspacing");
    if (!spacing || !std::holds_alternative<std::u16string>(*spacing))
        throw std::invalid_argument("Empheq right side requires string columnspacing");
    table.attributes().set(u"columnspacing",
                           prefix(attribute(table, u"columnspacing"),
                                  count ? std::optional<std::size_t>(count - 1) : std::nullopt) +
                               u" 0em");
}
void EmpheqUtil::adjustTable(BeginItem &empheq, TexParser &parser) {
    const auto leftText = empheq.left.value_or(u"");
    const auto rightText = empheq.right.value_or(u"");
    if (leftText.empty() && rightText.empty())
        return;
    auto table = empheq.Last();
    if (!table)
        throw std::logic_error("Empheq begin frame has no table");
    auto &original = parse_util::copyNode(*table, parser);
    if (!leftText.empty())
        left(*table, original, leftText, parser);
    if (!rightText.empty())
        right(*table, original, rightText, parser);
}
bool EmpheqUtil::checkEnv(std::u16string_view env) {
    if (env.ends_with(u'*'))
        env.remove_suffix(1);
    return std::ranges::find(allowEnv, env) != allowEnv.end();
}
} // namespace folia::math::tex
