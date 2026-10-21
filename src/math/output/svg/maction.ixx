/*************************************************************
 * Native owner for MathJax 4.1.3 output/svg/Wrappers/maction.ts.
 *************************************************************/
export module folia.math.output.svg.maction;
export import folia.math.output.common;
import std;

export namespace folia::math::output::svg {
class SvgMaction final : public common::CommonMaction {
  public:
    using common::CommonMaction::CommonMaction;
    common::SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents) override;
};

common::CommonWrapperRegistration mactionRegistration();
} // namespace folia::math::output::svg
