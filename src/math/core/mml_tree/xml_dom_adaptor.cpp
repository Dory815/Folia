module folia.math.core.mml_tree.xml_dom_adaptor;
import folia.math.adaptors.lite_adaptor;
import folia.math.util.utf16;
import std;

namespace folia::math::mml {
namespace {
using folia::math::DOMNodeRef;
using folia::math::adaptors::lite::LiteBase;
using folia::math::adaptors::lite::LiteComment;
using folia::math::adaptors::lite::LiteElement;
using folia::math::adaptors::lite::LiteText;

class DomXmlObject final : public XmlObject {
public:
    explicit DomXmlObject(DOMNodeRef value) : node(std::move(value)) {}
    DOMNodeRef node;
};

std::shared_ptr<DomXmlObject const> payload(XmlRef const& object) {
    if (!object) throw std::invalid_argument("XML payload is null");
    auto result = std::dynamic_pointer_cast<DomXmlObject const>(object);
    if (!result) throw std::invalid_argument("XML payload has an unsupported owner");
    return result;
}

void requireNode(DOMNodeRef const& node) {
    if (!node) throw std::invalid_argument("Lite node is null");
}

std::shared_ptr<LiteElement const> element(DOMNodeRef const& node) {
    auto result = std::dynamic_pointer_cast<LiteElement const>(node);
    if (!result) throw std::invalid_argument("XML payload is not a LiteElement");
    return result;
}

DOMNodeRef cloneLiteNode(LiteBase& lite, DOMNodeRef const& node) {
    requireNode(node);
    if (std::dynamic_pointer_cast<LiteElement const>(node))
        return lite.clone(node, true);
    if (auto comment = std::dynamic_pointer_cast<LiteComment const>(node))
        return LiteComment::create(comment->value());
    if (auto text = std::dynamic_pointer_cast<LiteText const>(node))
        return LiteText::create(text->value());
    throw std::invalid_argument("Unsupported foreign HTML node kind");
}

XmlRef wrapFrozen(DOMNodeRef node) {
    requireNode(node);
    return std::make_shared<DomXmlObject>(std::move(node));
}

std::u16string decode(std::string_view text) {
    return folia::math::utf16::decode(text).text;
}
}

struct XmlDomAdaptor::State {
    std::unique_ptr<LiteBase> lite = std::make_unique<LiteBase>();
};

XmlDomAdaptor::XmlDomAdaptor() : state_(std::make_unique<State>()) {}

XmlDomAdaptor::~XmlDomAdaptor() = default;

std::shared_ptr<XmlDomAdaptor> XmlDomAdaptor::create() {
    return std::shared_ptr<XmlDomAdaptor>(new XmlDomAdaptor());
}

XmlRef XmlDomAdaptor::fromDOM(DOMNodeRef const& node) const {
    return wrapFrozen(cloneLiteNode(*state_->lite, node));
}

XmlRef XmlDomAdaptor::clone(XmlRef object) const {
    return fromDOM(payload(object)->node);
}

folia::math::DOMNodeRef XmlDomAdaptor::cloneDOM(XmlRef object) const {
    return cloneLiteNode(*state_->lite, payload(object)->node);
}

std::string XmlDomAdaptor::serializeXML(XmlRef object) const {
    auto const& node = payload(object)->node;
    (void)element(node);
    return folia::math::utf16::encode(state_->lite->serializeXML(node));
}

std::optional<std::u16string> XmlDomAdaptor::getAttribute(
    XmlRef object, std::string_view name) const {
    auto const& node = payload(object)->node;
    (void)element(node);
    return state_->lite->getAttribute(node, decode(name));
}

XmlRef XmlDomAdaptor::node(std::string_view kind,
    std::vector<std::pair<std::string, std::u16string>> attributes,
    std::vector<XmlRef> children) const {
    folia::math::DOMAttributes ownedAttributes;
    ownedAttributes.reserve(attributes.size());
    for (auto& [name, value] : attributes)
        ownedAttributes.push_back({decode(name), std::move(value)});

    std::vector<DOMNodeRef> ownedChildren;
    ownedChildren.reserve(children.size());
    for (auto const& child : children)
        ownedChildren.push_back(cloneLiteNode(*state_->lite, payload(child)->node));

    auto result = state_->lite->node(decode(kind), ownedAttributes, ownedChildren);
    return wrapFrozen(std::move(result));
}

std::string XmlDomAdaptor::outerHTML(XmlRef object) const {
    auto const& node = payload(object)->node;
    (void)element(node);
    return folia::math::utf16::encode(state_->lite->outerHTML(node));
}

std::u16string XmlDomAdaptor::textContent(XmlRef object) const {
    auto const& node = payload(object)->node;
    (void)element(node);
    return state_->lite->textContent(node);
}

std::string XmlDomAdaptor::kind(XmlRef object) const {
    auto const& node = payload(object)->node;
    return folia::math::utf16::encode(node->kind());
}

folia::math::DOMAttributes XmlDomAdaptor::allAttributes(XmlRef object) const {
    auto const& node = payload(object)->node;
    (void)element(node);
    return state_->lite->allAttributes(node);
}

std::vector<XmlRef> XmlDomAdaptor::childNodes(XmlRef object) const {
    auto const& node = payload(object)->node;
    (void)element(node);
    auto children = state_->lite->childNodes(node);
    std::vector<XmlRef> result;
    result.reserve(children.size());
    for (auto& child : children) result.push_back(wrapFrozen(std::move(child)));
    return result;
}

std::u16string XmlDomAdaptor::value(XmlRef object) const {
    auto const& node = payload(object)->node;
    if (auto text = std::dynamic_pointer_cast<LiteText const>(node))
        return text->value();
    if (std::dynamic_pointer_cast<LiteElement const>(node)) return {};
    throw std::invalid_argument("XML payload is not a Lite node");
}
}
