/*************************************************************
 * Native Common phantom emission from MathJax 4.1.3.
 * Source: output/svg/Wrappers/mphantom.ts, SvgMphantom.toSVG.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :mphantom;
import :wrapper;
import std;

namespace folia::math::output::common {
SceneNodes CommonMphantom::emitScene(scene::Builder &builder,
                                     std::span<scene::NodeId const> parents) {
    // Source standardizes the phantom wrapper but deliberately does not add
    // its hidden children to the output tree.
    return standardSceneNodes(builder, parents).nodes;
}
} // namespace folia::math::output::common
