module folia.math.adaptors.lite_adaptor;
import :element;
import std;

namespace folia::math::adaptors::lite {
LiteElement::LiteElement(std::u16string kind,
                         folia::math::DOMAttributes attributes,
                         std::vector<LiteNodeRef> children)
    : kind_(std::move(kind)), attributes_(std::move(attributes)),
      children_(std::move(children)) {}

LiteElementRef LiteElement::create(std::u16string kind,
                                   folia::math::DOMAttributes attributes,
                                   std::vector<LiteNodeRef> children) {
    auto element = LiteElementRef(new LiteElement(
        std::move(kind), std::move(attributes), std::move(children)));
    for (auto const& child : element->children_) {
        if (!child) throw std::invalid_argument("LiteElement child is null");
        child->setParent(element);
    }
    return element;
}
}
