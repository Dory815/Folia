/*************************************************************
 * Source port of MathJax 4.1.3 output/common/LineBBox.ts.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common.line_bbox;
import std;

namespace folia::math::output::common {
namespace {
using mml::AttributeSet;

std::optional<std::u16string> string_attribute(
    AttributeSet const& attributes, std::u16string_view name) {
    auto const* value = attributes.lookupRaw(name);
    if (!value) return std::nullopt;
    auto const* string = std::get_if<std::u16string>(value);
    if (!string)
        throw std::invalid_argument("LineBBox indentation attributes must be strings");
    return *string;
}

std::optional<std::u16string> inherited_string(
    AttributeSet const& attributes, std::u16string_view name) {
    auto const* value = attributes.effectiveInherited(name);
    if (!value) return std::nullopt;
    auto const* string = std::get_if<std::u16string>(value);
    if (!string)
        throw std::invalid_argument("LineBBox inherited indentation attributes must be strings");
    return *string;
}

bool aliases(std::optional<std::u16string> const& value, std::u16string_view name) {
    return value && *value == name;
}
}

LineBBox LineBBox::from(BBox const& bbox, std::optional<double> leading,
                        IndentDataRef indent) {
    LineBBox result;
    static_cast<BBox&>(result) = bbox;
    result.lineLeading = leading;
    if (indent) result.indentData = std::move(indent);
    return result;
}

LineBBox::LineBBox(BBoxData def, std::optional<std::array<double, 2>> startValue)
    : BBox(std::move(def)), originalL(L) {
    if (startValue) start = *startValue;
}

void LineBBox::append(LineBBox& child) {
    if (isFirst) {
        child.originalL += child.L;
        child.L = 0;
    }
    if (child.indentData) indentData = child.indentData;
    lineLeading = child.lineLeading;
    BBox::append(child);
    isFirst = child.isFirst;
}

LineBBox LineBBox::copy() const {
    LineBBox result = LineBBox::from(*this, lineLeading);
    result.indentData = indentData;
    result.lineLeading = lineLeading;
    result.start = start;
    result.end = end;
    result.originalL = originalL;
    result.isFirst = isFirst;
    return result;
}

void LineBBox::getIndentData(mml::MmlNode const& node) {
    auto const& attributes = node.attributes();
    auto align = string_attribute(attributes, u"indentalign");
    auto shift = string_attribute(attributes, u"indentshift");
    auto first_align = string_attribute(attributes, u"indentalignfirst");
    auto first_shift = string_attribute(attributes, u"indentshiftfirst");
    auto last_align = string_attribute(attributes, u"indentalignlast");
    auto last_shift = string_attribute(attributes, u"indentshiftlast");
    if (aliases(first_align, u"indentalign")) first_align = inherited_string(attributes, u"indentalign");
    if (aliases(first_shift, u"indentshift")) first_shift = inherited_string(attributes, u"indentshift");
    if (aliases(last_align, u"indentalign")) last_align = align;
    if (aliases(last_shift, u"indentshift")) last_shift = shift;
    indentData = std::make_shared<IndentData>();
    indentData->lines = {
        IndentPair{std::move(first_align), std::move(first_shift)},
        IndentPair{std::move(align), std::move(shift)},
        IndentPair{std::move(last_align), std::move(last_shift)}};
}

IndentDataRef LineBBox::copyIndentData(LineBBox const& bbox) const {
    if (!bbox.indentData)
        throw std::invalid_argument("LineBBox has no indentation data to copy");
    return std::make_shared<IndentData>(*bbox.indentData);
}
}
