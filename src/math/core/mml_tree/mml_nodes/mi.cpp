/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mi.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
namespace {
const AttributeValue *inheritedValue(AttributeList const &attributes, std::u16string_view name) {
    const auto found =
        std::find_if(attributes.begin(), attributes.end(),
                     [name](const InheritedAttribute &entry) { return entry.name == name; });
    return found == attributes.end() ? nullptr : &found->value;
}

bool isCombiningMark(char16_t value) {
    return (value >= 0x0300 && value <= 0x036F) || (value >= 0x1AB0 && value <= 0x1ABE) ||
           (value >= 0x1DC0 && value <= 0x1DFF) || (value >= 0x20D0 && value <= 0x20EF);
}

bool isRegexDot(char16_t value) {
    return value != u'\n' && value != u'\r' && value != 0x2028 && value != 0x2029;
}
} // namespace

MmlMi::MmlMi(MmlDocument &document) : AbstractMmlTokenNode(document, NodeKind::mi) {}

AttributeSet MmlMi::defaults() {
    return nodeDefaultAttributes(NodeKind::mi);
}

NodeKind MmlMi::kind() const noexcept {
    return NodeKind::mi;
}

bool MmlMi::operatorName(std::u16string_view value) {
    const auto alpha = [](char16_t c) {
        return (c >= u'a' && c <= u'z') || (c >= u'A' && c <= u'Z');
    };
    if (value.empty() || !alpha(value.front()))
        return false;
    return std::all_of(value.begin() + 1, value.end(),
                       [&](char16_t c) { return alpha(c) || (c >= u'0' && c <= u'9'); });
}

bool MmlMi::singleCharacter(std::u16string_view value) {
    const auto match = [&](std::size_t start) {
        if (start >= value.size() || !isRegexDot(value[start]))
            return false;
        return std::all_of(value.begin() + static_cast<std::ptrdiff_t>(start + 1), value.end(),
                           isCombiningMark);
    };
    const bool surrogatePrefix =
        !value.empty() && value.front() >= 0xD800 && value.front() <= 0xDBFF;
    return match(0) || (surrogatePrefix && match(1));
}

void MmlMi::inheritAttributes(AttributeList const &inherited, bool display, double level,
                              bool prime) {
    AbstractMmlTokenNode::inheritAttributes(inherited, display, level, prime);
    const auto value = getText();
    const auto *variant = inheritedValue(inherited, u"mathvariant");
    // Source receives inherited entries as [node, value] arrays. The array's
    // truthiness depends on presence, even when its value is false/zero/empty.
    if (singleCharacter(value) && !variant)
        attributes().setInherited(u"mathvariant", std::u16string(u"italic"));
}

MmlNode *MmlMi::setTeXclass(MmlNode *previous) {
    getPrevClass(previous);
    const auto name = getText();
    if (name.size() > 1 && operatorName(name)) {
        const auto *variant = attributes().lookup(u"mathvariant");
        const auto *variantName = variant ? std::get_if<std::u16string>(variant) : nullptr;
        auto &tex = properties().tex;
        if (variantName && *variantName == u"normal" &&
            // Source tests getProperty(...) === undefined, so engaged false
            // is distinct from absence and suppresses automatic operator class.
            !tex.auto_operator && !tex.explicit_tex_class) {
            setTexClass(TexClass::op);
            tex.auto_operator = true;
        }
    }
    return this;
}
} // namespace folia::math::mml
