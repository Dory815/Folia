/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/mphantom.js placement.
 * SvgMphantom.toSVG calls standardSvgNodes only and deliberately does not
 * emit child wrappers, while the measurement tree remains Common-owned.
 *************************************************************/
export module folia.math.output.scene:mphantom;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MphantomPlacement final : public SvgWrapperPlacement {
protected:
    void placeContent(common::CommonWrapper&,Builder&,WrapperFactory&,
                      std::span<const NodeRef>) override;
};
void registerMphantomPlacement(WrapperFactory&);
}
