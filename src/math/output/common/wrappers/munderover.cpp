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
    : CommonScriptbase(factory, node, parent, true, limitsNode(node).under()) {
}

void CommonMunder::initializeSource() {
    CommonScriptbase::initializeSource();
    stretchChildren();
}

CommonWrapper *CommonMunder::scriptChild() const {
    return childAt(*this, limitsNode(*node).under());
}

void CommonMunder::computeBBox(BBox &box, bool recompute) {
    if (hasMovableLimits()) {
        CommonScriptbase::computeBBox(box, recompute);
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

CommonMover::CommonMover(CommonWrapperFactory &factory, mml::MmlNode &node, CommonWrapper *parent)
    : CommonScriptbase(factory, node, parent, true, limitsNode(node).over()) {
}

void CommonMover::initializeSource() {
    CommonScriptbase::initializeSource();
    stretchChildren();
}

CommonWrapper *CommonMover::scriptChild() const {
    return childAt(*this, limitsNode(*node).over());
}

void CommonMover::computeBBox(BBox &box, bool) {
    if (hasMovableLimits()) {
        CommonScriptbase::computeBBox(box);
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

CommonMunderover::CommonMunderover(CommonWrapperFactory &factory, mml::MmlNode &node,
                                   CommonWrapper *parent)
    : CommonScriptbase(factory, node, parent, true, limitsNode(node).under()) {
}

void CommonMunderover::initializeSource() {
    CommonScriptbase::initializeSource();
    stretchChildren();
}

CommonWrapper *CommonMunderover::underChild() const {
    return childAt(*this, limitsNode(*node).under());
}

CommonWrapper *CommonMunderover::overChild() const {
    return childAt(*this, limitsNode(*node).over());
}

CommonWrapper *CommonMunderover::scriptChild() const {
    return underChild();
}

void CommonMunderover::computeBBox(BBox &box, bool) {
    if (hasMovableLimits()) {
        CommonScriptbase::computeBBox(box);
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
} // namespace folia::math::output::common
