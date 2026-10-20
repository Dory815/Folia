module folia.math.input.tex.node_factory;
import std;

namespace folia::math::tex {
void NodeCreators::merge(NodeCreators const &layer) {
    auto merge = [](auto &destination, auto const &source) {
        if (!source)
            return;
        if (!*source)
            throw std::invalid_argument("A configured node creator must be callable");
        destination = source;
    };
    merge(node, layer.node);
    merge(token, layer.token);
    merge(text, layer.text);
    merge(error, layer.error);
}
NodeFactory::NodeFactory(mml::MmlFactory &factory, NodeKindRegistration *registration) noexcept
    : mmlFactory_(&factory), registration_(registration),
      creators_{
          .node = createNode, .token = createToken, .text = createText, .error = createError} {}
void NodeFactory::setConfiguration(NodeFactoryConfiguration &owner) noexcept {
    configuration_ = &owner;
    registration_ = &owner;
}
NodeFactoryConfiguration &NodeFactory::configuration() const {
    if (!configuration_)
        throw std::logic_error("NodeFactory has no active ParseOptions configuration");
    return *configuration_;
}
void NodeFactory::setCreators(NodeCreators const &layer) {
    creators_.merge(layer);
}

bool NodeFactory::isTokenKind(mml::NodeKind kind) noexcept {
    switch (kind) {
    case mml::NodeKind::mi:
    case mml::NodeKind::mn:
    case mml::NodeKind::mo:
    case mml::NodeKind::mtext:
    case mml::NodeKind::mspace:
    case mml::NodeKind::ms:
        return true;
    default:
        return false;
    }
}

void NodeFactory::notifyCreated(mml::NodeKind kind, mml::MmlNode &node) const {
    if (registration_)
        registration_->addNode(kind, node);
}

mml::MmlNode &NodeFactory::createNode(NodeFactory &factory, mml::NodeKind kind,
                                      std::span<mml::MmlNode *const> children,
                                      NodeDefinition definition, mml::MmlNode *text) {
    auto &node = factory.mmlFactory().make(kind, {}, children);
    if (text)
        node.appendChild(*text);
    NodeUtil::setProperties(node, std::move(definition));
    return node;
}

mml::MmlNode &NodeFactory::createToken(NodeFactory &factory, mml::NodeKind kind,
                                       NodeDefinition definition,
                                       std::optional<std::u16string_view> text) {
    auto *textNode = factory.makeText(text);
    return factory.makeNode(kind, {}, std::move(definition), textNode);
}
mml::MmlNode *NodeFactory::createText(NodeFactory &factory,
                                      std::optional<std::u16string_view> text) {
    if (!text)
        return nullptr;
    auto &node = factory.mmlFactory().make(mml::NodeKind::text);
    node.setText(std::u16string(*text));
    return &node;
}
mml::MmlNode &NodeFactory::createError(NodeFactory &factory, std::u16string_view message) {
    auto *text = factory.makeText(message);
    auto &token = factory.makeNode(mml::NodeKind::mtext, {}, {}, text);
    mml::AttributeSet attributes;
    attributes.set(u"data-mjx-error", std::u16string(message));
    mml::MmlNode *children[]{&token};
    return factory.makeNode(mml::NodeKind::merror, children, std::move(attributes));
}

mml::MmlNode &NodeFactory::makeNode(mml::NodeKind kind, std::span<mml::MmlNode *const> children,
                                    NodeDefinition definition, mml::MmlNode *text) {
    auto callback = creators_.node.value();
    auto &node = callback(*this, kind, children, std::move(definition), text);
    // Register only the public node request, after the creator returned.
    notifyCreated(kind, node);
    return node;
}
mml::MmlNode &NodeFactory::makeToken(mml::NodeKind kind, std::optional<std::u16string_view> text,
                                     NodeDefinition definition) {
    if (!isTokenKind(kind)) {
        throw std::invalid_argument("TeX token construction requires a token MML kind");
    }
    auto callback = creators_.token.value();
    return callback(*this, kind, std::move(definition), text);
}

mml::MmlNode *NodeFactory::makeText(std::optional<std::u16string_view> text) {
    auto callback = creators_.text.value();
    return callback(*this, text);
}

mml::MmlNode &NodeFactory::makeError(std::u16string_view message) {
    auto callback = creators_.error.value();
    return callback(*this, message);
}
} // namespace folia::math::tex
