/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/math.ts placement.
 *************************************************************/
export module folia.math.output.scene:math;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MathPlacement final : public SvgWrapperPlacement {
protected:
    void placeContent(common::CommonWrapper&,Builder&,WrapperFactory&,
                      std::span<const NodeRef>) override;
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                               std::span<const NodeRef>) override;
};
void registerMathPlacement(WrapperFactory&);
}
