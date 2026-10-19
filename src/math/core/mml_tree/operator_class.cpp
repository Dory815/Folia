module folia.math.core.mml_tree.operator_class;
import std;

namespace folia::math::mml {
namespace {
bool truthy(std::optional<bool> value) {
    return value.value_or(false);
}

std::optional<double> numericAttribute(AttributeSet const &attributes, std::u16string_view name) {
    const auto *value = attributes.lookup(name);
    if (const auto *number = value ? std::get_if<double>(value) : nullptr)
        return *number;
    return std::nullopt;
}
} // namespace

MmlNode *adjustOperatorTeXclass(MmlNode &receiver, MmlNode *previous) {
    const auto texClass = receiver.texClass();
    std::optional<TexClass> previousClass = receiver.prevClass();
    if (texClass == TexClass::none)
        return previous;

    if (previous) {
        const auto current = receiver.texClass();
        if (truthy(previous->properties().tex.auto_operator) && current &&
            (*current == TexClass::bin || *current == TexClass::rel)) {
            previous->setTexClass(TexClass::ord);
            previousClass = TexClass::ord;
        }

        previousClass = previous->texClass().value_or(TexClass::ord);
        receiver.setPrevClass(previousClass);
        receiver.setPrevLevel(numericAttribute(receiver.attributes(), u"scriptlevel"));
    } else {
        previousClass = TexClass::none;
        receiver.setPrevClass(previousClass);
    }

    if (texClass == TexClass::bin &&
        (previousClass == TexClass::none || previousClass == TexClass::bin ||
         previousClass == TexClass::op || previousClass == TexClass::rel ||
         previousClass == TexClass::open || previousClass == TexClass::punct)) {
        receiver.setTexClass(TexClass::ord);
    } else if (previousClass == TexClass::bin && texClass &&
               (*texClass == TexClass::rel || *texClass == TexClass::close ||
                *texClass == TexClass::punct)) {
        receiver.setPrevClass(TexClass::ord);
        previousClass = TexClass::ord;
        previous->setTexClass(TexClass::ord);
    } else if (texClass == TexClass::bin) {
        MmlNode *child = nullptr;
        MmlNode *parent = receiver.parent();
        while (parent) {
            MmlNode *const conditionParent = parent->parent();
            if (!conditionParent)
                break;
            if (!parent->isEmbellished())
                break;
            const auto children = parent->children();
            bool continueWalk = children.size() == 1;
            if (!continueWalk) {
                const bool isRow = parent->isKind(NodeKind::mrow);
                if (!isRow)
                    continueWalk = parent->core() == child;
            }
            if (!continueWalk)
                break;
            child = parent;
            parent = parent->parent();
        }
        if (!child)
            child = &receiver;
        if (!parent)
            throw std::logic_error("binary operator has no parent endpoint");
        const auto children = parent->children();
        if (!children.empty() && children.back() == child)
            receiver.setTexClass(TexClass::ord);
    }
    return &receiver;
}
} // namespace folia::math::mml
