// MathJax 4.1.3 HtmlNode.ts; Apache-2.0. Root-owned typed declarations.
export module folia.math.core.mml_tree.mml_nodes.html_node;
export import folia.math.core.mml_tree.html_adaptor;
import std;
export namespace folia::math::mml {
class HtmlNode : public XMLNode, public TextContentBinding {
public:
    explicit HtmlNode(Factory& factory):XMLNode(factory) {
        TextContentBinding::textContent=[this] {return textContent();};
    }
    std::string kind()const override;
    virtual XmlRef getHTML()const;
    // Preserve the actual virtual setXML return, including another XML receiver.
    virtual XMLNode* setHTML(XmlRef, XmlAdaptor* adaptor=nullptr);
    virtual std::string getSerializedHTML()const;
    virtual std::u16string textContent()const;
    std::string toString()const override;
    void verifyTree(std::optional<PropertyList> options={})override;
};
}
