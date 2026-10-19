/*************************************************************
 * MathJax 4.1.3 TexError.ts message formatting; Apache-2.0.
 * Source hashes and oracle evidence are recorded in README.md.
 *************************************************************/
module folia.math.input.tex.tex_error;
import std;

namespace folia::math::tex {
namespace {
using view = std::u16string_view;
bool digit(char16_t c) noexcept { return c >= u'0' && c <= u'9'; }
bool lower(char16_t c) noexcept { return c >= u'a' && c <= u'z'; }
bool lineTerminator(char16_t c) noexcept {
    return c == u'\n' || c == u'\r' || c == 0x2028 || c == 0x2029;
}
bool dot(char16_t c) noexcept { return !lineTerminator(c); }

std::size_t pluralBodyStart(view s, std::size_t start) noexcept {
    if (start >= s.size() || s[start++] != u'{') return view::npos;
    auto const letters = start;
    while (start < s.size() && lower(s[start])) ++start;
    if (start == letters || start + 1 >= s.size() || s[start] != u':' || s[start + 1] != u'%') return view::npos;
    start += 2;
    auto const digits = start;
    while (start < s.size() && digit(s[start])) ++start;
    if (start == digits || start >= s.size() || s[start] != u'|') return view::npos;
    return start + 1;
}

// Match the capture alternatives of the fixed Source regex, preserving ordered
// alternatives and greedy repetition with suffix backtracking.
std::size_t captureEnd(view s, std::size_t start) {
    if (start >= s.size()) return view::npos;
    if (digit(s[start])) {
        auto end = start + 1;
        while (end < s.size() && digit(s[end])) ++end;
        return end;
    }
    if (s[start] == u'{') {
        auto end = start + 1;
        while (end < s.size() && digit(s[end])) ++end;
        if (end > start + 1 && end < s.size() && s[end] == u'}') return end + 1;
        auto const body = pluralBodyStart(s, start);
        if (body != view::npos) {
            std::vector<std::size_t> terminal(s.size() + 1, view::npos);
            for (auto i = s.size(); i-- > body;) {
                if (s[i] == u'}') { terminal[i] = i + 1; continue; }
                if (s[i] == u'%' && i + 1 < s.size()) {
                    if (s[i + 1] == u'{') {
                        auto j = i + 2;
                        while (j < s.size() && digit(s[j])) ++j;
                        if (j > i + 2 && j < s.size() && s[j] == u'}' && j + 1 <= s.size())
                            terminal[i] = terminal[j + 1];
                    }
                    if (terminal[i] == view::npos && dot(s[i + 1])) terminal[i] = terminal[i + 2];
                }
                if (terminal[i] == view::npos) terminal[i] = terminal[i + 1];
            }
            if (terminal[body] != view::npos) return terminal[body];
        }
    }
    return dot(s[start]) ? start + 1 : view::npos;
}
std::vector<std::u16string> splitFormat(view s) {
    std::vector<std::u16string> parts;
    std::size_t previous = 0;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] != u'%') continue;
        auto const end = captureEnd(s, i + 1);
        if (end == view::npos) continue;
        parts.emplace_back(s.substr(previous, i - previous));
        parts.emplace_back(s.substr(i + 1, end - i - 1));
        previous = end;
        i = end - 1;
    }
    parts.emplace_back(s.substr(previous));
    return parts;
}
bool pluralForm(view s) noexcept {
    auto const body = pluralBodyStart(s, 0);
    if (body == view::npos || body >= s.size() || s.back() != u'}') return false;
    for (auto i = body; i + 1 < s.size(); ++i)
        if (!dot(s[i])) return false;
    return true;
}

// Source parseInt(..., 10) only uses the leading decimal digits. Saturation is
// sufficient because any index beyond vector::size produces the same undefined
// array access, which Array.join turns into an empty string.
std::optional<std::size_t> argumentIndex(view digits, std::size_t argument_count) noexcept {
    std::size_t value = 0;
    bool any = false;
    for (auto c : digits) {
        if (!digit(c)) break;
        any = true;
        auto const d = static_cast<std::size_t>(c - u'0');
        if (value > argument_count || value > (std::numeric_limits<std::size_t>::max() - d) / 10)
            return std::nullopt;
        value = value * 10 + d;
    }
    if (!any || value == 0 || value > argument_count) return std::nullopt;
    return value - 1;
}
std::u16string processFormat(view format, std::span<std::u16string const> arguments) {
    auto parts = splitFormat(format);
    for (std::size_t i = 1; i < parts.size(); i += 2) {
        auto& part = parts[i];
        auto const c = part.empty() ? char16_t{} : part.front();
        if (digit(c)) {
            auto const index = argumentIndex(part, arguments.size());
            part = index ? arguments[*index] : std::u16string{};
        } else if (c == u'{') {
            view const remainder(part.data() + 1, part.size() - 1);
            // Match Source's whole-substring lexicographic comparisons, not a
            // stricter one-digit test. E.g. "{10}" passes this condition.
            if (remainder >= u"0" && remainder <= u"9") {
                auto const digits = part.size() >= 2
                    ? view(part).substr(1, part.size() - 2) : view{};
                auto const index = argumentIndex(digits, arguments.size());
                part = index ? arguments[*index] : std::u16string{};
            } else if (pluralForm(part)) {
                part.insert(part.begin(), u'%');
            }
        }
    }
    std::u16string result;
    std::size_t size = 0;
    for (auto const& part : parts) size += part.size();
    result.reserve(size);
    for (auto const& part : parts) result += part;
    return result;
}
std::string toUtf8ReplacingInvalidUtf16(view text) {
    std::string result;
    result.reserve(text.size());
    auto append = [&result](std::uint32_t cp) {
        if (cp <= 0x7F) result.push_back(static_cast<char>(cp));
        else if (cp <= 0x7FF) {
            result.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            result.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            result.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    };
    for (std::size_t i = 0; i < text.size(); ++i) {
        auto const first = static_cast<std::uint16_t>(text[i]);
        if (first >= 0xD800 && first <= 0xDBFF) {
            if (i + 1 < text.size()) {
                auto const second = static_cast<std::uint16_t>(text[i + 1]);
                if (second >= 0xDC00 && second <= 0xDFFF) {
                    append(0x10000 + ((first - 0xD800) << 10) + (second - 0xDC00));
                    ++i;
                    continue;
                }
            }
            append(0xFFFD);
        } else if (first >= 0xDC00 && first <= 0xDFFF) {
            append(0xFFFD);
        } else append(first);
    }
    return result;
}
}

TexError::TexError(std::string id, std::u16string format,
                   std::span<std::u16string const> arguments)
    : id_(std::move(id)), message_(processFormat(format, arguments)),
      what_(toUtf8ReplacingInvalidUtf16(message_)) {}
}
