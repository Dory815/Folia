/*************************************************************
 * Exact inherited SvgWrapper.toSVG profile for mi, mn, ms, mspace.
 * Each of these pinned classes inherits both toSVG and addChildren.
 *************************************************************/
export module folia.math.output.scene:inherited_svg_wrapper;
export import :wrapper;
import std;

export namespace folia::math::output::scene {
class InheritedSvgWrapperPlacement final : public SvgWrapperPlacement {
protected:
    void placeContent(common::CommonWrapper&,Builder&,WrapperFactory&,
                      std::span<const NodeRef> roots) override;
};

// This is an explicit Source class registration, not an unknown-kind fallback.
void registerInheritedSvgWrappers(WrapperFactory&);
}
