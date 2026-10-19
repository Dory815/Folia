module folia.math.input.tex.node_factory;
import std;

namespace folia::math::tex {
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

void NodeFactory::notifyCreated(mml::NodeKind kind, mml::MmlNode& node) const {
    if (registration_) registration_->addNode(kind, node);
}

mml::MmlNode& NodeFactory::makeNode(
    mml::NodeKind kind,
    std::span<mml::MmlNode* const> children,
    mml::AttributeSet attributes) const {
    auto& node = mmlFactory_.make(kind, std::move(attributes), children);
    // Source NodeFactory.create registers only calls whose public request kind
    // is "node", and only after the creator has returned successfully.
    notifyCreated(kind, node);
    return node;
}

mml::MmlNode& NodeFactory::makeToken(
    mml::NodeKind kind,
    std::u16string_view text,
    mml::AttributeSet attributes) const {
    if (!isTokenKind(kind)) {
        throw std::invalid_argument("TeX token construction requires a token MML kind");
    }
    auto& textNode = makeText(text);
    mml::MmlNode* children[]{&textNode};
    return makeNode(kind, children, std::move(attributes));
}

mml::MmlNode& NodeFactory::makeText(std::u16string_view text) const {
    auto& node = mmlFactory_.make(mml::NodeKind::text);
    node.setText(std::u16string(text));
    return node;
}

mml::MmlNode& NodeFactory::makeError(std::u16string_view message) const {
    auto& text = makeText(message);
    mml::MmlNode* textChildren[]{&text};
    auto& token = makeNode(mml::NodeKind::mtext, textChildren);

    mml::AttributeSet errorAttributes;
    errorAttributes.set(u"data-mjx-error", std::u16string(message));
    mml::MmlNode* errorChildren[]{&token};
    return makeNode(mml::NodeKind::merror, errorChildren, std::move(errorAttributes));
}
}
