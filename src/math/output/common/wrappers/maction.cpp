/*************************************************************
 * Native CommonMaction methods mirroring MathJax 4.1.3.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :maction;
import :wrapper;
import :mo;
import folia.math.core.mml_tree.mml_nodes.maction;
import std;

namespace folia::math::output::common {
void CommonMaction::initializeSource() {
    CommonWrapper::initializeSource();
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
