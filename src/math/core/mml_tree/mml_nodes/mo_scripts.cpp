module folia.math.core.mml_tree.mml_nodes.mo;
import std;

namespace folia::math::mml {
namespace {
bool isAccentChild(MmlNode *child, MmlNode const *target) {
    return child && child->isEmbellished() && child->coreMO() == target;
}
} // namespace

void MmlMo::inheritAttributes(AttributeList const &inherited, bool display, double level,
                              bool prime) {
    AbstractMmlTokenNode::inheritAttributes(inherited, display, level, prime);
    const std::u16string snapshot = getText();
    checkOperatorTable(snapshot);
    checkPseudoScripts(snapshot);
    checkPrimes(snapshot);
    checkMathAccent(snapshot);
}

void MmlMo::checkPseudoScripts(std::u16string_view text) {
    if (!matchMoPattern(pseudoScripts(), text))
        return;
    MmlNode *const core = coreParent();
    MmlNode *const parentNode = core ? core->Parent() : nullptr;
    bool pseudo = parentNode == nullptr;
    if (parentNode) {
        const bool isSubsup = parentNode->isKind(NodeKind::msubsup);
        const bool isOnlySubsup = isSubsup && !parentNode->isKind(NodeKind::msub);
        pseudo = !isOnlySubsup;
    }
    properties().tex.pseudo_script = pseudo;
    if (pseudo) {
        attributes().setInherited(u"lspace", 0.0);
        attributes().setInherited(u"rspace", 0.0);
    }
}

void MmlMo::checkPrimes(std::u16string_view text) {
    if (!matchMoPattern(primes(), text))
        return;
    std::u16string mapped;
    mapped.reserve(text.size());
    for (std::size_t index = 0; index < text.size(); ++index) {
        char32_t codepoint = text[index];
        if (codepoint >= 0xD800 && codepoint <= 0xDBFF && index + 1 < text.size()) {
            const char32_t low = text[index + 1];
            if (low >= 0xDC00 && low <= 0xDFFF) {
                codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
                ++index;
            }
        }
        const auto found =
            std::find_if(remapPrimes().begin(), remapPrimes().end(),
                         [codepoint](auto const &item) { return item.first == codepoint; });
        if (found == remapPrimes().end())
            throw std::range_error("prime remap missing a codepoint");
        const char32_t replacement = found->second;
        if (replacement <= 0xFFFF)
            mapped.push_back(static_cast<char16_t>(replacement));
        else {
            const char32_t scalar = replacement - 0x10000;
            mapped.push_back(static_cast<char16_t>(0xD800 + (scalar >> 10)));
            mapped.push_back(static_cast<char16_t>(0xDC00 + (scalar & 0x3FF)));
        }
    }
    properties().tex.primes = std::move(mapped);
}

void MmlMo::checkMathAccent(std::u16string_view text) {
    if (properties().tex.math_accent || !Parent())
        return;
    MmlNode *const parentNode = Parent();
    if (parentNode->kind() != NodeKind::munderover)
        return;
    const auto children = parentNode->children(); // snapshot before child inspection
    MmlNode *const base = children.size() > 0 ? children[0] : nullptr;
    MmlNode *const under = children.size() > 1 ? children[1] : nullptr;
    MmlNode *const over = children.size() > 2 ? children[2] : nullptr;
    if (base && base->isEmbellished() && base->coreMO() == this)
        return;
    const bool isUnder = isAccentChild(under, this);
    // Preserve pinned Source's isOver branch: it intentionally compares the
    // under child core, not the over child core.
    const bool isOver = over && over->isEmbellished() && under && under->coreMO() == this;
    if (!isUnder && !isOver)
        return;
    if (isMathAccent(text))
        properties().tex.math_accent = true;
    else if (isMathAccentWithWidth(text))
        properties().tex.math_accent = false;
}

bool MmlMo::isMathAccent(std::optional<std::u16string_view> text) {
    const std::u16string defaultText = text ? std::u16string{} : getText();
    const auto actual = text ? *text : std::u16string_view(defaultText);
    return matchMoPattern(mathaccents(), actual);
}

bool MmlMo::isMathAccentWithWidth(std::optional<std::u16string_view> text) {
    const std::u16string defaultText = text ? std::u16string{} : getText();
    const auto actual = text ? *text : std::u16string_view(defaultText);
    return matchMoPattern(mathaccentsWithWidth(), actual);
}
} // namespace folia::math::mml
