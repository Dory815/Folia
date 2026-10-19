module folia.math.output.common;
import :mtext;
import :output_jax;
import :text_node;
import folia.math.util.utf16;
import std;

namespace folia::math::output::common {
namespace {
constexpr std::array<std::string_view, 4> inheritedVariants{"normal", "bold", "italic",
                                                            "bold-italic"};

mml::TextNode &requireText(CommonWrapper &wrapper) {
    auto *text = dynamic_cast<mml::TextNode *>(wrapper.node);
    if (!text)
        throw std::logic_error("CommonMtext break child is not an MML TextNode");
    return *text;
}

std::u16string joinWords(std::span<std::u16string const> words) {
    std::u16string result;
    for (std::size_t i = 0; i < words.size(); ++i) {
        if (i)
            result.push_back(u' ');
        result += words[i];
    }
    return result;
}
} // namespace

CommonMtext::CommonMtext(CommonWrapperFactory &factory, mml::MmlNode &source, CommonWrapper *parent)
    : CommonWrapper(factory, source, parent) {
}

std::vector<std::u16string> CommonMtext::splitText(std::u16string_view text) const {
    std::vector<std::u16string> words;
    std::size_t start = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] != u' ')
            continue;
        words.emplace_back(text.substr(start, i - start));
        start = i + 1;
    }
    words.emplace_back(text.substr(start));
    return words;
}

double CommonMtext::textWidth(std::u16string_view text) {
    if (!textNode_) {
        auto &sourceText = mmlText(std::u16string{text});
        node->document().attachAuxiliary(*node, sourceText);
        auto &created = factory().wrap(sourceText);
        textNode_ = dynamic_cast<CommonTextNode *>(&created);
        if (!textNode_)
            throw std::logic_error("MML text did not receive CommonTextNode");
        textNode_->parent = this;
    }
    requireText(*textNode_).setText(std::u16string{text});
    textNode_->invalidateBBox(false);
    return textNode_->getBBox().w;
}

std::size_t CommonMtext::breakCount() {
    return breakPoints.size();
}

void CommonMtext::setBreakAt(IndexData index) {
    breakPoints.push_back(std::move(index));
}

void CommonMtext::clearBreakPoints() {
    breakPoints.clear();
}

void CommonMtext::getVariant() {
    const auto &options = jax().options();
    const auto &metrics = jax().currentMetrics();
    const auto *mmlParent = node->Parent();
    const bool inError = mmlParent && mmlParent->kind() == mml::NodeKind::merror;
    const bool inherited = inError ? options.merrorInheritFont : options.mtextInheritFont;
    std::string family = inherited ? metrics.family : std::string{};
    const std::string &configuredFamily = inError ? options.merrorFont : options.mtextFont;
    if (family.empty())
        family = configuredFamily;
    const bool explicitFont = (inherited && !metrics.family.empty()) || !configuredFamily.empty();
    if (!explicitFont) {
        CommonWrapper::getVariant();
        return;
    }

    std::string variant = "normal";
    if (const auto *value = node->attributes().lookup(u"mathvariant")) {
        if (const auto *text = std::get_if<std::u16string>(value))
            variant = utf16::encode(*text);
    }
    bool italic = false;
    bool bold = false;
    if (variant == "bold")
        bold = true;
    else if (variant == "italic")
        italic = true;
    else if (variant == "bold-italic")
        italic = bold = true;
    else if (std::find(inheritedVariants.begin(), inheritedVariants.end(), variant) ==
             inheritedVariants.end()) {
        const auto css = jax().face().getCssFont(variant);
        if (!css.family.empty())
            family = css.family;
        italic = css.italic;
        bold = css.bold;
    }
    this->variant = explicitVariant(family, bold ? "bold" : "", italic ? "italic" : "");
}

std::shared_ptr<LineBBox> CommonMtext::computeLineBBox(std::size_t index) {
    auto result =
        std::make_shared<LineBBox>(LineBBox::from(getOuterBBox(), linebreakOptions().lineleading));
    if (breakPoints.empty())
        return result;

    result->w = getBreakWidth(index);
    if (index == 0) {
        result->R = 0;
        addLeftBorders(*result);
    } else {
        result->L = 0;
        IndentData data;
        for (auto &line : data.lines) {
            line.alignment = u"left";
            line.shift = u"0";
        }
        result->indentData = std::make_shared<IndentData>(std::move(data));
        if (index == breakPoints.size())
            addRightBorders(*result);
    }
    return result;
}

double CommonMtext::getBreakWidth(std::size_t index) {
    const auto children = childNodes;
    if (children.empty())
        throw std::logic_error("CommonMtext has no child wrappers");
    const auto fallbackStart = std::array<std::size_t, 2>{0, 0};
    const auto fallbackEnd = std::array<std::size_t, 2>{children.size(), 0};
    const auto start = index && index - 1 < breakPoints.size()
                           ? breakPoints[index - 1].value_or(fallbackStart)
                           : fallbackStart;
    const auto end =
        index < breakPoints.size() ? breakPoints[index].value_or(fallbackEnd) : fallbackEnd;
    if (start[0] >= children.size())
        throw std::out_of_range("CommonMtext start child index");
    const auto wordsAt = [&](std::size_t child) {
        return splitText(requireText(*children[child]).getText());
    };

    auto words = wordsAt(start[0]);
    if (start[0] == end[0]) {
        const auto first = std::min(start[1], words.size());
        const auto last = std::max(first, std::min(end[1], words.size()));
        return textWidth(
            joinWords(std::span<std::u16string const>(words).subspan(first, last - first)));
    }

    const auto first = std::min(start[1], words.size());
    double width = textWidth(joinWords(std::span<std::u16string const>(words).subspan(first)));
    std::size_t child = start[0];
    while (++child < end[0] && child < children.size())
        width += children[child]->getBBox().w;
    if (child < children.size()) {
        words = wordsAt(child);
        const auto last = std::min(end[1], words.size());
        width += textWidth(joinWords(std::span<std::u16string const>(words).first(last)));
    }
    return width;
}
} // namespace folia::math::output::common
