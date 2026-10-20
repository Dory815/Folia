// MathJax 4.1.3 output/svg/Wrappers/mphantom.ts: SvgMphantom.toSVG.
// The Common wrapper keeps the established phantom measurement behavior;
// its Scene emission mirrors Source standardSvgNodes and omits hidden children.
// See src/math/LICENSE-MathJax.txt.
export module folia.math.output.common:mphantom;
export import :wrapper;
import std;

export namespace folia::math::output::common {
class CommonMphantom : public CommonWrapper {
  public:
    using CommonWrapper::CommonWrapper;
    SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents) override;
};
} // namespace folia::math::output::common
