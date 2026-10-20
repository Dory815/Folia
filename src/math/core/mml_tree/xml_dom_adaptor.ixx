export module folia.math.core.mml_tree.xml_dom_adaptor;
export import folia.math.core.mml_tree.html_adaptor;
export import folia.math.core.dom_adaptor;
import std;

export namespace folia::math::mml {
class XmlDomAdaptor final : public HtmlAdaptor {
    struct State;
    std::unique_ptr<State> state_;

    XmlDomAdaptor();

public:
    static std::shared_ptr<XmlDomAdaptor> create();
    ~XmlDomAdaptor() override;
    XmlDomAdaptor(XmlDomAdaptor const&) = delete;
    XmlDomAdaptor& operator=(XmlDomAdaptor const&) = delete;
    XmlDomAdaptor(XmlDomAdaptor&&) = delete;
    XmlDomAdaptor& operator=(XmlDomAdaptor&&) = delete;

    XmlRef fromDOM(folia::math::DOMNodeRef const& node) const;

    XmlRef clone(XmlRef object) const override;
    std::string serializeXML(XmlRef object) const override;
    std::optional<std::u16string> getAttribute(
        XmlRef object, std::string_view name) const override;
    XmlRef node(std::string_view kind,
        std::vector<std::pair<std::string, std::u16string>> attributes,
        std::vector<XmlRef> children) const override;
    std::string outerHTML(XmlRef object) const override;
    std::u16string textContent(XmlRef object) const override;
    std::string kind(XmlRef object) const override;

    folia::math::DOMAttributes allAttributes(XmlRef object) const;
    std::vector<XmlRef> childNodes(XmlRef object) const;
    std::u16string value(XmlRef object) const;
};
}
