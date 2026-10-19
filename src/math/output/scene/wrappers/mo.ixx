/*************************************************************
 * Source-mapped SVG Mo glyph and delimiter placement.
 *************************************************************/
export module folia.math.output.scene:mo;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MoPlacement final : public Placement {
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                               std::span<const NodeRef>) override;
};
void registerMoPlacement(WrapperFactory&);
}
