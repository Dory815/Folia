export module folia.math.adaptors.lite_adaptor:parser;
export import folia.math.core.dom_adaptor;
export import :element;
export import :text;
export import :document;
import folia.math.util.entities;
import folia.math.util.regexp_host;
import std;

export namespace folia::math::adaptors::lite {
class LiteBase;
class LiteParser final {
    std::shared_ptr<entities::EntityModule> entities_;

    static bool selfClosingTag(std::u16string_view kind) noexcept;
    static bool pcdataTag(std::u16string_view kind) noexcept;
    static std::optional<std::u16string_view>
    namespaceFor(std::u16string_view kind) noexcept;

public:
    explicit LiteParser(std::shared_ptr<entities::EntityModule> entities = {});

    LiteDocumentRef parseFromString(std::u16string_view text,
        std::u16string_view format, LiteBase& adaptor);
    LiteTextRef addText(LiteBase& adaptor, LiteElementRef const& node,
                        std::u16string_view text);
    LiteCommentRef addComment(LiteBase& adaptor, LiteElementRef const& node,
                              std::u16string_view comment);
    LiteElementRef closeTag(LiteBase& adaptor, LiteElementRef node,
                            std::u16string_view tag);
    LiteElementRef openTag(LiteBase& adaptor, LiteElementRef node,
        std::u16string_view tag, std::deque<std::optional<std::u16string>>& parts);
    void addAttributes(LiteBase& adaptor, LiteElementRef const& node,
        std::span<std::optional<std::u16string> const> attributes);
    void handlePCDATA(LiteBase& adaptor, LiteElementRef const& node,
        std::u16string_view kind,
        std::deque<std::optional<std::u16string>>& parts);
    void checkDocument(LiteBase& adaptor, LiteDocumentRef const& document);
    LiteElementRef getOnlyChild(LiteBase& adaptor,
                                LiteElementRef const& body) const;
    folia::math::DOMAttributes allAttributes(LiteBase& adaptor,
        LiteElementRef const& node, bool xml = false) const;
    std::u16string serialize(LiteBase& adaptor, LiteElementRef const& node,
                             bool xml = false) const;
    std::u16string serializeInner(LiteBase& adaptor,
        LiteElementRef const& node, bool xml = false) const;
    std::u16string protectHTML(std::u16string_view text) const;
    std::u16string protectAttribute(std::u16string_view text,
                                    bool xml = false) const;
};
}
