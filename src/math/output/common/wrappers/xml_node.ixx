// MathJax 4.1.3 output/common/Wrappers/XmlNode.ts.
export module folia.math.output.common:xml_node;
export import :wrapper;
import folia.math.core.mml_tree.mml_nodes.html_node;
import folia.math.core.mml_tree.html_adaptor;
import std;

export namespace folia::math::output::common {
class CommonXmlNode : public CommonWrapper {
public:
    double rscale=1;
    CommonXmlNode(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    void initializeSource() override;
    Property getHDW(DOMNodeRef,std::string_view use,
                    std::optional<std::string_view> force=std::nullopt);
    std::array<double,3> splitHDW(const Property&) const;
    Property getFontStyles() const;
    DOMNodeRef getHTML();
    // Backend-specific wrapping/measurement is supplied by the output
    // renderer, as in the Source abstract mixin.
    virtual DOMNodeRef addHDW(DOMNodeRef,PropertyList& styles)=0;
    BBox measureXmlNode(DOMNodeRef);
    void getStyles() override;
    void getScale() override;
    void getVariant() override;
protected:
    void computeBBox(BBox&,bool recompute=false) override;
};
}
