/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mfenced.ts.
 *************************************************************/
export module folia.math.output.common:mfenced;
export import :wrapper;
export import :mrow;
import folia.math.core.mml_tree.mml_nodes.mfenced;
import std;
export namespace folia::math::output::common {
class CommonMfenced : public CommonWrapper {
public:
    CommonMrow* mrow=nullptr;
    CommonMfenced(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    void initializeSource() override;
    void createMrow();
    void addMrowChildren();
    void addMo(mml::MmlNode*);
    std::size_t breakCount()override;
protected:
    void computeBBox(BBox&,bool recompute=false)override;
    std::shared_ptr<LineBBox> computeLineBBox(std::size_t)override;
};
}
