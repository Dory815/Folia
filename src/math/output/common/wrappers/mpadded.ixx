/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mpadded.ts.
 *************************************************************/
export module folia.math.output.common:mpadded;
export import :wrapper;
import folia.math.core.mml_tree.mml_nodes.mpadded;
import std;

export namespace folia::math::output::common {
class CommonMpadded : public CommonWrapper {
public:
    using CommonWrapper::CommonWrapper;
    std::array<double,9> getDimens();
    double containerWidth() const;
    double dimen(const Property&,const BBox&,std::string_view direction={},std::optional<double> minimum={}) const;
    void setBBoxDimens(BBox&);
    std::string getChildAlign(std::size_t) const override;
    double getWrapWidth(std::size_t) override;
protected:
    void computeBBox(BBox&,bool recompute=false) override;
};
}
