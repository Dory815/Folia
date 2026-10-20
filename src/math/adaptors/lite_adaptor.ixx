export module folia.math.adaptors.lite_adaptor;
export import folia.math.core.dom_adaptor;
export import folia.math.util.entities;
export import :element;
export import :text;
export import :document;
export import :window;
export import :list;
export import :parser;
export import folia.math.util.regexp_host;
import std;

export namespace folia::math::adaptors::lite {
using LiteSearchInput = folia::math::DOMSearchInput;

struct LiteAdaptorOptions {
    bool badCSS = true;
    bool badSizes = true;
    double fontSize = 16;
    double cjkCharWidth = 1;
    double unknownCharWidth = 0.6;
    double unknownCharHeight = 0.8;
    std::u16string fontFamily = u"Times";
};

class LiteBase : public folia::math::AbstractDOMAdaptor {
protected:
    LiteWindowRef window_;
    std::shared_ptr<LiteParser> parser_;
    folia::math::DOMNodeRef create(std::u16string_view kind,
                                  std::u16string_view ns = {}) override;

public:
    using AbstractDOMAdaptor::setAttribute;
    explicit LiteBase(LiteWindowRef window = {},
                      std::shared_ptr<entities::EntityModule> entities = {});
    LiteWindowRef window() const noexcept { return window_; }
    void window(LiteWindowRef value);
    std::shared_ptr<LiteParser> const& parser() const noexcept { return parser_; }
    folia::math::DOMDocumentRef parse(std::u16string_view text,
                                      std::u16string_view format = {}) override;
    folia::math::DOMNodeRef text(std::u16string_view text) override;
    folia::math::DOMNodeRef comment(std::u16string_view text) override;
    LiteDocumentRef createDocument();

