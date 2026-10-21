/*************************************************************
 * Native Source output/svg/Wrappers/merror.ts owner.
 *************************************************************/
export module folia.math.output.svg.merror;
export import folia.math.output.common;
import std;

export namespace folia::math::output::svg {
class SvgMerror final : public common::CommonWrapper {
  public:
    using common::CommonWrapper::CommonWrapper;
    common::SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents) override;
};

common::CommonWrapperRegistration merrorRegistration();
} // namespace folia::math::output::svg
