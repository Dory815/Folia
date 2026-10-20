module folia.math.adaptors.lite_adaptor;
import :document;
import :element;
import std;

namespace folia::math::adaptors::lite {
LiteDocument::LiteDocument(std::weak_ptr<folia::math::DOMWindow> defaultView)
    : defaultView_(std::move(defaultView)), type_() {
    head_ = LiteElement::create(u"head");
    body_ = LiteElement::create(u"body");
    root_ = LiteElement::create(u"html", {}, {head_, body_});
}

LiteDocumentRef LiteDocument::create(
    std::weak_ptr<folia::math::DOMWindow> defaultView) {
    return LiteDocumentRef(new LiteDocument(std::move(defaultView)));
}
}
