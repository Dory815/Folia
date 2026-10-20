export module folia.math.adaptors.lite_adaptor:document;
export import :element;
import std;

export namespace folia::math::adaptors::lite {
class LiteWindow;
class LiteParser;
class LiteDocument;
using LiteDocumentRef = std::shared_ptr<LiteDocument>;

class LiteDocument final : public folia::math::DOMDocument {
    friend class LiteBase;
    friend class LiteParser;
    LiteElementRef root_;
    LiteElementRef head_;
    LiteElementRef body_;
    std::weak_ptr<folia::math::DOMWindow> defaultView_;
    std::u16string type_;

    explicit LiteDocument(std::weak_ptr<folia::math::DOMWindow> defaultView);

public:
    static LiteDocumentRef create(std::weak_ptr<folia::math::DOMWindow> defaultView = {});
    LiteElementRef root() const noexcept { return root_; }
    LiteElementRef head() const noexcept { return head_; }
    LiteElementRef body() const noexcept { return body_; }
    folia::math::DOMWindowRef defaultView() const noexcept { return defaultView_.lock(); }
    void defaultView(std::weak_ptr<folia::math::DOMWindow> value) noexcept {
        defaultView_ = std::move(value);
    }
    std::u16string const& type() const noexcept { return type_; }
    void type(std::u16string value) { type_ = std::move(value); }
};
}
