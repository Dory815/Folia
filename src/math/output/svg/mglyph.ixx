/*************************************************************
 * Native Source output/svg/Wrappers/mglyph.ts owner.
 *************************************************************/
export module folia.math.output.svg.mglyph;
export import folia.math.output.common;
import std;

export namespace folia::math::output::svg {
class SvgMglyph final : public common::CommonMglyph {
  public:
    using common::CommonMglyph::CommonMglyph;
    common::SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents) override;
};

common::CommonWrapperRegistration mglyphRegistration();
} // namespace folia::math::output::svg
