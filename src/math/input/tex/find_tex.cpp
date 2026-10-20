// MathJax 4.1.3 input/tex/FindTeX.ts; Apache-2.0.
module folia.math.input.tex.find_tex;
import folia.math.util.string;
import folia.math.util.regexp_host;
import std;

namespace folia::math::tex {
namespace {

std::u16string join(std::span<std::u16string const> strings) {
    std::u16string result;
    for (std::size_t i = 0; i < strings.size(); ++i) {
        if (i) result.push_back(u'|');
        result += strings[i];
    }
    return result;
}

std::u16string captureOrEmpty(regexp_host::Match const& match,
                              std::size_t index) {
    const auto value = match.at(index);
    return value ? *value : std::u16string{};
}

} // namespace

FindTeX::FindTeX(FindTeXOptions options)
    : core::AbstractFindMath<FindTeXOptions>(std::move(options)) {
    getPatterns();
}

void FindTeX::getPatterns() {
    std::vector<std::u16string> starts;
    std::vector<std::u16string> parts;
    std::vector<std::u16string> specialParts;
    end_.clear();
    environmentCapture_ = 0;
    specialCapture_ = 0;
    std::size_t nextCapture = 1;

    for (const auto& delimiters : options_.inlineMath)
        addPattern(starts, delimiters, false);
    for (const auto& delimiters : options_.displayMath)
        addPattern(starts, delimiters, true);

    if (!starts.empty()) {
        std::stable_sort(starts.begin(), starts.end(), [](auto const& left,
                                                          auto const& right) {
            return string_util::sortLength(left, right) < 0;
        });
        parts.push_back(join(starts));
    }

    if (options_.processEnvironments) {
        parts.emplace_back(u"\\\\begin\\s*\\{([^}]*)\\}");
        environmentCapture_ = nextCapture++;
    }
    if (options_.processEscapes) specialParts.emplace_back(u"\\\\([\\\\$])");
    if (options_.processRefs)
        specialParts.emplace_back(u"(\\\\(?:eq)?ref\\s*\\{[^}]*\\})");
    if (!specialParts.empty()) {
        parts.push_back(u"(" + join(specialParts) + u")");
        specialCapture_ = nextCapture;
    }

    hasPatterns_ = !parts.empty();
    start_ = std::make_shared<regexp_host::RegExp>(join(parts), u"g");
}

void FindTeX::addPattern(std::vector<std::u16string>& starts,
                         std::array<std::u16string, 2> const& delimiters,
                         bool display) {
    const auto& open = delimiters[0];
    const auto& close = delimiters[1];
    if (open.empty())
        throw std::invalid_argument("FindTeX opening delimiters cannot be empty");
    starts.push_back(string_util::quotePattern(open));
    end_.insert_or_assign(open, EndItem{close, display, endPattern(close)});
}

std::shared_ptr<regexp_host::RegExp> FindTeX::endPattern(
    std::u16string_view close,
    std::optional<std::u16string_view> overridePattern) const {
    std::u16string pattern;
    if (overridePattern && !overridePattern->empty())
        pattern.assign(*overridePattern);
    else
        pattern = string_util::quotePattern(close);
    pattern += u"|\\\\(?:[a-zA-Z]|.)|[{}]";
    return std::make_shared<regexp_host::RegExp>(std::move(pattern), u"g");
}

std::optional<core::ProtoItem> FindTeX::findEnd(
    std::u16string_view text, std::size_t n, regexp_host::Match const& start,
    EndItem const& end) const {
    const auto startText = start.at(0);
    if (!startText) throw std::logic_error("FindTeX start match has no full match");
    const std::size_t contentStart = start.index + startText->size();
    end.pattern->last_index = contentStart;
    std::size_t braces = 0;
    const std::u16string source(text);
    for (;;) {
        const auto match = end.pattern->exec(source);
        if (!match) break;
        const auto capture = match->at(1);
        const std::u16string candidate = capture && !capture->empty()
            ? *capture : captureOrEmpty(*match, 0);
        if (candidate == end.close && braces == 0) {
            const auto full = match->at(0);
            if (!full) throw std::logic_error("FindTeX end match has no full match");
            const auto display = end.display ? core::MathDisplay::display_math
                                             : core::MathDisplay::inline_math;
            return core::protoItem(*startText,
                std::u16string(text.substr(contentStart, match->index - contentStart)),
                *full, n, start.index, match->index + full->size(), display);
        }
        const auto full = captureOrEmpty(*match, 0);
        if (full == u"{") ++braces;
        else if (full == u"}" && braces) --braces;
    }
    return {};
}

void FindTeX::findMathInString(std::vector<core::ProtoItem>& math,
                              std::size_t n,
                              std::u16string const& text) {
    start_->last_index = 0;
    for (;;) {
        const auto start = start_->exec(text);
        if (!start) break;

        std::optional<core::ProtoItem> match;
        const auto environment = environmentCapture_ ? start->at(environmentCapture_)
                                                     : std::nullopt;
        if (environmentCapture_ && environment) {
            const std::u16string ending = u"\\\\end\\s*(\\{" +
                string_util::quotePattern(*environment) + u"\\})";
            EndItem end{u"{" + *environment + u"}", true,
                        endPattern({}, std::u16string_view(ending))};
            match = findEnd(text, n, *start, end);
            if (match) {
                match->math = match->open + match->math + match->close;
                match->open.clear();
                match->close.clear();
            }
        } else {
            const auto special = specialCapture_ ? start->at(specialCapture_)
                                                 : std::nullopt;
            if (specialCapture_ && special) {
                const auto end = start->index + special->size();
                if (special->size() == 2) {
                    match = core::protoItem(u"\\", special->substr(1), u"", n,
                        start->index, end);
                } else {
                    match = core::protoItem(u"", *special, u"", n,
                        start->index, end, core::MathDisplay::inline_math);
                }
            } else {
                const auto open = start->at(0);
                if (!open) throw std::logic_error("FindTeX start match has no full match");
                const auto end = end_.find(*open);
                if (end == end_.end())
                    throw std::logic_error("FindTeX start delimiter has no end record");
                match = findEnd(text, n, *start, end->second);
            }
        }

        if (match) {
            start_->last_index = match->end.n;
            math.emplace_back(std::move(*match));
        }
    }
}

std::vector<core::ProtoItem> FindTeX::findMath(
    std::span<std::u16string const> strings) {
    std::vector<core::ProtoItem> math;
    if (!hasPatterns_) return math;
    for (std::size_t i = 0; i < strings.size(); ++i)
        findMathInString(math, i, strings[i]);
    return math;
}

} // namespace folia::math::tex
