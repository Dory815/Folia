/*************************************************************
 * Source-mapped SVG Mfrac placement.
 *************************************************************/
export module folia.math.output.scene:mfrac;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MfracPlacement final : public Placement {
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                               std::span<const NodeRef>) override;
};
void registerMfracPlacement(WrapperFactory&);
}
