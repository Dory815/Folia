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
    // P23f Root stub: MmlMo::checkPseudoScripts.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
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
    // P23f Root stub: MmlMo::checkMathAccent.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
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
