// Typed host surface for MathJax HtmlNode.ts; no DOM implementation or layout.
export module folia.math.core.mml_tree.html_adaptor;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class HtmlAdaptor : public XmlAdaptor {
public:
    virtual Property getAttribute(XmlRef, std::string_view)=0;
    virtual XmlRef node(std::string_view, PropertyList, std::vector<XmlRef>)=0;
    virtual std::string outerHTML(XmlRef)=0;
    virtual std::u16string textContent(XmlRef)=0;
    virtual std::string kind(XmlRef)=0;
};
}
