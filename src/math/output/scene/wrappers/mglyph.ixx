/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/mglyph.ts placement.
 *************************************************************/
export module folia.math.output.scene:mglyph;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MglyphPlacement final : public Placement {
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                               std::span<const NodeRef>) override;
};
void registerMglyphPlacement(WrapperFactory&);
}
