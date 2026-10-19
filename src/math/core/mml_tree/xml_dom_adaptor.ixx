// XmlAdaptor/HtmlAdaptor projection over one canonical output DOM store.
// XmlObject is only a retained handle to the same DOMNodeRef; this module
// never serializes and reparses a second tree.
export module folia.math.core.mml_tree.xml_dom_adaptor;
export import folia.math.core.mml_tree.html_adaptor;
export import folia.math.core.dom_adaptor;
import std;

export namespace folia::math::mml {
class XmlDomAdaptor final : public HtmlAdaptor,
                            public std::enable_shared_from_this<XmlDomAdaptor> {
    struct State;
    std::shared_ptr<State> state_;
    explicit XmlDomAdaptor(std::shared_ptr<folia::math::AbstractDOMAdaptor>);
    XmlRef wrap(folia::math::DOMNodeRef);
public:
    static std::shared_ptr<XmlDomAdaptor> create(
        std::shared_ptr<folia::math::AbstractDOMAdaptor>);
    ~XmlDomAdaptor() override;
    XmlDomAdaptor(const XmlDomAdaptor&)=delete;
    XmlDomAdaptor& operator=(const XmlDomAdaptor&)=delete;

    folia::math::DOMNodeRef resolve(XmlRef) const;
    // Canonical bridge entry for an existing node in the output store.
    XmlRef fromDOM(folia::math::DOMNodeRef);
    // Returns the same live Property view used by the DOM node's Context
    // projection; callers may pass it to ContextPropertyBindings::to().
    Property nodeProperty(XmlRef) const;
    std::shared_ptr<folia::math::AbstractDOMAdaptor> domAdaptor() const;

    XmlRef clone(XmlRef) override;
    std::string serializeXML(XmlRef) override;
    Property getAttribute(XmlRef,std::string_view) override;
    XmlRef node(std::string_view,PropertyList,std::vector<XmlRef>) override;
    std::string outerHTML(XmlRef) override;
    std::u16string textContent(XmlRef) override;
    std::string kind(XmlRef) override;
};
}
