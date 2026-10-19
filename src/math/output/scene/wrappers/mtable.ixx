/*************************************************************
 * MathJax output/svg/Wrappers/mtable.ts.
 *************************************************************/
export module folia.math.output.scene:mtable;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MtablePlacement final : public Placement {
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                               std::span<const NodeRef>) override;
};
void registerMtablePlacement(WrapperFactory&);
}
