/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/TextNode.ts.
 * Text-node measurement mixin behavior for the native wrapper graph.
 *************************************************************/
export module folia.math.output.common:text_node;
export import :wrapper;
import folia.math.core.mml_tree.mml_node;
import std;

export namespace folia::math::output::common {
class CommonTextNode : public CommonWrapper {
public:
    using CommonWrapper::CommonWrapper;
    std::shared_ptr<std::vector<std::uint32_t>> remappedText(
        std::u16string_view text,std::string_view variant) const;
protected:
    void computeBBox(BBox&,bool recompute=false) override;
    void getStyles() override {}
    void getVariant() override {}
    void getScale() override {}
    void getSpace() override {}
private:
    std::u16string addUtextBBox(BBox&,std::u16string_view,std::string_view);
    static void updateBBox(BBox&,double h,double d,double w);
};
}
