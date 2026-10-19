module folia.math.output.common;
import :ms;
import std;

namespace folia::math::output::common {
namespace {
std::u16string quoteValue(mml::AttributeSet const &attributes, std::u16string_view name) {
    const auto *value = attributes.lookup(name);
    if (const auto *text = value ? std::get_if<std::u16string>(value) : nullptr)
        return *text;
    return u"\"";
}
} // namespace

CommonWrapper &CommonMs::createText(std::u16string_view text) {
    return wrapNode(mmlText(std::u16string{text}));
}

void CommonMs::initializeSource() {
    CommonWrapper::initializeSource();
    const auto &attributes = node->attributes();
    auto left = quoteValue(attributes, u"lquote");
    auto right = quoteValue(attributes, u"rquote");
    if (variant != "monospace") {
        if (!attributes.isSet(u"lquote") && left == u"\"")
            left = u"\u201c";
        if (!attributes.isSet(u"rquote") && right == u"\"")
            right = u"\u201d";
    }

    auto &opening = createText(left);
    childNodes.insert(childNodes.begin(), &opening);
    auto &closing = createText(right);
    childNodes.push_back(&closing);
}
} // namespace folia::math::output::common
