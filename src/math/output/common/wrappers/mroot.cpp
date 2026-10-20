/*************************************************************
 * Native CommonMroot algorithms from MathJax 4.1.3.
 *************************************************************/
module folia.math.output.common;
import :mroot;
import :wrapper;
import :mo;
import :output_jax;
import folia.math.util.bbox;
import std;

namespace folia::math::output::common {
namespace {
double selectedSurdSize(CommonMo const &surd) {
    if (!surd.size)
        return 0;
    if (std::holds_alternative<AssemblyStretchSize>(*surd.size))
        return -1;
    return static_cast<double>(std::get<FixedStretchSize>(*surd.size).index);
}

double sourceMax(double left, double right) {
    return std::isnan(left) || std::isnan(right) ? std::numeric_limits<double>::quiet_NaN()
                                                 : std::max(left, right);
}

CommonWrapper &requiredRoot(CommonMroot const &wrapper) {
    const auto root = wrapper.root();
    if (!root || *root >= wrapper.childNodes.size() || !wrapper.childNodes[*root])
        throw std::logic_error("mroot wrapper is missing its degree child");
    return *wrapper.childNodes[*root];
}
} // namespace

void CommonMroot::combineRootBBox(BBox &target, BBox const &surdBox, double height) const {
    const auto rootBox = requiredRoot(*this).getOuterBBox();
    const double y = getRootDimens(surdBox, height)[1];
    target.combine(rootBox, 0, y);
}

std::array<double, 3> CommonMroot::getRootDimens(BBox const &surdBox, double height) const {
    const auto rootBox = requiredRoot(*this).getOuterBBox();
    const double size = surd ? selectedSurdSize(*surd) : 0;
    const double offset = (size < 0 ? .5 : .6) * surdBox.w;
    const double ratio = offset / rootBox.rscale;
    const double width = sourceMax(rootBox.w, ratio);
    const double dx = sourceMax(0, width - rootBox.w);
    const double rootHeightValue = rootHeight(rootBox, surdBox, size, height);
    const double x = width * rootBox.rscale - offset;
    return {x, rootHeightValue, dx};
}

double CommonMroot::rootHeight(BBox const &rootBox, BBox const &surdBox, double size,
                               double height) const {
    const double total = surdBox.h + surdBox.d;
    const double baseline = (size < 0 ? 1.9 : .55 * total) - (total - height);
    const double extension = sourceMax(0, rootBox.d * rootBox.rscale);
    return baseline + extension;
}

double CommonMroot::rootWidth() const {
    const auto box = requiredRoot(*this).getOuterBBox();
    return .4 + box.w * box.rscale;
}

double CommonMroot::addRoot(RadicalGeometry &geometry, BBox const &surdBox, double height) {
    const auto dimensions = getRootDimens(surdBox, height);
    const auto rootBox = requiredRoot(*this).getOuterBBox();
    geometry.degree_origin = scene::Point{dimensions[2] * rootBox.rscale, dimensions[1]};
    return dimensions[0];
}

SceneNodes CommonMroot::emitScene(scene::Builder &builder, std::span<scene::NodeId const> parents) {
    if (auto embellished = toEmbellishedScene(builder, parents))
        return *embellished;
    auto containers = standardSceneNodes(builder, parents);
    auto &nodes = containers.nodes;
    auto const &content = containers.content;
    const auto geometry = layoutGeometry();
    const auto surdNodes = surd->emitScene(builder, content);
    surd->placeScene(builder, surdNodes, geometry.surd_origin.x, geometry.surd_origin.y);
    const auto degreeIndex = root();
    if (!degreeIndex || !geometry.degree_origin)
        throw std::logic_error("CommonMroot is missing its degree geometry");
    const auto degreeNodes = childNodes.at(*degreeIndex)->emitScene(builder, content);
    childNodes.at(*degreeIndex)
        ->placeScene(builder, degreeNodes, geometry.degree_origin->x, geometry.degree_origin->y);
    if (!nodes.empty() && nodes.front()) {
        const auto baseGroup = builder.add_group(*nodes.front());
        const std::array<scene::NodeId, 1> baseParent{baseGroup};
        const auto baseNodes = childNodes.at(base())->emitScene(builder, baseParent);
        childNodes.at(base())->placeScene(builder, baseNodes, geometry.base_origin.x,
                                          geometry.base_origin.y);
    }
    scene::Rule overbar;
    overbar.bounds = geometry.overbar;
    overbar.role = scene::ElementRole::radical_rule;
    if (!nodes.empty() && nodes.back())
        builder.add_primitive(*nodes.back(), overbar);
    return nodes;
}
} // namespace folia::math::output::common
