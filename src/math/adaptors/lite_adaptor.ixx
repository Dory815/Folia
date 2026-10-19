export module folia.math.adaptors.lite_adaptor;
export import folia.math.core.dom_adaptor;
export import folia.math.util.entities;
export import folia.math.adaptors.lite.element;
export import folia.math.adaptors.lite.text;
export import folia.math.adaptors.lite.document;
export import folia.math.adaptors.lite.list;
export import folia.math.adaptors.lite.window;
export import folia.math.adaptors.lite.parser;
export import folia.math.util.regexp_host;
import std;
export namespace folia::math::adaptors::lite {
using LiteSearchInput = std::variant<std::u16string, LiteNodeRef, std::vector<LiteNodeRef>>;

class LiteBase : public AbstractDOMAdaptor {
protected:
    std::shared_ptr<LiteNodeStore> store_;
    std::function<void()> serializeXmlObserver_;
    LiteNodeRef window_;
    std::shared_ptr<LiteParser> parser_;
    LiteNodeRef create(std::u16string_view kind, std::u16string_view ns = {}) override;
public:
    explicit LiteBase(std::shared_ptr<LiteNodeStore> store = {},
                      std::shared_ptr<entities::EntityModule> entities = {});
    LiteNodeRef window() const { return window_; }
    void window(LiteNodeRef value) { window_ = std::move(value); }
    std::shared_ptr<LiteParser> parser() const { return parser_; }
    void parser(std::shared_ptr<LiteParser> value) { parser_ = std::move(value); }
    LiteNodeRef parse(std::u16string_view text, std::u16string_view format = {}) override;
    LiteNodeRef text(std::u16string_view text) override;
    void setSerializeXmlObserver(std::function<void()> observer){
        serializeXmlObserver_=std::move(observer);
    }
    void setProperty(LiteNodeRef node, std::string_view name, Property value) override;
    Property getProperty(LiteNodeRef node, std::string_view name) const override;
    LiteNodeRef comment(std::u16string_view text) override;
    LiteNodeRef createDocument();
    LiteNodeValue head(LiteNodeValue doc = Missing{}) const override;
    LiteNodeValue body(LiteNodeValue doc = Missing{}) const override;
    LiteNodeValue root(LiteNodeValue doc = Missing{}) const override;
    std::u16string doctype(LiteNodeValue doc = Missing{}) const override;
    std::vector<LiteNodeRef> tags(LiteNodeRef node, std::u16string_view name, std::u16string_view ns = {}) const override;
    std::vector<LiteNodeRef> tags(LiteNodeRef node, std::u16string_view name, std::u16string_view ns, std::optional<std::size_t> stop) const;
    LiteNodeRef elementById(LiteNodeRef node, std::u16string_view id) const;
    std::vector<LiteNodeRef> elementsByClass(LiteNodeRef node, std::u16string_view name, std::optional<std::size_t> stop = {}) const;
    std::vector<LiteNodeRef> elementsByAttribute(LiteNodeRef node, std::u16string_view name, std::u16string_view value, std::optional<std::size_t> stop = {}) const;
    std::vector<LiteNodeRef> getElements(const std::vector<LiteSearchInput>& selectors, LiteNodeRef doc) const override;
    LiteNodeValue getElement(std::u16string_view selector, LiteNodeValue node = Missing{}) const override;
    bool contains(LiteNodeRef container, LiteNodeRef node) const override;
    LiteNodeValue parent(LiteNodeRef node) const override;
    std::ptrdiff_t childIndex(LiteNodeRef node) const;
    LiteNodeRef append(LiteNodeRef node, LiteNodeRef child) override;
    void insert(LiteNodeRef child, LiteNodeRef sibling) override;
    LiteNodeRef remove(LiteNodeRef child) override;
    LiteNodeRef replace(LiteNodeRef node, LiteNodeRef oldNode) override;
    LiteNodeRef clone(LiteNodeRef node, bool deep = true) const override;
    LiteNodeRef split(LiteNodeRef text, double index) override;
    LiteNodeValue next(LiteNodeRef node) const override;
    LiteNodeValue previous(LiteNodeRef node) const override;
    LiteNodeValue firstChild(LiteNodeRef node) const override;
    LiteNodeValue lastChild(LiteNodeRef node) const override;
    DOMNodeArray childNodes(LiteNodeRef node) const override;
    std::u16string kind(LiteNodeRef node) const override;
    std::u16string value(LiteNodeRef node) const override;
    std::u16string textContent(LiteNodeRef node) const override;
    std::u16string innerHTML(LiteNodeRef node) const override;
    std::u16string outerHTML(LiteNodeRef node) const override;
    std::u16string serializeXML(LiteNodeRef node) const override;
    void setAttribute(LiteNodeRef node, std::u16string_view name, Property value, std::u16string_view ns = {}) override;
    Property getAttribute(LiteNodeRef node, std::u16string_view name) const override;
    void removeAttribute(LiteNodeRef node, std::u16string_view name) override;
    bool hasAttribute(LiteNodeRef node, std::u16string_view name) const override;
    std::vector<AttributeData> allAttributes(LiteNodeRef node) const override;
    void addClass(LiteNodeRef node, std::u16string_view name) override;
    void removeClass(LiteNodeRef node, std::u16string_view name) override;
    bool hasClass(LiteNodeRef node, std::u16string_view name) const override;
    void setStyle(LiteNodeRef node, std::u16string_view name, std::u16string_view value) override;
    std::u16string getStyle(LiteNodeRef node, std::u16string_view name) override;
    Property allStyles(LiteNodeRef node) const override;
    void insertRules(LiteNodeRef node, const std::vector<std::u16string>& rules) override;
    double fontSize(LiteNodeRef node) const override;
    std::u16string fontFamily(LiteNodeRef node) const override;
    std::pair<double,double> nodeSize(LiteNodeRef node, double em=1, std::optional<bool> local={}) const override;
    PageBBox nodeBBox(LiteNodeRef node) const override;
};

class LiteAdaptor final : public LiteBase {
    PropertyList options_;
    bool badCSS_ = true, badSizes_ = true;
    std::function<std::u16string(std::u16string_view)> cjkStripper_;
    std::function<Property(LiteNodeRef)> sourceTextContent_;
    std::function<Property(std::string_view)> sourceOption_;
    std::function<double(std::string_view)> sourceNumberOption_;
public:
    LiteAdaptor(LiteNodeRef window = {}, PropertyList options = {}, std::shared_ptr<LiteNodeStore> store = {},
                std::shared_ptr<entities::EntityModule> entities = {});
    static PropertyList classOptions();
    static PropertyList mixinOptions();
    const PropertyList& options() const { return options_; }
    PropertyList& options() { return options_; }
    void options(PropertyList value) { options_ = std::move(value); }
    void badCSS(bool value) { badCSS_=value; }
    void badSizes(bool value) { badSizes_=value; }
    void cjkStripper(std::function<std::u16string(std::u16string_view)> value) { cjkStripper_=std::move(value); }
    void sourceTextContent(std::function<Property(LiteNodeRef)> value) { sourceTextContent_=std::move(value); }
    std::function<Property(LiteNodeRef)> exchangeSourceTextContent(
        std::function<Property(LiteNodeRef)> value){
        return std::exchange(sourceTextContent_,std::move(value));
    }
    void sourceOption(std::function<Property(std::string_view)> value) { sourceOption_=std::move(value); }
    void sourceNumberOption(std::function<double(std::string_view)> value) { sourceNumberOption_=std::move(value); }
    std::function<double(std::string_view)> exchangeSourceNumberOption(
        std::function<double(std::string_view)> value){
        return std::exchange(sourceNumberOption_,std::move(value));
    }
    static std::u16string cjkPatternSource();
    static std::u16string replaceCjk(std::u16string_view text, regexp_host::RegExp& pattern);
    double fontSize(LiteNodeRef node) const override;
    std::u16string fontFamily(LiteNodeRef node) const override;
    Property fontFamilyProperty() const;
    std::pair<double,double> nodeSize(LiteNodeRef node, double em=1, std::optional<bool> local={}) const override;
    PageBBox nodeBBox(LiteNodeRef node) const override;
};
}
