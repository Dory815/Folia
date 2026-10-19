/*************************************************************
 * MathJax output/svg/Wrappers/mtr.ts and mlabeledtr.ts.
 *************************************************************/
export module folia.math.output.scene:mtr;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MtrPlacement final : public Placement {
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                               std::span<const NodeRef>) override;
    // SvgMlabeledtr.toSVG renders its leading label after the row body, into
    // the parent table's labels group. The table later positions that same
    // rendered child; it must not construct a second label wrapper.
    static NodeRef buildLabel(common::CommonMtr&,Builder&,WrapperFactory&,
                              const NodeRef& labels);
};
void registerMtrPlacement(WrapperFactory&);
}
