/*************************************************************
 * Native CommonMfenced methods mirroring MathJax 4.1.3.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :mfenced;
import :mrow;
import :inferred_mrow;
import :wrapper;
import folia.math.core.mml_tree.mml_nodes.mfenced;
import std;

namespace folia::math::output::common {
CommonMfenced::CommonMfenced(CommonWrapperFactory &factory, mml::MmlNode &source,
                             CommonWrapper *parentValue)
    : CommonWrapper(factory, source, parentValue) {
}

void CommonMfenced::initializeSource() {
    CommonWrapper::initializeSource();
    createMrow();
    addMrowChildren();
}

void CommonMfenced::createMrow() {
    auto &rowNode = node->document().factory().make(mml::NodeKind::inferred_mrow);
    rowNode.inheritAttributesFrom(*node);
    auto &rowWrapper = wrapNode(rowNode);
    auto *inferred = dynamic_cast<CommonMrow *>(&rowWrapper);
    if (!inferred)
        throw std::logic_error("inferred mrow wrapper is not a CommonMrow");
    mrow = inferred;
    mrow->parent = this;
}

void CommonMfenced::addMo(mml::MmlNode *operatorNode) {
    if (!operatorNode)
        return;
    if (!mrow)
        throw std::logic_error("CommonMfenced has no inferred mrow");
    auto &wrapper = wrapNode(*operatorNode);
    mrow->childNodes.push_back(&wrapper);
    wrapper.parent = mrow;
}

void CommonMfenced::addMrowChildren() {
    auto *fenced = dynamic_cast<mml::MmlMfenced *>(node);
    if (!fenced)
        throw std::logic_error("CommonMfenced requires MmlMfenced");
    if (!mrow)
        throw std::logic_error("CommonMfenced has no inferred mrow");

    addMo(fenced->open);
    const auto count = childNodes.size();
    if (count != 0)
        mrow->childNodes.push_back(childNodes.front());
    for (std::size_t childIndex = 1; childIndex < count; ++childIndex) {
        const auto separatorIndex = childIndex - 1;
        if (separatorIndex < fenced->separators.size())
            addMo(fenced->separators[separatorIndex]);
        auto *child = childNodes[childIndex];
        if (!child)
            throw std::logic_error("CommonMfenced has a null child wrapper");
        mrow->childNodes.push_back(child);
    }
    addMo(fenced->close);
    mrow->stretchChildren();
}

std::size_t CommonMfenced::breakCount() {
    if (!mrow)
        throw std::logic_error("CommonMfenced has no inferred mrow");
    return mrow->breakCount();
}

void CommonMfenced::computeBBox(BBox &box, bool recompute) {
    if (!mrow)
        throw std::logic_error("CommonMfenced has no inferred mrow");
    box.updateFrom(mrow->getOuterBBox());
    (void)setChildPWidths(recompute);
}

std::shared_ptr<LineBBox> CommonMfenced::computeLineBBox(std::size_t index) {
    if (!mrow)
        throw std::logic_error("CommonMfenced has no inferred mrow");
    return mrow->getLineBBox(index);
}
} // namespace folia::math::output::common
