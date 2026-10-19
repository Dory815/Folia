/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mglyph.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common:mglyph;
export import :wrapper;
export import folia.math.core.mml_tree.mml_nodes.mglyph;
import folia.math.core.mml_tree.mml_node;

export namespace folia::math::output::common {
class CommonMglyph : public CommonWrapper {
    CommonWrapper* charWrapper_=nullptr;
    void getParameters();
public:
    double width=0;
    double height=0;
    double valign=0;
    CommonMglyph(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    void initializeSource() override;
    CommonWrapper* charWrapper() const noexcept { return charWrapper_; }
protected:
    void computeBBox(BBox&,bool recompute=false) override;
};
}
