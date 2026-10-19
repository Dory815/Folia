/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/mrow.ts placement.
 *************************************************************/
export module folia.math.output.scene:mrow;
export import :wrapper;
export import :svg_paint;
import std;

export namespace folia::math::output::scene {
class MrowPlacement final : public Placement {
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                               std::span<const NodeRef> parents) override;
};
void registerMrowSvgWrapper(WrapperFactory&);
}
