module folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.operator_class;
import std;

namespace folia::math::mml {
namespace {
bool isUnwrappableKind(MmlNode const &node) {
    return node.isKind(NodeKind::mrow) || node.kind() == NodeKind::tex_atom ||
           node.kind() == NodeKind::mstyle || node.kind() == NodeKind::mphantom;
}

bool truthy(AttributeValue const *value) {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return false;
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean;
    if (const auto *number = std::get_if<double>(value))
        return *number != 0.0 && !std::isnan(*number);
    if (const auto *text = std::get_if<std::u16string>(value))
        return !text->empty();
    return false;
}

bool isAccentChild(MmlNode *child, MmlNode const *target) {
    return child && child->isEmbellished() && child->coreMO() == target;
}
} // namespace

MmlNode *MmlMo::coreParent() {
    MmlNode *embellished = nullptr;
    MmlNode *current = this;
    while (current && current->isEmbellished() && current->coreMO() == this &&
           dynamic_cast<MmlMath *>(current) == nullptr) {
        embellished = current;
        current = current->parent();
    }
    return embellished ? embellished : this;
}

std::u16string MmlMo::coreText(MmlNode *parentNode) {
    if (!parentNode)
        return {};
    MmlNode *node = parentNode;
    if (node->isEmbellished()) {
        MmlNode *core = node->coreMO();
        if (auto *text = dynamic_cast<TextNode *>(core))
            return text->getText();
        if (auto *token = dynamic_cast<AbstractMmlTokenNode *>(core))
            return token->getText();
        throw std::logic_error("embellished core operator is not a text token");
    }

    while (node && (isUnwrappableKind(*node) || node->kind() == NodeKind::munderover)) {
        const auto children = node->children();
        if (children.empty())
            break;
        if (node->kind() != NodeKind::munderover && children.size() != 1)
            break;
        node = children.front();
    }
    if (!node || !node->isToken())
        return {};
    if (auto *token = dynamic_cast<AbstractMmlTokenNode *>(node))
        return token->getText();
    return {};
}

bool MmlMo::isAccent() {
    bool accent = false;
    MmlNode *const enclosing = coreParent()->parent();
    if (!enclosing)
        return accent;
    std::optional<std::u16string_view> key;
    if (enclosing->kind() == NodeKind::mover) {
        auto *mover = dynamic_cast<MmlMunderover *>(enclosing);
        if (!mover)
            throw std::logic_error("mover kind has incompatible family type");
        const auto children = enclosing->children();
        const auto index = mover->over();
        if (index < children.size() && children[index]) {
            MmlNode *const core = children[index]->coreMO();
            if (core && core->coreMO())
                key = u"accent";
        }
    } else if (enclosing->kind() == NodeKind::munder) {
        auto *munder = dynamic_cast<MmlMunderover *>(enclosing);
        if (!munder)
            throw std::logic_error("munder kind has incompatible family type");
        const auto children = enclosing->children();
        const auto index = munder->under();
        if (index < children.size() && children[index]) {
            MmlNode *const core = children[index]->coreMO();
            if (core && core->coreMO())
                key = u"accentunder";
        }
    } else if (enclosing->kind() == NodeKind::munderover) {
        auto *multi = dynamic_cast<MmlMunderover *>(enclosing);
        if (!multi)
            throw std::logic_error("munderover kind has incompatible family type");
        const auto children = enclosing->children();
        const auto coreOfCore = [&](std::size_t index) -> MmlNode * {
            if (index >= children.size() || !children[index])
                return nullptr;
            MmlNode *const core = children[index]->coreMO();
            return core ? core->coreMO() : nullptr;
        };
        if (coreOfCore(multi->over()))
            key = u"accent";
        else if (coreOfCore(multi->under()))
            key = u"accentunder";
    }
    if (!key)
        return accent;
    if (enclosing->attributes().own(*key))
        return accent;
    return truthy(attributes().lookup(u"accent"));
}

MmlNode *MmlMo::setTeXclass(MmlNode *previous) {
    const auto form = attributes().lookup(u"form");
    const auto fence = attributes().lookup(u"fence");
    if (!properties().tex.explicit_tex_class && hasSpacingAttributes())
        return nullptr;
    if (truthy(fence) && texClass() == TexClass::rel) {
        if (form && std::get_if<std::u16string>(form) &&
            std::get<std::u16string>(*form) == u"prefix")
            setTexClass(TexClass::open);
        if (form && std::get_if<std::u16string>(form) &&
            std::get<std::u16string>(*form) == u"postfix")
            setTexClass(TexClass::close);
    }
    return adjustTeXclass(previous);
}

MmlNode *MmlMo::adjustTeXclass(MmlNode *previous) {
    return adjustOperatorTeXclass(*this, previous);
}
} // namespace folia::math::mml
