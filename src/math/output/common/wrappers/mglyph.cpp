module folia.math.output.common;
import :mglyph;
import :text_node;
import folia.math.util.js_numeric_input;
import folia.math.util.string;
import std;

namespace folia::math::output::common {
namespace {
LengthInput lengthValue(mml::AttributeValue const *value) {
    if (const auto *number = value ? std::get_if<double>(value) : nullptr)
        return *number;
    if (const auto *text = value ? std::get_if<std::u16string>(value) : nullptr)
        return *text;
    return std::u16string{};
}

std::u16string codePointText(double point) {
    if (!std::isfinite(point) || std::trunc(point) != point || point < 0 || point > 0x10FFFF)
        throw std::range_error("Invalid mglyph index code point");
    const std::array<double, 1> codepoint{point};
    return string_util::unicodeString(codepoint);
}

std::u16string attributeText(mml::AttributeSet const &attributes, std::u16string_view name) {
    const auto *value = attributes.lookup(name);
    if (const auto *text = value ? std::get_if<std::u16string>(value) : nullptr)
        return *text;
    return {};
}

bool attributeTruthy(mml::AttributeSet const &attributes, std::u16string_view name) {
    const auto *value = attributes.lookup(name);
    if (const auto *text = value ? std::get_if<std::u16string>(value) : nullptr)
        return !text->empty();
    if (const auto *number = value ? std::get_if<double>(value) : nullptr)
        return *number != 0 && !std::isnan(*number);
    if (const auto *boolean = value ? std::get_if<bool>(value) : nullptr)
        return *boolean;
    return false;
}
} // namespace

void CommonMglyph::initializeSource() {
    CommonWrapper::initializeSource();
    if (!dynamic_cast<mml::MmlMglyph *>(node))
        throw std::logic_error("CommonMglyph requires an MmlMglyph node");
    getParameters();
}

void CommonMglyph::getParameters() {
    const auto &attributes = node->attributes();
    const auto source = attributeText(attributes, u"src");
    const auto widthValue = attributeText(attributes, u"width");
    const auto heightValue = attributeText(attributes, u"height");
    const auto valignValue = attributeText(attributes, u"valign");
    if (attributeTruthy(attributes, u"src")) {
        width = widthValue == u"auto" ? 1 : length2em(lengthValue(attributes.lookup(u"width")));
        height = heightValue == u"auto" ? 1 : length2em(lengthValue(attributes.lookup(u"height")));
        const auto *rawValign = attributes.lookup(u"valign");
        const auto valignInput = attributeTruthy(attributes, u"valign")
                                     ? lengthValue(rawValign)
                                     : LengthInput{std::u16string{u"0"}};
        valign = length2em(valignInput);
        return;
    }

    const auto index = js_input::parseInt(attributeText(attributes, u"index"));
    auto text = codePointText(index);
    auto &textNode = mmlText(std::move(text));
    auto &wrapped = wrapNode(textNode);
    charWrapper_ = dynamic_cast<CommonTextNode *>(&wrapped);
    if (!charWrapper_)
        throw std::logic_error("mglyph index text did not receive a CommonTextNode wrapper");
}

void CommonMglyph::computeBBox(BBox &box, bool) {
    if (charWrapper_) {
        box.updateFrom(charWrapper_->getBBox());
        return;
    }
    box.w = width;
    box.h = height + valign;
    box.d = -valign;
}
} // namespace folia::math::output::common
