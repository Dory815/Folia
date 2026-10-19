export module folia.math.core.dom_adaptor;
export import folia.math.core.dom_node;
import folia.math.util.property_host;
import std;

export namespace folia::math {
struct AttributeData { std::u16string name; Property value; };
struct PageBBox { double left=0, top=0, right=0, bottom=0; };
using DOMSearchInput = std::variant<std::u16string, DOMNodeRef, std::vector<DOMNodeRef>>;

class AbstractDOMAdaptor {
protected:
    virtual DOMNodeRef create(std::u16string_view kind, std::u16string_view ns = {}) = 0;
public:
    // The Source field starts as Undefined and may later be assigned Null or
    // an actual document. Preserve that tag at the generic DOM boundary.
    DOMNodeValue document = Missing{};
    Property canMeasureNodes = true;
    virtual ~AbstractDOMAdaptor() = default;
    virtual DOMNodeRef parse(std::u16string_view text, std::u16string_view format = {}) = 0;
    DOMNodeRef node(std::u16string_view kind, PropertyList def = {},
                     PropertyArray children = {}, std::u16string_view ns = {});
    virtual DOMNodeRef text(std::u16string_view text) = 0;
    // Typed reconstruction of an immutable HTML/XML comment snapshot. This
    // native protocol does not add a method to the Source DOMAdaptor class.
    virtual DOMNodeRef comment(std::u16string_view text) = 0;
    virtual void setProperty(DOMNodeRef node, std::string_view name, Property value) = 0;
    virtual Property getProperty(DOMNodeRef node, std::string_view name) const = 0;
    void setAttributes(DOMNodeRef node, const PropertyList& def);
    virtual DOMNodeValue head(DOMNodeValue doc = Missing{}) const = 0;
    virtual DOMNodeValue body(DOMNodeValue doc = Missing{}) const = 0;
    virtual DOMNodeValue root(DOMNodeValue doc = Missing{}) const = 0;
    virtual std::u16string doctype(DOMNodeValue doc = Missing{}) const = 0;
    virtual std::vector<DOMNodeRef> tags(DOMNodeRef node, std::u16string_view name, std::u16string_view ns = {}) const = 0;
    virtual std::vector<DOMNodeRef> getElements(const std::vector<DOMSearchInput>& selectors, DOMNodeRef doc) const = 0;
    virtual DOMNodeValue getElement(std::u16string_view selector, DOMNodeValue node = Missing{}) const = 0;
    virtual bool contains(DOMNodeRef container, DOMNodeRef node) const = 0;
    virtual DOMNodeValue parent(DOMNodeRef node) const = 0;
    virtual DOMNodeRef append(DOMNodeRef node, DOMNodeRef child) = 0;
    virtual void insert(DOMNodeRef child, DOMNodeRef sibling) = 0;
    virtual DOMNodeRef remove(DOMNodeRef child) = 0;
    virtual DOMNodeRef replace(DOMNodeRef node, DOMNodeRef oldNode);
    virtual DOMNodeRef clone(DOMNodeRef node, bool deep = true) const = 0;
    virtual DOMNodeRef split(DOMNodeRef text, double index) = 0;
    virtual DOMNodeValue next(DOMNodeRef node) const = 0;
    virtual DOMNodeValue previous(DOMNodeRef node) const = 0;
    virtual DOMNodeValue firstChild(DOMNodeRef node) const = 0;
    virtual DOMNodeValue lastChild(DOMNodeRef node) const = 0;
    virtual DOMNodeArray childNodes(DOMNodeRef node) const = 0;
    virtual DOMNodeValue childNode(DOMNodeRef node, double index) const;
    virtual std::u16string kind(DOMNodeRef node) const = 0;
    virtual std::u16string value(DOMNodeRef node) const = 0;
    virtual std::u16string textContent(DOMNodeRef node) const = 0;
    virtual std::u16string innerHTML(DOMNodeRef node) const = 0;
    virtual std::u16string outerHTML(DOMNodeRef node) const = 0;
    virtual std::u16string serializeXML(DOMNodeRef node) const = 0;
    virtual void setAttribute(DOMNodeRef node, std::u16string_view name, Property value, std::u16string_view ns = {}) = 0;
    virtual Property getAttribute(DOMNodeRef node, std::u16string_view name) const = 0;
    virtual void removeAttribute(DOMNodeRef node, std::u16string_view name) = 0;
    virtual bool hasAttribute(DOMNodeRef node, std::u16string_view name) const = 0;
    virtual std::vector<AttributeData> allAttributes(DOMNodeRef node) const = 0;
    virtual void addClass(DOMNodeRef node, std::u16string_view name) = 0;
    virtual void removeClass(DOMNodeRef node, std::u16string_view name) = 0;
    virtual bool hasClass(DOMNodeRef node, std::u16string_view name) const = 0;
    virtual std::vector<std::u16string> allClasses(DOMNodeRef node) const;
    virtual void setStyle(DOMNodeRef node, std::u16string_view name, std::u16string_view value) = 0;
    virtual std::u16string getStyle(DOMNodeRef node, std::u16string_view name) = 0;
    virtual Property allStyles(DOMNodeRef node) const = 0;
    virtual void insertRules(DOMNodeRef node, const std::vector<std::u16string>& rules) = 0;
    virtual std::u16string cssText(DOMNodeRef node) const;
    virtual double fontSize(DOMNodeRef node) const = 0;
    virtual std::u16string fontFamily(DOMNodeRef node) const = 0;
    virtual std::pair<double,double> nodeSize(DOMNodeRef node, double em=1, std::optional<bool> local={}) const = 0;
    virtual PageBBox nodeBBox(DOMNodeRef node) const = 0;
};
}
