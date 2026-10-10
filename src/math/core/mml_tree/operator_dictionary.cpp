/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native host methods for MathJax 4.1.3 OperatorDictionary.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.operator_dictionary;
import folia.math.core.mml_tree.mml_node;
import folia.math.core.mml_tree.mml_node_host;
import folia.math.util.utf16;
import std;

namespace folia::math::mml::operator_dict {

OperatorDef OPDEF(Property left, Property right, Property texClass, Property properties) {
    if (texClass.missing()) texClass = static_cast<double>(TEXCLASS.BIN);
    if (properties.missing()) properties = PropertyNull{};
    return PropertyArray{std::move(left), std::move(right),
                         std::move(texClass), std::move(properties)};
}

RangeDef getRange(std::u16string_view text) {
    const std::string key = host::debugText(text);

    // Preserve JavaScript's ordered `a || b || c` reads. In particular, an
    // inherited truthy Object.prototype value is a dictionary hit.
    Property infix = host::get(Property(OPTABLE()), "infix");
    Property definition = host::get(infix, key);
    if (!host::truthy(definition)) {
        Property prefix = host::get(Property(OPTABLE()), "prefix");
        definition = host::get(prefix, key);
    }
    if (!host::truthy(definition)) {
        Property postfix = host::get(Property(OPTABLE()), "postfix");
        definition = host::get(postfix, key);
    }
    if (host::truthy(definition)) {
        Property texClass = host::get(definition, "2");
        return PropertyArray{Property(0.0), Property(0.0),
                             std::move(texClass), Property("mo")};
    }

    const auto first = utf16::codePointAt(text, 0);
    const Property codepoint = first
        ? Property(static_cast<double>(*first))
        : Property(Missing{});

    // MathJax's for-of captures the RANGES array once; its array iterator
    // consults the current length and current indexed slots on each step.
    const PropertyArray ranges = RANGES();
    for (std::size_t i = 0; i < ranges.size(); ++i) {
        Property row = ranges.get(i);

        // Source reads and compares the upper endpoint first. It reads the
        // lower endpoint only when the upper comparison succeeds, and then
        // breaks even if that lower comparison fails.
        Property upper = host::get(row, "1");
        const double pointValue = host::number(codepoint);
        const double upperValue = host::number(upper);
        if (pointValue <= upperValue) {
            Property lower = host::get(row, "0");
            const double lowerValue = host::number(lower);
            if (pointValue >= lowerValue) return row;
            break;
        }
    }
    return PropertyArray{Property(0.0), Property(0.0),
                         Property(static_cast<double>(TEXCLASS.REL)),
                         Property("mo")};
}

} // namespace folia::math::mml::operator_dict
