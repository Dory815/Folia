/*************************************************************
 * Source port of MathJax 4.1.3 output/common/LineBBox.ts.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common.line_bbox;
import std;

namespace folia::math::output::common {
LineBBox LineBBox::from(const BBox& bbox, std::optional<double> leading,
                        std::shared_ptr<PropertyArray> indent) {
    LineBBox result;
    static_cast<BBox&>(result) = bbox;
    // Source constructs a default LineBBox (whose originalL is its initial
    // L), then Object.assigns the plain BBox fields.  originalL is therefore
    // not replaced by bbox.L here; copying another LineBBox uses copy() to
    // transfer its explicit originalL field.
    result.lineLeading = leading;
    if (indent) result.indentData = std::move(indent);
    return result;
}

LineBBox::LineBBox(BBoxData def,
                   std::optional<std::array<double, 2>> startValue)
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
    // Source copy retains indentData's array identity.
    result.indentData = indentData;
    result.lineLeading = lineLeading;
    result.start = start;
    result.end = end;
    result.originalL = originalL;
    result.isFirst = isFirst;
    return result;
}

void LineBBox::getIndentData(const mml::MmlNode& node) {
    const auto attributes = node.attributes()->getAllAttributes();
    auto align = attributes.get("indentalign");
    auto shift = attributes.get("indentshift");
    auto firstAlign = attributes.get("indentalignfirst");
    auto firstShift = attributes.get("indentshiftfirst");
    auto lastAlign = attributes.get("indentalignlast");
    auto lastShift = attributes.get("indentshiftlast");
    if (firstAlign == Property("indentalign"))
        firstAlign = node.attributes()->getInherited("indentalign");
    if (firstShift == Property("indentshift"))
        firstShift = node.attributes()->getInherited("indentshift");
    if (lastAlign == Property("indentalign")) lastAlign = align;
    if (lastShift == Property("indentshift")) lastShift = shift;
    indentData = std::make_shared<PropertyArray>();
    const auto pair = [](Property first, Property second) {
        return Property(PropertyArray{std::move(first), std::move(second)});
    };
    indentData->push(pair(std::move(firstAlign), std::move(firstShift)));
    indentData->push(pair(std::move(align), std::move(shift)));
    indentData->push(pair(std::move(lastAlign), std::move(lastShift)));
}

std::shared_ptr<PropertyArray> LineBBox::copyIndentData(const LineBBox& bbox) const {
    if (!bbox.indentData)
        throw options::Exception("TypeError", u"Cannot read properties of null (reading 'map')");
    auto result = std::make_shared<PropertyArray>();
    const PropertyList source = bbox.indentData->record();
    const auto length = bbox.indentData->size();
    result->resize(length);
    for (std::size_t index = 0; index < length; ++index) {
        const auto name = std::to_string(index);
        if (!source.has(name)) continue; // Array#map preserves holes.
        const Property pairValue = source.get(name);
        const auto pair = PropertyArray::array(pairValue);
        if (!pair)
            throw options::Exception("TypeError", u"object is not iterable");
        const PropertyList pairRecord = pair->record();
        const Property first = pairRecord.get("0");
        const Property second = pairRecord.get("1");
        result->set(index, Property(PropertyArray{first, second}));
    }
    return result;
}
}
