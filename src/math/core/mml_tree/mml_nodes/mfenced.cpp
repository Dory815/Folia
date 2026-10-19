module folia.math.core.mml_tree.mml_nodes.mfenced;
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
namespace {
std::u16string requiredString(const AttributeSet &attributes, std::u16string_view name) {
    const auto *value = attributes.lookup(name);
    if (const auto *text = value ? std::get_if<std::u16string>(value) : nullptr)
        return *text;
    throw std::invalid_argument("mfenced delimiter attributes must be strings");
}

void removeWhitespace(std::u16string &value) {
    std::erase_if(value, [](char16_t character) {
        return character == u' ' || character == u'\t' || character == u'\n' || character == u'\r';
    });
}
} // namespace

MmlMfenced::MmlMfenced(MmlDocument &document) : AbstractMmlNode(document, NodeKind::mfenced) {}

AttributeSet MmlMfenced::defaults() {
    return nodeDefaultAttributes(NodeKind::mfenced);
}

NodeKind MmlMfenced::kind() const noexcept {
    return NodeKind::mfenced;
}

void MmlMfenced::copyNativeStateTo(MmlNode &destination, MmlClone &clone) const {
    auto *fenced = dynamic_cast<MmlMfenced *>(&destination);
    if (!fenced) {
        throw std::logic_error("mfenced clone destination has the wrong dynamic type");
    }

    const auto remap = [&](MmlNode *source) -> MmlNode * {
        if (!source)
            return nullptr;

        MmlNode &target = clone.clone(*source);
        if (target.parent() == fenced)
            return &target;
        if (target.parent()) {
            throw std::invalid_argument(
                "mfenced cloned auxiliary already has a different parent");
        }
        clone.document().attachAuxiliary(*fenced, target);
        return &target;
    };

    fenced->open = remap(open);
    fenced->close = remap(close);
    fenced->separators.clear();
    fenced->separators.reserve(separators.size());
    for (MmlNode *separator : separators)
        fenced->separators.push_back(remap(separator));
}

MmlNode &MmlMfenced::fakeNode(std::u16string character, AttributeSet nodeAttributes,
                              std::optional<TexClass> texClassValue) {
    auto &text = document().factory().make(NodeKind::text);
    text.setText(std::move(character));
    const std::array<MmlNode *, 1> textChildren{&text};
    auto &node = document().factory().make(NodeKind::mo, std::move(nodeAttributes), textChildren);
    node.setTexClass(texClassValue);
    document().attachAuxiliary(*this, node);
    return node;
}

void MmlMfenced::addFakeNodes() {
    auto openText = requiredString(attributes(), u"open");
    auto closeText = requiredString(attributes(), u"close");
    auto separatorText = requiredString(attributes(), u"separators");
    removeWhitespace(openText);
    removeWhitespace(closeText);
    removeWhitespace(separatorText);

    if (!openText.empty()) {
        AttributeSet values;
        values.set(u"fence", true);
        values.set(u"form", std::u16string{u"prefix"});
        open = &fakeNode(std::move(openText), std::move(values), TexClass::open);
    }

    if (!separatorText.empty()) {
        while (separatorText.size() < children().size() - (children().empty() ? 0 : 1)) {
            separatorText.push_back(separatorText.back());
        }
        std::size_t index = 0;
        for (std::size_t childIndex = 1; childIndex < children().size(); ++childIndex) {
            if (!children()[childIndex])
                continue;
            std::u16string value;
            if (index < separatorText.size())
                value.push_back(separatorText[index]);
            ++index;
            separators.push_back(&fakeNode(std::move(value)));
        }
    }

    if (!closeText.empty()) {
        AttributeSet values;
        values.set(u"fence", true);
        values.set(u"form", std::u16string{u"postfix"});
        close = &fakeNode(std::move(closeText), std::move(values), TexClass::close);
    }
}

void MmlMfenced::setChildInheritedAttributes(AttributeList const &inherited, bool display,
                                             double level, bool prime) {
    addFakeNodes();
    if (open)
        open->setInheritedAttributes(inherited, display, level, prime);
    if (close)
        close->setInheritedAttributes(inherited, display, level, prime);
    for (auto *separator : separators) {
        if (separator)
            separator->setInheritedAttributes(inherited, display, level, prime);
    }
    AbstractMmlNode::setChildInheritedAttributes(inherited, display, level, prime);
}

MmlNode *MmlMfenced::setTeXclass(MmlNode *previous) {
    getPrevClass(previous);
    if (open)
        previous = open->setTeXclass(previous);
    if (!children().empty() && children()[0])
        previous = children()[0]->setTeXclass(previous);
    for (std::size_t index = 1; index < children().size(); ++index) {
        const auto separatorIndex = index - 1;
        if (separatorIndex < separators.size() && separators[separatorIndex]) {
            previous = separators[separatorIndex]->setTeXclass(previous);
        }
        if (children()[index])
            previous = children()[index]->setTeXclass(previous);
    }
    if (close)
        previous = close->setTeXclass(previous);
    if (!open || !close) {
        MmlNode *target = open;
        if (!target && !children().empty())
            target = children()[0];
        if (!target)
            target = close;
        updateTeXclass(target);
    }
    return previous;
}
} // namespace folia::math::mml
