// Source method split retained: selected-child projections from MathJax 4.1.3.
module folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.core.mml_tree.mml_document;
import folia.math.util.js_numeric_input;
import std;

namespace folia::math::mml {
namespace {
double selectionValue(const AttributeValue *value, double fallback) {
    if (!value)
        return fallback;
    if (const auto *number = std::get_if<double>(value))
        return *number;
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean ? 1.0 : 0.0;
    if (const auto *text = std::get_if<std::u16string>(value))
        return js_input::number(*text);
    if (std::holds_alternative<AttributeNull>(*value))
        return 0.0;
    return 0.0;
}
} // namespace

MmlNode &MmlMaction::selected() const {
    const auto raw = selectionValue(attributes().lookup(u"selection"), 1.0);
    const auto length = children().size();
    if (length != 0) {
        if (std::isnan(raw))
            return document().factory().make(NodeKind::mrow);
        const double index = std::max(1.0, std::min(static_cast<double>(length), raw)) - 1.0;
        if (std::isfinite(index) && index >= 0.0 && index < static_cast<double>(length) &&
            std::floor(index) == index) {
            return *children()[static_cast<std::size_t>(index)];
        }
    }
    return document().factory().make(NodeKind::mrow);
}

bool MmlMaction::isEmbellished() const {
    return selected().isEmbellished();
}

bool MmlMaction::isSpacelike() const {
    return selected().isSpacelike();
}

MmlNode *MmlMaction::core() {
    return selected().core();
}

MmlNode *MmlMaction::coreMO() {
    return selected().coreMO();
}
} // namespace folia::math::mml
