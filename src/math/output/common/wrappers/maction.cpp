/*************************************************************
 * Native CommonMaction methods mirroring MathJax 4.1.3.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :maction;
import :wrapper;
import :mo;
import folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.util.string;
import std;

namespace folia::math::output::common {
void CommonMaction::initializeSource() {
    CommonWrapper::initializeSource();
    getParameters();
}

void CommonMaction::getParameters() {
    const auto *value = node->attributes().lookup(u"data-offsets");
    std::u16string_view offsets;
    if (const auto *text = value ? std::get_if<std::u16string>(value) : nullptr) {
        offsets = *text;
    } else if (value && !std::holds_alternative<mml::AttributeNull>(*value)) {
        if (const auto *number = std::get_if<double>(value);
            number && (*number == 0 || std::isnan(*number))) {
            // Source `offsets || ''` treats zero and NaN as empty defaults.
        } else if (const auto *boolean = std::get_if<bool>(value); boolean && !*boolean) {
            // Source `offsets || ''` treats false as the empty default.
        } else {
            throw std::invalid_argument("maction data-offsets must be text when truthy");
        }
    }

    const auto parts = string_util::split(offsets);
    const std::u16string_view dx =
        !parts.empty() && !parts[0].empty() ? std::u16string_view(parts[0]) : u".2em";
    const std::u16string_view dy =
        parts.size() > 1 && !parts[1].empty() ? std::u16string_view(parts[1]) : u".1em";
    tipDx_ = length2em(LengthInput(std::u16string(dx)));
    tipDy_ = length2em(LengthInput(std::u16string(dy)));
}

CommonWrapper *CommonMaction::selected() {
    auto *action = dynamic_cast<mml::MmlMaction *>(node);
    if (!action)
        throw std::logic_error("CommonMaction requires MmlMaction");

    // Delegate selection, clamping, invalid-number handling and the fallback
    // row to the actual MML family method. Common only finds its matching
    // already-wrapped child, or wraps that returned fallback node.
    auto &selectedNode = action->selected();
    const auto nodes = node->children();
    for (std::size_t i = 0; i < nodes.size() && i < childNodes.size(); ++i) {
        if (nodes[i] == &selectedNode && childNodes[i])
            return childNodes[i];
    }
    return &wrapNode(selectedNode);
}

void CommonMaction::computeBBox(BBox &box, bool recompute) {
    auto *child = selected();
    if (!child)
        throw std::logic_error("Maction selected no wrapper");
    box.updateFrom(child->getOuterBBox());
    (void)child->setChildPWidths(recompute);
}

std::size_t CommonMaction::breakCount() {
    auto *child = selected();
    if (node->isEmbellished()) {
        auto *mo = child ? dynamic_cast<CommonMo *>(child->coreMO()) : nullptr;
        if (!mo)
            throw std::logic_error("embellished Maction has no selected core mo");
        return mo->embellishedBreakCount();
    }
    return child ? child->breakCount() : 0;
}

std::shared_ptr<LineBBox> CommonMaction::computeLineBBox(std::size_t index) {
    auto *child = selected();
    if (!child)
        throw std::logic_error("Maction selected no wrapper");
    return getChildLineBBox(*child, index);
}
} // namespace folia::math::output::common
