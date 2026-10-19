/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mtext.ts.
 *************************************************************/
export module folia.math.output.common:mtext;
export import :wrapper;
export import :linebreak_visitor;
import folia.math.core.mml_tree.mml_nodes.mtext;
import std;

export namespace folia::math::output::common {
class CommonMtext : public CommonWrapper {
    CommonWrapper* textNode_=nullptr;
    std::vector<std::u16string> splitText(std::u16string_view) const;
protected:
    void getVariant() override;
    std::shared_ptr<LineBBox> computeLineBBox(std::size_t) override;
public:
    std::vector<IndexData> breakPoints;
    CommonMtext(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    double textWidth(std::u16string_view);
    std::size_t breakCount() override;
    void setBreakAt(IndexData);
    void clearBreakPoints();
    double getBreakWidth(std::size_t);
};
}
