export module folia.math.adaptors.lite_adaptor:window;
export import :document;
import std;

export namespace folia::math::adaptors::lite {
class LiteWindow;
using LiteWindowRef = std::shared_ptr<LiteWindow>;

class LiteWindow final : public folia::math::DOMWindow {
    LiteDocumentRef document_;

    LiteWindow() = default;

public:
    static LiteWindowRef create();
    LiteDocumentRef document() const noexcept { return document_; }
};
}
