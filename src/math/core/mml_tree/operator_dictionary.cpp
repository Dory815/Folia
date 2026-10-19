// Typed lookups preserving OperatorDictionary.ts form precedence and ranges.
module folia.math.core.mml_tree.operator_dictionary;
import std;

namespace folia::math::mml::operator_dict {
OperatorDef OPDEF(double left, double right, TexClass texClass,
                  std::vector<AttributeSet::Entry> properties) {
    return {left, right, texClass, std::move(properties)};
}

const OperatorDef* findOperatorDefinition(std::u16string_view form,
                                          std::u16string_view text) {
    for (const auto& entry : OPTABLE()) {
        if (entry.form == form && entry.text == text) return &entry.definition;
    }
    return nullptr;
}

RangeDef getRange(std::u16string_view text) {
    constexpr std::array<std::u16string_view, 3> forms{u"infix", u"prefix", u"postfix"};
    for (const auto form : forms) {
        if (const auto* definition = findOperatorDefinition(form, text)) {
            return {0, 0, definition->texClass, u"mo"};
        }
    }

    char32_t codepoint = 0;
    if (!text.empty()) {
        const auto first = static_cast<std::uint16_t>(text.front());
        if (first >= 0xd800 && first <= 0xdbff && text.size() > 1) {
            const auto second = static_cast<std::uint16_t>(text[1]);
            if (second >= 0xdc00 && second <= 0xdfff) {
                codepoint = 0x10000 + ((first - 0xd800) << 10) + (second - 0xdc00);
            } else {
                codepoint = first;
            }
        } else {
            codepoint = first;
        }
    }
    for (const auto& range : RANGES()) {
        if (range.first <= codepoint && codepoint <= range.last) return range;
    }
    return {0, 0, TexClass::rel, u"mo"};
}
}
