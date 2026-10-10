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

void MmlMo::setInheritedAttributes(AttributeList attributes, Property display,
                                   Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    host::inheritanceDefaults(display, level, prime);
    AbstractMmlTokenNode::setInheritedAttributes(std::move(attributes), display,
                                                 level, prime);

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
    const auto pattern = host::moStatic(services.pseudoScripts);
    if (!host::moMatch(pattern, mo)) return;

    MmlNode* const core = coreParent();
    if (!core) host::typeError("Cannot read Parent from missing core parent");
    MmlNode* const parent = core->Parent();
    const bool isPseudo = !parent ||
        !(parent->isKind("msubsup") && !parent->isKind("msub"));
    setProperty("pseudoscript", isPseudo);
    if (isPseudo) {
        auto currentAttributes = attributes();
        if (!currentAttributes) host::typeError("Cannot set inherited lspace on null mo attributes");
        currentAttributes->setInherited("lspace", 0);
        currentAttributes = attributes();
        if (!currentAttributes) host::typeError("Cannot set inherited rspace on null mo attributes");
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
    MmlNode* const parent = Parent();
    if (!getProperty("mathaccent").missing()) return;
    if (!parent || !parent->isKind("munderover")) return;

    const Children nodes = parent->childNodes;
    if (!nodes) host::typeError("Cannot destructure null munderover children");
    MmlNode* const base = nodes->empty() ? nullptr : (*nodes)[0];
    MmlNode* const under = nodes->size() > 1 ? (*nodes)[1] : nullptr;
    MmlNode* const over = nodes->size() > 2 ? (*nodes)[2] : nullptr;
    if (!base) host::typeError("Cannot read base embellishment");
    if (base->isEmbellished() && base->coreMO() == this) return;

    const bool isUnder = under && under->isEmbellished() && under->coreMO() == this;

    // Keep the upstream isOver expression as written: it tests under.coreMO()
    // (and may therefore throw after a truthy over test when under is absent).
    bool isOver = false;
    if (over && over->isEmbellished()) {
        if (!under) host::typeError("Cannot read coreMO of missing under child");
        isOver = under->coreMO() == this;
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
