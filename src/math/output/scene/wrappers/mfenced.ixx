/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/mfenced.ts placement.
 *************************************************************/
export module folia.math.output.scene:mfenced;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MfencedPlacement final : public SvgWrapperPlacement {
protected:
    void placeContent(common::CommonWrapper&,Builder&,WrapperFactory&,
                      std::span<const NodeRef>) override;
};
void registerMfencedPlacement(WrapperFactory&);
}
