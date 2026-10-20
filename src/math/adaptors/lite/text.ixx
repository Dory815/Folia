export module folia.math.adaptors.lite_adaptor:text;
export import folia.math.core.dom_adaptor;
import std;

export namespace folia::math::adaptors::lite {
class LiteElement;
class LiteText;
class LiteComment;

using LiteTextRef = std::shared_ptr<LiteText>;
using LiteCommentRef = std::shared_ptr<LiteComment>;

class LiteText : public folia::math::DOMNode {
    friend class LiteBase;
    std::u16string value_;

protected:
    explicit LiteText(std::u16string value);

public:
    static LiteTextRef create(std::u16string value = {});
    std::u16string_view kind() const noexcept override { return u"#text"; }
    std::u16string const& value() const noexcept { return value_; }
    void value(std::u16string text) { value_ = std::move(text); }
};

class LiteComment final : public LiteText {
    explicit LiteComment(std::u16string value);

public:
    static LiteCommentRef create(std::u16string rawComment);
    std::u16string_view kind() const noexcept override { return u"#comment"; }
};
}
