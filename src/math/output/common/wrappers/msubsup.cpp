/*************************************************************
 * Candidate typed port of MathJax 4.1.3 output/common/Wrappers/msubsup.ts.
 *************************************************************/
module folia.math.output.common;
import :msubsup;
import :scriptbase;
import :wrapper;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.util.lengths;
import std;

namespace folia::math::output::common {
namespace {
mml::MmlMsubsup const &scriptNode(mml::MmlNode const &node) {
    const auto *script = dynamic_cast<mml::MmlMsubsup const *>(&node);
    if (!script)
        throw std::logic_error("script wrapper requires an MmlMsubsup node");
    return *script;
}

double parameter(CommonWrapper const &wrapper, std::string_view name) {
    return wrapper.jax().face().parameters.required(name);
}

LengthInput attrLength(CommonWrapper const &wrapper, std::u16string_view name) {
    const auto *value = wrapper.node->attributes().lookup(name);
    if (!value)
        return 0.0;
    if (const auto *text = std::get_if<std::u16string>(value))
        return *text;
    if (const auto *number = std::get_if<double>(value))
        return *number;
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean ? 1.0 : 0.0;
    return 0.0;
}
} // namespace

CommonMsub::CommonMsub(CommonWrapperFactory &factory, mml::MmlNode &node, CommonWrapper *parent,
                       std::optional<std::size_t> scriptChildIndex)
    : CommonScriptbase(factory, node, parent, useIC,
                       scriptChildIndex ? *scriptChildIndex : scriptNode(node).sub()) {
}

CommonWrapper *CommonMsub::scriptChild() const {
    const auto index = scriptNode(*node).sub();
    return index < childNodes.size() ? childNodes[index] : nullptr;
}

std::array<double, 2> CommonMsub::getOffset() const {
    return {baseIsChar ? 0 : getAdjustedIc(), -getV()};
}

CommonMsup::CommonMsup(CommonWrapperFactory &factory, mml::MmlNode &node, CommonWrapper *parent,
                       std::optional<std::size_t> scriptChildIndex)
    : CommonScriptbase(factory, node, parent, useIC,
                       scriptChildIndex ? *scriptChildIndex : scriptNode(node).sup()) {
}

CommonWrapper *CommonMsup::scriptChild() const {
    return childNodes.size() <= scriptNode(*node).sup() ? nullptr
                                                        : childNodes[scriptNode(*node).sup()];
}

std::array<double, 2> CommonMsup::getOffset() const {
    const double x = getAdjustedIc() - (baseRemoveIc ? 0 : baseIc);
    return {x, getU()};
}

CommonMsubsup::CommonMsubsup(CommonWrapperFactory &factory, mml::MmlNode &node,
                             CommonWrapper *parent, std::optional<std::size_t> scriptChildIndex)
    : CommonScriptbase(factory, node, parent, useIC,
                       scriptChildIndex ? *scriptChildIndex : scriptNode(node).sup()) {
}

CommonWrapper *CommonMsubsup::subChild() const {
    const auto index = scriptNode(*node).sub();
    return index < childNodes.size() ? childNodes[index] : nullptr;
}

CommonWrapper *CommonMsubsup::supChild() const {
    const auto index = scriptNode(*node).sup();
    return index < childNodes.size() ? childNodes[index] : nullptr;
}

CommonWrapper *CommonMsubsup::scriptChild() const {
    return supChild();
}

SceneNodes CommonMsubsup::emitScene(scene::Builder &builder,
                                    std::span<scene::NodeId const> parents) {
    if (auto embellished = toEmbellishedScene(builder, parents))
        return *embellished;

    auto containers = standardSceneNodes(builder, parents);
    auto &nodes = containers.nodes;
    const auto &content = containers.content;
    auto *base = baseChild();
    auto *sup = supChild();
    auto *sub = subChild();
    if (!base || !sup || !sub)
        throw std::logic_error("CommonMsubsup requires base, superscript and subscript wrappers");

    const double width = getBaseWidth();
    const double x = getAdjustedIc();
    const auto uvq = getUVQ();
    const auto baseNodes = base->emitScene(builder, content);
    if (content.empty())
        throw std::logic_error("CommonMsubsup has no Scene container for its scripts");
    const std::array<scene::NodeId, 1> scriptParent{content.back()};
    const auto supNodes = sup->emitScene(builder, scriptParent);
    const auto subNodes = sub->emitScene(builder, scriptParent);

    base->placeScene(builder, baseNodes, 0, 0);
    sub->placeScene(builder, subNodes, width + (baseIsChar ? 0 : x), uvq[1]);
    sup->placeScene(builder, supNodes, width + x, uvq[0]);
    return nodes;
}

std::array<double, 3> CommonMsubsup::getUVQ(const BBox *subboxValue, const BBox *supboxValue) {
    // Preserve Source default-argument evaluation order: fetch both child boxes
    // before checking the cache.
    const BBox subbox = subboxValue ? *subboxValue : subChild()->getOuterBBox();
    const BBox supbox = supboxValue ? *supboxValue : supChild()->getOuterBBox();
    const auto bbox = baseCore->getLineBBox(baseCore->breakCount());
    if (UVQ)
        return *UVQ;
    const double rule = parameter(*this, "rule_thickness");
    const double t = 3 * rule;
    const double subShift =
        length2em(attrLength(*this, u"subscriptshift"), parameter(*this, "sub2"));
    const double drop =
        baseCharZero(bbox->d * baseScale + parameter(*this, "sub_drop") * subbox.rscale);
    const double supDepth = supbox.d * supbox.rscale;
    const double subHeight = subbox.h * subbox.rscale;
    double u = getU();
    double v = std::max(drop, subShift);
    double q = u - supDepth - (subHeight - v);
    if (q < t) {
        v += t - q;
        const double p = .8 * parameter(*this, "x_height") - (u - supDepth);
        if (p > 0) {
            u += p;
            v -= p;
        }
    }
    u = std::max(length2em(attrLength(*this, u"superscriptshift"), u), u);
    v = std::max(length2em(attrLength(*this, u"subscriptshift"), v), v);
    q = u - supDepth - (subHeight - v);
    UVQ = std::array<double, 3>{u, -v, q};
    return *UVQ;
}

BBox CommonMsubsup::appendScripts(BBox &box) const {
    auto *sub = subChild();
    auto *sup = supChild();
    if (!sub || !sup || !baseCore)
        throw std::logic_error("msubsup requires base, subscript and superscript wrappers");
    const BBox subbox = sub->getOuterBBox();
    const BBox supbox = sup->getOuterBBox();
    const double width = getBaseWidth();
    const double x = getAdjustedIc();
    const auto uvq = const_cast<CommonMsubsup *>(this)->getUVQ();
    auto *base = baseChild();
    const auto line = base->getLineBBox(base->breakCount());
    const double y = box.d - line->d;
    box.combine(subbox, width + (baseIsChar ? 0 : x), uvq[1] - y);
    box.combine(supbox, width + x, uvq[0] - y);
    box.w += parameter(*this, "scriptspace");
    return box;
}
} // namespace folia::math::output::common
