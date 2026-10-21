// Typed host surface for MathJax HtmlNode.ts; no DOM ownership.
export module folia.math.core.mml_tree.html_adaptor;
export import folia.math.core.mml_tree.mml_document;
export import folia.math.core.dom_adaptor;
import std;

export namespace folia::math::mml {
class HtmlAdaptor : public XmlAdaptor {
  public:
    virtual folia::math::DOMNodeRef cloneDOM(XmlRef) const = 0;
    virtual std::optional<std::u16string> getAttribute(XmlRef, std::string_view) const = 0;
    virtual XmlRef node(std::string_view, std::vector<std::pair<std::string, std::u16string>>,
                        std::vector<XmlRef>) const = 0;
    virtual std::string outerHTML(XmlRef) const = 0;
    virtual std::u16string textContent(XmlRef) const = 0;
    virtual std::string kind(XmlRef) const = 0;
};
} // namespace folia::math::mml
