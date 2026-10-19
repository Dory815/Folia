/*************************************************************
 * MathJax output/svg/Wrappers/mpadded.ts placement.
 *************************************************************/
export module folia.math.output.scene:mpadded;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MpaddedPlacement final : public Placement {
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                               std::span<const NodeRef>) override;
};
void registerMpaddedPlacement(WrapperFactory&);
}
