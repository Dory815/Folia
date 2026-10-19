// MathJax 4.1.3 TexParser lexical methods; Apache-2.0 Source semantics.
module folia.math.input.tex.tex_scanner;
import std;
import folia.math.input.tex.tex_error;

namespace folia::math::tex {
namespace {
bool isAsciiLetter(char16_t c) noexcept {
    return (c >= u'a' && c <= u'z') || (c >= u'A' && c <= u'Z');
}
bool isLineTerminator(char16_t c) noexcept {
    return c == u'\n' || c == u'\r' || c == 0x2028 || c == 0x2029;
}
bool isEcmaWhitespace(char16_t c) noexcept {
    return (c >= 0x0009 && c <= 0x000D) || c == 0x0020 || c == 0x00A0 ||
           c == 0x1680 || (c >= 0x2000 && c <= 0x200A) || c == 0x2028 ||
           c == 0x2029 || c == 0x202F || c == 0x205F || c == 0x3000 || c == 0xFEFF;
}
}

TexScanner::TexScanner(std::u16string source) : source_(std::move(source)) {}
std::u16string_view TexScanner::source() const noexcept { return source_; }
std::size_t TexScanner::position() const noexcept { return position_; }
void TexScanner::setPosition(std::size_t position) noexcept { position_ = position; }
void TexScanner::replaceSource(std::u16string source) { source_ = std::move(source); }
std::u16string_view TexScanner::currentControlSequence() const noexcept { return currentControlSequence_; }
void TexScanner::setCurrentControlSequence(std::u16string value) { currentControlSequence_ = std::move(value); }

[[noreturn]] void TexScanner::fail(std::string id, std::u16string format,
                                   std::span<std::u16string const> arguments) const {
    throw TexError(std::move(id), std::move(format), arguments);
}

std::u16string TexScanner::getCodePoint() const {
    if (position_ >= source_.size()) return {};
    const auto first = static_cast<std::uint32_t>(source_[position_]);
    if (first >= 0xD800 && first <= 0xDBFF && position_ + 1 < source_.size()) {
        const auto second = static_cast<std::uint32_t>(source_[position_ + 1]);
        if (second >= 0xDC00 && second <= 0xDFFF)
            return source_.substr(position_, 2);
    }
    return source_.substr(position_, 1);
}

bool TexScanner::nextIsSpace() const noexcept {
    return position_ < source_.size() && isEcmaWhitespace(source_[position_]);
}

std::u16string TexScanner::getNext() {
    while (nextIsSpace()) ++position_;
    return getCodePoint();
}

std::u16string TexScanner::getCS() {
    if (position_ >= source_.size()) {
        ++position_;
        return u" ";
    }
    const auto start = position_;
    const auto first = source_[position_];
    if (isAsciiLetter(first)) {
        while (position_ < source_.size() && isAsciiLetter(source_[position_])) ++position_;
        const auto lettersEnd = position_;
        if (position_ < source_.size() && source_[position_] == u' ') ++position_;
        return source_.substr(start, lettersEnd - start);
    }
    if (first >= 0xD800 && first <= 0xDBFF && position_ + 1 < source_.size() &&
        !isLineTerminator(source_[position_ + 1])) {
        position_ += 2;
        return source_.substr(start, 2);
    }
    if (!isLineTerminator(first)) {
        ++position_;
        return source_.substr(start, 1);
    }
    ++position_;
    return u" ";
}

std::optional<std::u16string> TexScanner::getArgument(std::u16string_view, bool noneOK) {
    const auto next = getNext();
    if (next.empty()) {
        if (noneOK) return std::nullopt;
        fail("MissingArgFor", u"Missing argument for %1", std::array<std::u16string,1>{currentControlSequence_});
    }
    if (next == u"}") {
        if (noneOK) return std::nullopt;
        fail("ExtraCloseMissingOpen", u"Extra close brace or missing open brace");
    }
    if (next == u"\\") {
        ++position_;
        return u"\\" + getCS();
    }
    if (next == u"{") {
        const auto begin = ++position_;
        std::size_t depth = 1;
        while (position_ < source_.size()) {
            const auto c = source_[position_++];
            if (c == u'\\') {
                ++position_;
            } else if (c == u'{') {
                ++depth;
            } else if (c == u'}' && --depth == 0) {
                return source_.substr(begin, position_ - begin - 1);
            }
        }
        fail("MissingCloseBrace", u"Missing close brace");
    }
    auto result = getCodePoint();
    position_ += result.size();
    return result;
}

std::optional<std::u16string> TexScanner::getBrackets(
    std::u16string_view, std::optional<std::u16string> fallback, bool matchBrackets) {
    if (getNext() != u"[") return fallback;
    const auto begin = ++position_;
    std::size_t braces = 0, brackets = 0;
    while (position_ < source_.size()) {
        const auto c = source_[position_++];
        switch (c) {
        case u'{': ++braces; break;
        case u'\\': ++position_; break;
        case u'}':
            if (braces == 0) {
                const std::array<std::u16string,1> args{u"']'"};
                fail("ExtraCloseLooking", u"Extra close brace while looking for %1", args);
            }
            --braces;
            break;
        case u'[': if (braces == 0) ++brackets; break;
        case u']':
            if (braces == 0) {
                if (!matchBrackets || brackets == 0) return source_.substr(begin, position_ - begin - 1);
                --brackets;
            }
            break;
        }
    }
    fail("MissingCloseBracket", u"Could not find closing ']' for argument to %1",
         std::array<std::u16string,1>{currentControlSequence_});
}

std::u16string TexScanner::getUpTo(std::u16string_view, std::u16string_view token) {
    while (nextIsSpace()) ++position_;
    const auto begin = position_;
    std::size_t braces = 0;
    while (position_ < source_.size()) {
        const auto beforeSpace = position_;
        auto c = getNext();
        position_ += c.size();
        if (c == u"\\") c += getCS();
        else if (c == u"{") ++braces;
        else if (c == u"}") {
            if (braces == 0)
                fail("ExtraCloseLooking", u"Extra close brace while looking for %1",
                     std::array<std::u16string,1>{std::u16string(token)});
            --braces;
        }
        if (braces == 0 && c == token) return source_.substr(begin, beforeSpace - begin);
    }
    fail("TokenNotFoundForCommand", u"Could not find %1 for %2",
         std::array<std::u16string,2>{std::u16string(token), currentControlSequence_});
}

bool TexScanner::getStar() {
    if (getNext() == u"*") { ++position_; return true; }
    return false;
}
}
