/*************************************************************
 * Candidate typed port of MathJax 4.1.3 output/common/Wrappers/munderover.ts.
 *************************************************************/
module folia.math.output.common;
import :munderover;
import :scriptbase;
import :wrapper;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.util.bbox;
import std;

namespace folia::math::output::common {
namespace {
mml::MmlMunderover const &limitsNode(mml::MmlNode const &node) {
    const auto *result = dynamic_cast<mml::MmlMunderover const *>(&node);
    if (!result)
        throw std::logic_error("under/over wrapper requires an MmlMunderover node");
    return *result;
}

CommonWrapper *childAt(CommonWrapper const &wrapper, std::size_t index) {
    if (index >= wrapper.childNodes.size() || !wrapper.childNodes[index])
        throw std::logic_error("under/over wrapper is missing an indexed child wrapper");
    return wrapper.childNodes[index];
}

bool sourceTruthy(mml::AttributeValue const *value) {
    if (!value || std::holds_alternative<mml::AttributeNull>(*value))
        return false;
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean;
    if (const auto *number = std::get_if<double>(value))
        return *number != 0 && !std::isnan(*number);
    return !std::get<std::u16string>(*value).empty();
}

bool hasAccent(CommonWrapper const &wrapper) {
    return sourceTruthy(wrapper.node->attributes().lookup(u"accent"));
}

double parameter(CommonWrapper const &wrapper, std::string_view name) {
    return wrapper.jax().face().parameters.required(name);
}
} // namespace

CommonMunder::CommonMunder(CommonWrapperFactory &factory, mml::MmlNode &node, CommonWrapper *parent)
    : CommonMsub(factory, node, parent, limitsNode(node).under()) {}

void CommonMunder::initializeSource() {
    CommonMsub::initializeSource();
    stretchChildren();
}

CommonWrapper *CommonMunder::scriptChild() const {
    return childAt(*this, limitsNode(*node).under());
}

void CommonMunder::computeBBox(BBox &box, bool recompute) {
    if (hasMovableLimits()) {
        CommonMsub::computeBBox(box, recompute);
        return;
    }
    box.empty();
    const BBox base = baseChild()->getOuterBBox();
    const BBox under = scriptChild()->getOuterBBox();
    const double v = getUnderKV(base, under)[1];
    const double delta = isLineBelow ? 0 : getDelta(*scriptChild(), true);
    const auto widths = getDeltaW({base, under}, {0, -delta, 0});
    box.combine(base, widths[0], 0);
    box.combine(under, widths[1], v);
    box.d += parameter(*this, "big_op_spacing5");
    box.clean();
    setChildPWidths(recompute);
}

SceneNodes CommonMunder::emitScene(scene::Builder &builder,
                                   std::span<scene::NodeId const> parents) {
    if (auto embellished = toEmbellishedScene(builder, parents))
        return *embellished;
    if (hasMovableLimits())
        return CommonMsub::emitScene(builder, parents);
    auto containers = standardSceneNodes(builder, parents);
    auto *baseChildValue = baseChild();
    auto *underChildValue = scriptChild();
    if (!baseChildValue || !underChildValue)
        throw std::logic_error("under wrapper requires base and under children");
    const BBox base = baseChildValue->getOuterBBox();
    const BBox under = underChildValue->getOuterBBox();
    const double delta = isLineBelow ? 0 : getDelta(*underChildValue, true);
    const double v = getUnderKV(base, under)[1];
    const auto widths = getDeltaW({base, under}, {0, -delta, 0});
    auto emittedBase = baseChildValue->emitScene(builder, containers.content);
    auto emittedUnder = underChildValue->emitScene(builder, containers.content);
    baseChildValue->placeScene(builder, emittedBase, widths[0], 0);
    underChildValue->placeScene(builder, emittedUnder, widths[1], v);
    return containers.nodes;
}

CommonMover::CommonMover(CommonWrapperFactory &factory, mml::MmlNode &node, CommonWrapper *parent)
    : CommonMsup(factory, node, parent, limitsNode(node).over()) {}

void CommonMover::initializeSource() {
    CommonMsup::initializeSource();
    stretchChildren();
}

CommonWrapper *CommonMover::scriptChild() const {
    return childAt(*this, limitsNode(*node).over());
}

void CommonMover::computeBBox(BBox &box, bool) {
    if (hasMovableLimits()) {
        CommonMsup::computeBBox(box);
        return;
    }
    box.empty();
    BBox base = baseChild()->getOuterBBox();
    const BBox over = scriptChild()->getOuterBBox();
    if (hasAccent(*this))
        base.h = std::max(base.h, parameter(*this, "x_height") * baseScale);
    const double u = getOverKU(base, over)[1];
    const double delta = isLineAbove ? 0 : getDelta(*scriptChild());
    const auto widths = getDeltaW({base, over}, {0, delta, 0});
    box.combine(base, widths[0], 0);
    box.combine(over, widths[1], u);
    box.h += parameter(*this, "big_op_spacing5");
    box.clean();
}

SceneNodes CommonMover::emitScene(scene::Builder &builder, std::span<scene::NodeId const> parents) {
    if (auto embellished = toEmbellishedScene(builder, parents))
        return *embellished;
    if (hasMovableLimits())
        return CommonMsup::emitScene(builder, parents);
    auto containers = standardSceneNodes(builder, parents);
    auto *baseChildValue = baseChild();
    auto *overChildValue = scriptChild();
    if (!baseChildValue || !overChildValue)
        throw std::logic_error("over wrapper requires base and over children");
    const BBox base = baseChildValue->getOuterBBox();
    const BBox over = overChildValue->getOuterBBox();
    const double delta = isLineAbove ? 0 : getDelta(*overChildValue);
    const double u = getOverKU(base, over)[1];
    const auto widths = getDeltaW({base, over}, {0, delta, 0});
    auto emittedBase = baseChildValue->emitScene(builder, containers.content);
    auto emittedOver = overChildValue->emitScene(builder, containers.content);
    baseChildValue->placeScene(builder, emittedBase, widths[0], 0);
    overChildValue->placeScene(builder, emittedOver, widths[1], u);
    return containers.nodes;
}

CommonMunderover::CommonMunderover(CommonWrapperFactory &factory, mml::MmlNode &node,
                                   CommonWrapper *parent)
    : CommonMsubsup(factory, node, parent, limitsNode(node).over()) {}

void CommonMunderover::initializeSource() {
    CommonMsubsup::initializeSource();
    stretchChildren();
}

CommonWrapper *CommonMunderover::underChild() const {
    return childAt(*this, limitsNode(*node).under());
}

CommonWrapper *CommonMunderover::overChild() const {
    return childAt(*this, limitsNode(*node).over());
}

CommonWrapper *CommonMunderover::subChild() const {
    return underChild();
}

CommonWrapper *CommonMunderover::supChild() const {
    return overChild();
}

CommonWrapper *CommonMunderover::scriptChild() const {
    return overChild();
}

void CommonMunderover::computeBBox(BBox &box, bool) {
    if (hasMovableLimits()) {
        CommonMsubsup::computeBBox(box);
        return;
    }
    box.empty();
    const BBox over = overChild()->getOuterBBox();
    BBox base = baseChild()->getOuterBBox();
    const BBox under = underChild()->getOuterBBox();
    if (hasAccent(*this))
        base.h = std::max(base.h, parameter(*this, "x_height") * baseScale);
    const double u = getOverKU(base, over)[1];
    const double v = getUnderKV(base, under)[1];
    const double overDelta = getDelta(*overChild());
    const double underDelta = getDelta(*underChild(), true);
    const auto widths = getDeltaW({base, under, over},
                                  {0, isLineBelow ? 0 : -underDelta, isLineAbove ? 0 : overDelta});
    box.combine(base, widths[0], 0);
    box.combine(over, widths[2], u);
    box.combine(under, widths[1], v);
    const double spacing = parameter(*this, "big_op_spacing5");
    box.h += spacing;
    box.d += spacing;
    box.clean();
}

SceneNodes CommonMunderover::emitScene(scene::Builder &builder,
                                       std::span<scene::NodeId const> parents) {
    if (auto embellished = toEmbellishedScene(builder, parents))
        return *embellished;
    if (hasMovableLimits())
        return CommonMsubsup::emitScene(builder, parents);
    auto containers = standardSceneNodes(builder, parents);
    auto *baseChildValue = baseChild();
    auto *underChildValue = underChild();
    auto *overChildValue = overChild();
    if (!baseChildValue || !underChildValue || !overChildValue)
        throw std::logic_error("munderover wrapper requires base, under and over children");
    const BBox over = overChildValue->getOuterBBox();
    const BBox base = baseChildValue->getOuterBBox();
    const BBox under = underChildValue->getOuterBBox();
    const double overDelta = getDelta(*overChildValue);
    const double underDelta = getDelta(*underChildValue, true);
    const double u = getOverKU(base, over)[1];
    const double v = getUnderKV(base, under)[1];
    const auto widths = getDeltaW({base, under, over},
                                  {0, isLineBelow ? 0 : -underDelta, isLineAbove ? 0 : overDelta});
    auto emittedBase = baseChildValue->emitScene(builder, containers.content);
    auto emittedUnder = underChildValue->emitScene(builder, containers.content);
    auto emittedOver = overChildValue->emitScene(builder, containers.content);
    baseChildValue->placeScene(builder, emittedBase, widths[0], 0);
    underChildValue->placeScene(builder, emittedUnder, widths[1], v);
    overChildValue->placeScene(builder, emittedOver, widths[2], u);
    return containers.nodes;
}
} // namespace folia::math::output::common
