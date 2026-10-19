/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/maction.ts.
 *************************************************************/
export module folia.math.output.scene:maction;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MactionPlacement final:public SvgWrapperPlacement {
protected:
    void placeContent(common::CommonWrapper&,Builder&,WrapperFactory&,
                      std::span<const NodeRef>) override;
};
void registerMactionPlacement(WrapperFactory&);
}
