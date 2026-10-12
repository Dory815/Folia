/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/mo.ts script/accent methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mo_host;
import folia.math.util.string;
import std;

namespace folia::math::mml {

void MmlMo::inheritAttributes(AttributeList attributes, Property display,
                              Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    host::inheritanceDefaults(attributes, display, level, prime);
    AbstractMmlTokenNode::inheritAttributes(attributes, display, level, prime);

    // Source takes one text snapshot after the superclass has completed, then
    // performs these checks in this order.
    const std::u16string mo = getText();
    checkOperatorTable(mo);
    checkPseudoScripts(mo);
    checkPrimes(mo);
    checkMathAccent(mo);
}

void MmlMo::checkPseudoScripts(std::u16string_view mo) {
    tree::TreeCallGuard<MmlNode> guard;
    const auto& services = constructorOperators();
    const MoPatternHandle pattern = host::moStatic(services.pseudoScripts);
    if (!host::moMatch(pattern, mo)) return;

    MmlNode* const core = coreParent();
    if (!core) host::typeError("Cannot read properties of null (reading 'Parent')");
    const NodeValue parentValue = core->Parent();
    MmlNode* const parent = parentValue.nodeOrNull();
    bool isPseudo = !parent;
    if (parent) {
        const bool isSubsup = parent->isKind("msubsup");
        const bool isOnlySubsup = isSubsup ? !parent->isKind("msub") : false;
        isPseudo = !isOnlySubsup;
    }
    setProperty("pseudoscript", isPseudo);
    if (isPseudo) {
        auto currentAttributes = this->attributes();
        if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'setInherited')");
        currentAttributes->setInherited("lspace", 0);
        currentAttributes = this->attributes();
        if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'setInherited')");
        currentAttributes->setInherited("rspace", 0);
    }
}

void MmlMo::checkPrimes(std::u16string_view mo) {
    tree::TreeCallGuard<MmlNode> guard;
    const auto& services = constructorOperators();
    const auto pattern = host::moStatic(services.primes);
    if (!host::moMatch(pattern, mo)) return;

    const Property remap = host::moStatic(services.remap);
    const auto codepoints = string_util::unicodeChars(mo);
    std::vector<Property> values;
    values.reserve(codepoints.size());
    for (const double codepoint : codepoints) {
        // These are integral Unicode scalar/code-unit values. Decimal property
        // keys match JavaScript's numeric Object key conversion exactly.
        const auto key = std::to_string(static_cast<std::uint32_t>(codepoint));
        values.push_back(host::get(remap, key));
    }
    std::vector<double> mapped;
    mapped.reserve(values.size());
    for (const Property& value : values) mapped.push_back(host::number(value));
    const std::u16string primes = string_util::unicodeString(mapped);
    setProperty("primes", host::debugText(primes));
}

void MmlMo::checkMathAccent(std::u16string_view mo) {
    tree::TreeCallGuard<MmlNode> guard;
    const NodeValue parentValue = this->Parent();
    if (!getProperty("mathaccent").missing() || !parentValue.nodeOrNull()) return;
    MmlNode* const parent = parentValue.nodeOrNull();
    if (!parent->isKind("munderover")) return;

    const auto nodes = parent->childNodes;
    if (!nodes) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    const auto next = [&](std::size_t index) -> NodeValue {
        if (index >= nodes->size()) return Missing{};
        return nodes->get(index);
    };
    const NodeValue base = next(0);
    const NodeValue under = next(1);
    const NodeValue over = next(2);

    MmlNode* const baseNode = base.requireNode("isEmbellished");
    if (baseNode->isEmbellished() && baseNode->coreMO() == NodeValue(this)) return;

    bool isUnder = false;
    if (MmlNode* const underNode = under.nodeOrNull()) {
        if (underNode->isEmbellished()) isUnder = underNode->coreMO() == NodeValue(this);
    }
    bool isOver = false;
    if (MmlNode* const overNode = over.nodeOrNull()) {
        if (overNode->isEmbellished()) {
            MmlNode* const underNode = under.requireNode("coreMO");
            isOver = underNode->coreMO() == NodeValue(this);
        }
    }
    if (!isUnder && !isOver) return;
    if (isMathAccent(mo)) {
        setProperty("mathaccent", true);
    } else if (isMathAccentWithWidth(mo)) {
        setProperty("mathaccent", false);
    }
}

bool MmlMo::isMathAccent(std::optional<std::u16string_view> text) {
    tree::TreeCallGuard<MmlNode> guard;
    const std::u16string defaultText = text ? std::u16string{} : getText();
    const std::u16string_view actual = text ? *text : std::u16string_view(defaultText);
    const auto& services = constructorOperators();
    const auto pattern = host::moStatic(services.mathaccents);
    return host::moMatch(pattern, actual);
}

bool MmlMo::isMathAccentWithWidth(std::optional<std::u16string_view> text) {
    tree::TreeCallGuard<MmlNode> guard;
    const std::u16string defaultText = text ? std::u16string{} : getText();
    const std::u16string_view actual = text ? *text : std::u16string_view(defaultText);
    const auto& services = constructorOperators();
    const auto pattern = host::moStatic(services.mathaccentsWithWidth);
    return host::moMatch(pattern, actual);
}

}
