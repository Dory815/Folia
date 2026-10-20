module folia.math.core.dom_adaptor;
import folia.math.util.js_number;
import std;

namespace folia::math {
namespace {
std::u16string widenAscii(std::string_view text) {
    std::u16string result;
    result.reserve(text.size());
    for (const unsigned char ch : text) result.push_back(static_cast<char16_t>(ch));
    return result;
}
}

DOMNodeRef AbstractDOMAdaptor::node(std::u16string_view kind,
                                    std::span<AttributeData const> attributes,
                                    std::span<DOMNodeRef const> children,
                                    std::u16string_view ns) {
    auto result = create(kind, ns);
    setAttributes(result, attributes);
    for (auto const& child : children) append(result, child);
    return result;
}

void AbstractDOMAdaptor::setAttributes(DOMNodeRef const& node,
                                       std::span<AttributeData const> attributes) {
    for (auto const& attribute : attributes)
        setAttribute(node, attribute.name, attribute.value);
}

DOMNodeRef AbstractDOMAdaptor::parent(DOMNodeRef const& node) const noexcept {
    return node ? node->parent() : DOMNodeRef{};
}

DOMNodeRef AbstractDOMAdaptor::replace(DOMNodeRef replacement,
                                       DOMNodeRef const& oldNode) {
    insert(std::move(replacement), oldNode);
    return remove(oldNode);
}

DOMNodeRef AbstractDOMAdaptor::childNode(DOMNodeRef const& node, double index) const {
    if (!std::isfinite(index) || index < 0 || std::floor(index) != index) return {};
    const auto children = childNodes(node);
    if (index >= static_cast<double>(children.size())) return {};
    return children[static_cast<std::size_t>(index)];
}

std::u16string_view AbstractDOMAdaptor::kind(DOMNodeRef const& node) const noexcept {
    return node ? node->kind() : std::u16string_view{};
}

std::vector<std::u16string>
AbstractDOMAdaptor::allClasses(DOMNodeRef const& node) const {
    auto attribute = getAttribute(node, u"class");
    if (!attribute || attribute->empty()) return {};
    auto classes = std::move(*attribute);
    for (std::size_t at = 0; (at = classes.find(u"  ", at)) != std::u16string::npos;)
        classes.erase(at, 1);
    if (!classes.empty() && classes.front() == u' ') classes.erase(classes.begin());
    if (!classes.empty() && classes.back() == u' ') classes.pop_back();

    std::vector<std::u16string> result;
    std::size_t start = 0;
    for (;;) {
        const auto end = classes.find(u' ', start);
        result.emplace_back(classes.substr(start, end == std::u16string::npos
                                                       ? end : end - start));
        if (end == std::u16string::npos) break;
        start = end + 1;
    }
    return result;
}

std::u16string AbstractDOMAdaptor::cssText(DOMNodeRef const& node) const {
    return kind(node) == u"style" ? textContent(node) : std::u16string{};
}

void AbstractDOMAdaptor::setAttribute(DOMNodeRef const& node,
                                      std::u16string_view name,
                                      double value,
                                      std::u16string_view ns) {
    setAttribute(node, name, widenAscii(js_number::toString(value)), ns);
}
}
