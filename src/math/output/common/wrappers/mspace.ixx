/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mspace.ts.
 * Source-aligned common mspace wrapper.
 *************************************************************/
export module folia.math.output.common:mspace;
export import :wrapper;
import folia.math.core.mml_tree.mml_nodes.mspace;
import std;

export namespace folia::math::output::common {
class CommonMspace : public CommonWrapper {
public:
    std::string breakStyle;
    CommonMspace(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    void initializeSource() override;
    bool canBreak() const;
    std::size_t breakCount() override;
    void setBreakStyle(std::string_view linebreak={});
protected:
    void computeBBox(BBox&,bool recompute=false) override;
    std::shared_ptr<LineBBox> computeLineBBox(std::size_t) override;
};
}
