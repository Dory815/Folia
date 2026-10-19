/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mtd.ts.
 *************************************************************/
export module folia.math.output.common:mtd;
export import :mtable;
import folia.math.core.mml_tree.mml_nodes.mtd;
import std;
export namespace folia::math::output::common {
class CommonMtd : public CommonWrapper {
public:
    using CommonWrapper::CommonWrapper;
    bool fixesPWidth() const override { return false; }
    void invalidateBBox(bool bubble=true) override;
    double getWrapWidth(std::size_t) override;
    std::string getChildAlign(std::size_t) const override;
};
}
