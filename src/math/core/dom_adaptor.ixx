export module folia.math.core.dom_adaptor;
export import folia.math.core.dom_node;
import folia.math.util.styles;
import std;

export namespace folia::math {
struct AttributeData {
    std::u16string name;
    std::u16string value;
};
using DOMAttributes = std::vector<AttributeData>;
using DOMSearchInput = std::variant<std::u16string, DOMNodeRef, std::vector<DOMNodeRef>>;

struct PageBBox {
    double left = 0;
    double top = 0;
    double right = 0;
    double bottom = 0;
};

class AbstractDOMAdaptor {
protected:
    virtual DOMNodeRef create(std::u16string_view kind, std::u16string_view ns = {}) = 0;

public:
    DOMDocumentRef document;
    bool canMeasureNodes = true;
    virtual ~AbstractDOMAdaptor() = default;

    virtual DOMDocumentRef parse(std::u16string_view text, std::u16string_view format = {}) = 0;
    DOMNodeRef node(std::u16string_view kind, std::span<AttributeData const> attributes = {},
                    std::span<DOMNodeRef const> children = {}, std::u16string_view ns = {});
    virtual DOMNodeRef text(std::u16string_view text) = 0;
    virtual DOMNodeRef comment(std::u16string_view text) = 0;
    void setAttributes(DOMNodeRef const& node, std::span<AttributeData const> attributes);

    virtual DOMNodeRef head(DOMDocumentRef doc = {}) const = 0;
    virtual DOMNodeRef body(DOMDocumentRef doc = {}) const = 0;
    virtual DOMNodeRef root(DOMDocumentRef doc = {}) const = 0;
    virtual std::u16string doctype(DOMDocumentRef doc = {}) const = 0;
    virtual std::vector<DOMNodeRef> tags(DOMNodeRef const& node, std::u16string_view name,
                                         std::u16string_view ns = {}) const = 0;
    virtual std::vector<DOMNodeRef> getElements(std::span<DOMSearchInput const> selectors,
                                                DOMDocumentRef const& doc) const = 0;
    virtual DOMNodeRef getElement(std::u16string_view selector, DOMDocumentRef doc = {}) const = 0;
    virtual DOMNodeRef getElement(std::u16string_view selector, DOMNodeRef const& node) const = 0;
    virtual bool contains(DOMNodeRef const& container, DOMNodeRef const& node) const = 0;
    DOMNodeRef parent(DOMNodeRef const& node) const noexcept;
    virtual DOMNodeRef append(DOMNodeRef const& parent, DOMNodeRef child) = 0;
    virtual void insert(DOMNodeRef child, DOMNodeRef const& sibling) = 0;
    virtual DOMNodeRef remove(DOMNodeRef const& child) = 0;
    virtual DOMNodeRef replace(DOMNodeRef replacement, DOMNodeRef const& oldNode);
    virtual DOMNodeRef clone(DOMNodeRef const& node, bool deep = true) const = 0;
    virtual DOMNodeRef split(DOMNodeRef const& text, double index) = 0;
    virtual DOMNodeRef next(DOMNodeRef const& node) const = 0;
    virtual DOMNodeRef previous(DOMNodeRef const& node) const = 0;
    virtual DOMNodeRef firstChild(DOMNodeRef const& node) const = 0;
    virtual DOMNodeRef lastChild(DOMNodeRef const& node) const = 0;
    virtual std::vector<DOMNodeRef> childNodes(DOMNodeRef const& node) const = 0;
    virtual DOMNodeRef childNode(DOMNodeRef const& node, double index) const;

    std::u16string_view kind(DOMNodeRef const& node) const noexcept;
    virtual std::u16string value(DOMNodeRef const& node) const = 0;
    virtual std::u16string textContent(DOMNodeRef const& node) const = 0;
    virtual std::u16string innerHTML(DOMNodeRef const& node) const = 0;
    virtual std::u16string outerHTML(DOMNodeRef const& node) const = 0;
    virtual std::u16string serializeXML(DOMNodeRef const& node) const = 0;

    virtual void setAttribute(DOMNodeRef const& node, std::u16string_view name,
                              std::u16string_view value, std::u16string_view ns = {}) = 0;
    virtual void setAttribute(DOMNodeRef const& node, std::u16string_view name,
                              double value, std::u16string_view ns = {});
    virtual std::optional<std::u16string> getAttribute(DOMNodeRef const& node,
                                                       std::u16string_view name) const = 0;
    virtual void removeAttribute(DOMNodeRef const& node, std::u16string_view name) = 0;
    virtual bool hasAttribute(DOMNodeRef const& node, std::u16string_view name) const = 0;
    virtual DOMAttributes allAttributes(DOMNodeRef const& node) const = 0;

    virtual void addClass(DOMNodeRef const& node, std::u16string_view name) = 0;
    virtual void removeClass(DOMNodeRef const& node, std::u16string_view name) = 0;
    virtual bool hasClass(DOMNodeRef const& node, std::u16string_view name) const = 0;
    virtual std::vector<std::u16string> allClasses(DOMNodeRef const& node) const;
    virtual void setStyle(DOMNodeRef const& node, std::u16string_view name,
                          std::u16string_view value) = 0;
    virtual std::u16string getStyle(DOMNodeRef const& node, std::u16string_view name) = 0;
    virtual std::optional<std::u16string> allStyles(DOMNodeRef const& node) const = 0;
    virtual void insertRules(DOMNodeRef const& node,
                             std::span<std::u16string const> rules) = 0;
    virtual std::u16string cssText(DOMNodeRef const& node) const;

    virtual double fontSize(DOMNodeRef const& node) const = 0;
    virtual std::u16string fontFamily(DOMNodeRef const& node) const = 0;
    virtual std::pair<double, double> nodeSize(DOMNodeRef const& node, double em = 1,
                                              std::optional<bool> local = {}) const = 0;
    virtual PageBBox nodeBBox(DOMNodeRef const& node) const = 0;
};
}
