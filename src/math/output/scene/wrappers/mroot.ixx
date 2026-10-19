/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/mroot.ts placement.
 *************************************************************/
export module folia.math.output.scene:mroot;
export import :msqrt;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MrootPlacement final : public Placement {
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                              std::span<const NodeRef>) override;
};
void registerMrootPlacement(WrapperFactory&);
}
