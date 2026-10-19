module folia.math.output.common;
import :mspace;
import :output_jax;
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

mml::MmlMspace &requireMspace(mml::MmlNode &node) {
    auto *space = dynamic_cast<mml::MmlMspace *>(&node);
    if (!space)
        throw std::logic_error("CommonMspace requires an MmlMspace node");
    return *space;
}
} // namespace

bool CommonMspace::canBreak() const {
    auto *self = const_cast<CommonMspace *>(this);
    return requireMspace(*self->node).canBreak();
}

std::size_t CommonMspace::breakCount() {
    return breakStyle.empty() ? 0 : 1;
}

void CommonMspace::setBreakStyle(std::string_view linebreak) {
    if (!linebreak.empty()) {
        breakStyle = linebreak;
        return;
    }
    const bool forced = node->properties().layout.force_break.value_or(false);
    breakStyle = (requireMspace(*node).hasNewline() || forced) ? "before" : "";
}

void CommonMspace::initializeSource() {
    CommonWrapper::initializeSource();
    setBreakStyle();
}

void CommonMspace::computeBBox(BBox &box, bool) {
    const auto &attributes = node->attributes();
    box.w = length2em(lengthValue(attributes.lookup(u"width")), 0);
    box.h = length2em(lengthValue(attributes.lookup(u"height")), 0);
    box.d = length2em(lengthValue(attributes.lookup(u"depth")), 0);
}

std::shared_ptr<LineBBox> CommonMspace::computeLineBBox(std::size_t index) {
    const auto &attributes = node->attributes();
    const auto *leading = attributes.lookup(u"data-lineleading");
    const auto defaultLeading = jax().currentLinebreakOptions().lineleading;
    const auto leadingValue = leading ? lengthValue(leading) : LengthInput{defaultLeading};
    const double lineLeading = length2em(leadingValue, defaultLeading);
    auto result = std::make_shared<LineBBox>(LineBBox::from(BBox::zero(), lineLeading));
    if (index == 1) {
        result->getIndentData(*node);
        result->w = getBBox().w;
        result->isFirst = result->w == 0;
    }
    return result;
}
} // namespace folia::math::output::common
