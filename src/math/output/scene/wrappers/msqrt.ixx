/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/msqrt.ts placement.
 *************************************************************/
export module folia.math.output.scene:msqrt;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MsqrtPlacement final : public Placement {
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                              std::span<const NodeRef>) override;
};
void registerMsqrtPlacement(WrapperFactory&);
std::vector<NodeRef> buildMsqrtContent(common::CommonMsqrt&,Builder&,
    WrapperFactory&,std::span<const NodeRef> parents);
}
