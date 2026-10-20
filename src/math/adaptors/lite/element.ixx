export module folia.math.adaptors.lite_adaptor:element;
export import folia.math.core.dom_adaptor;
export import folia.math.util.styles;
import std;

export namespace folia::math::adaptors::lite {
class LiteText;
class LiteComment;
class LiteElement;

using LiteNodeRef = folia::math::DOMNodeRef;
using LiteElementRef = std::shared_ptr<LiteElement>;

class LiteElement final : public folia::math::DOMNode {
    friend class LiteBase;
    std::u16string kind_;
    folia::math::DOMAttributes attributes_;
    std::vector<LiteNodeRef> children_;
    std::shared_ptr<styles::Styles> styles_;

    LiteElement(std::u16string kind, folia::math::DOMAttributes attributes,
                std::vector<LiteNodeRef> children);

public:
    static LiteElementRef create(std::u16string kind,
                                 folia::math::DOMAttributes attributes = {},
                                 std::vector<LiteNodeRef> children = {});
    std::u16string_view kind() const noexcept override { return kind_; }
    folia::math::DOMAttributes const& attributes() const noexcept { return attributes_; }
    std::vector<LiteNodeRef> const& children() const noexcept { return children_; }
    std::shared_ptr<styles::Styles> const& styles() const noexcept { return styles_; }
};
}