    folia::math::DOMNodeRef head(folia::math::DOMDocumentRef doc = {}) const override;
    folia::math::DOMNodeRef body(folia::math::DOMDocumentRef doc = {}) const override;
    folia::math::DOMNodeRef root(folia::math::DOMDocumentRef doc = {}) const override;
    std::u16string doctype(folia::math::DOMDocumentRef doc = {}) const override;
    std::vector<folia::math::DOMNodeRef> tags(folia::math::DOMNodeRef const& node,
        std::u16string_view name, std::u16string_view ns = {}) const override;
    std::vector<folia::math::DOMNodeRef> tags(folia::math::DOMNodeRef const& node,
        std::u16string_view name, std::u16string_view ns,
        std::optional<std::size_t> stop) const;
    LiteElementRef elementById(folia::math::DOMNodeRef const& node,
                               std::u16string_view id) const;
    std::vector<LiteElementRef> elementsByClass(folia::math::DOMNodeRef const& node,
        std::u16string_view name, std::optional<std::size_t> stop = {}) const;
    std::vector<LiteElementRef> elementsByAttribute(folia::math::DOMNodeRef const& node,
        std::u16string_view name, std::u16string_view value,
        std::optional<std::size_t> stop = {}) const;
    std::vector<folia::math::DOMNodeRef> getElements(
        std::span<LiteSearchInput const> selectors,
        folia::math::DOMDocumentRef const& doc) const override;
    folia::math::DOMNodeRef getElement(std::u16string_view selector,
        folia::math::DOMDocumentRef doc = {}) const override;
    folia::math::DOMNodeRef getElement(std::u16string_view selector,
        folia::math::DOMNodeRef const& node) const override;
    bool contains(folia::math::DOMNodeRef const& container,
                  folia::math::DOMNodeRef const& node) const override;
    folia::math::DOMNodeRef append(folia::math::DOMNodeRef const& parent,
                                    folia::math::DOMNodeRef child) override;
    std::ptrdiff_t childIndex(folia::math::DOMNodeRef const& node) const;
    folia::math::DOMNodeRef replace(folia::math::DOMNodeRef replacement,
        folia::math::DOMNodeRef const& oldNode) override;
    void insert(folia::math::DOMNodeRef child,
                folia::math::DOMNodeRef const& sibling) override;
    folia::math::DOMNodeRef remove(folia::math::DOMNodeRef const& child) override;
    folia::math::DOMNodeRef clone(folia::math::DOMNodeRef const& node,
                                  bool deep = true) const override;
    folia::math::DOMNodeRef split(folia::math::DOMNodeRef const& text,
                                  double index) override;
    folia::math::DOMNodeRef next(folia::math::DOMNodeRef const& node) const override;
    folia::math::DOMNodeRef previous(folia::math::DOMNodeRef const& node) const override;
    folia::math::DOMNodeRef firstChild(folia::math::DOMNodeRef const& node) const override;
    folia::math::DOMNodeRef lastChild(folia::math::DOMNodeRef const& node) const override;
    std::vector<folia::math::DOMNodeRef> childNodes(
        folia::math::DOMNodeRef const& node) const override;
    std::u16string value(folia::math::DOMNodeRef const& node) const override;
    std::u16string textContent(folia::math::DOMNodeRef const& node) const override;
    std::u16string innerHTML(folia::math::DOMNodeRef const& node) const override;
    std::u16string outerHTML(folia::math::DOMNodeRef const& node) const override;
    std::u16string serializeXML(folia::math::DOMNodeRef const& node) const override;
    void setAttribute(folia::math::DOMNodeRef const& node,
        std::u16string_view name, std::u16string_view value,
        std::u16string_view ns = {}) override;
    void removeAttribute(folia::math::DOMNodeRef const& node,
                         std::u16string_view name) override;
    std::optional<std::u16string> getAttribute(folia::math::DOMNodeRef const& node,
        std::u16string_view name) const override;
    bool hasAttribute(folia::math::DOMNodeRef const& node,
                      std::u16string_view name) const override;
    folia::math::DOMAttributes allAttributes(
        folia::math::DOMNodeRef const& node) const override;
    void addClass(folia::math::DOMNodeRef const& node,
                  std::u16string_view name) override;
    void removeClass(folia::math::DOMNodeRef const& node,
                     std::u16string_view name) override;
    bool hasClass(folia::math::DOMNodeRef const& node,
                  std::u16string_view name) const override;
    void setStyle(folia::math::DOMNodeRef const& node,
        std::u16string_view name, std::u16string_view value) override;
    std::u16string getStyle(folia::math::DOMNodeRef const& node,
                            std::u16string_view name) override;
    std::optional<std::u16string> allStyles(
        folia::math::DOMNodeRef const& node) const override;
    void insertRules(folia::math::DOMNodeRef const& node,
                     std::span<std::u16string const> rules) override;
    double fontSize(folia::math::DOMNodeRef const& node) const override;
    std::u16string fontFamily(folia::math::DOMNodeRef const& node) const override;
    std::pair<double, double> nodeSize(folia::math::DOMNodeRef const& node,
        double em = 1, std::optional<bool> local = {}) const override;
    folia::math::PageBBox nodeBBox(
        folia::math::DOMNodeRef const& node) const override;
};

class LiteAdaptor final : public LiteBase {
    LiteAdaptorOptions options_;

public:
    using LiteBase::setAttribute;
    explicit LiteAdaptor(LiteAdaptorOptions options = {}, LiteWindowRef window = {},
        std::shared_ptr<entities::EntityModule> entities = {});
    LiteAdaptorOptions const& options() const noexcept { return options_; }
    void options(LiteAdaptorOptions value) { options_ = std::move(value); }
    static std::u16string cjkPatternSource();
    static std::u16string replaceCjk(std::u16string_view text,
                                     regexp_host::RegExp& pattern);
    double fontSize(folia::math::DOMNodeRef const& node) const override;
    std::u16string fontFamily(folia::math::DOMNodeRef const& node) const override;
    std::pair<double, double> nodeSize(folia::math::DOMNodeRef const& node,
        double em = 1, std::optional<bool> local = {}) const override;
    folia::math::PageBBox nodeBBox(
        folia::math::DOMNodeRef const& node) const override;
};
}
