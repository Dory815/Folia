// Native implementation of the three request-local ParseUtil operations.
// Source: MathJax 4.1.3 ts/input/tex/ParseUtil.ts (Apache-2.0).
module folia.math.input.tex.parse_util;
import std;
import folia.math.input.tex.tex_error;

namespace folia::math::tex::parse_util {
namespace {
bool startsWithAsciiLetter(std::u16string_view text) {
    if (text.empty()) return false;
    const auto first = text.front();
    return (first >= u'a' && first <= u'z') || (first >= u'A' && first <= u'Z');
}

bool endsWithControlWord(std::u16string_view text) {
    auto word_begin = text.size();
    while (word_begin > 0) {
        const auto character = text[word_begin - 1];
        if (!((character >= u'a' && character <= u'z') ||
              (character >= u'A' && character <= u'Z'))) {
            break;
        }
        --word_begin;
    }
    if (word_begin == text.size() || word_begin == 0 || text[word_begin - 1] != u'\\')
        return false;

    auto slash_begin = word_begin - 1;
    while (slash_begin > 0 && text[slash_begin - 1] == u'\\') --slash_begin;
    const auto slash_count = word_begin - slash_begin;
    return (slash_count % 2) == 1 &&
           (slash_begin == 0 || text[slash_begin - 1] != u'\\');
}

[[noreturn]] void throwTexError(std::string id, std::u16string message) {
    throw TexError(std::move(id), std::move(message));
}

[[noreturn]] void throwIllegalMacroParameter() {
    throwTexError("IllegalMacroParam", u"Illegal macro parameter reference");
}

[[noreturn]] void throwMaxBuffer() {
    throwTexError(
        "MaxBufferSize",
        u"MathJax internal buffer size exceeded; is there a recursive macro call?");
}
}

std::u16string addArgs(
    std::u16string_view left,
    std::u16string_view right,
    std::size_t maxBuffer) {
    const bool insert_space = startsWithAsciiLetter(right) && endsWithControlWord(left);

    if (left.size() > maxBuffer) throwMaxBuffer();
    auto combined_size = left.size();
    if (insert_space) {
        if (combined_size == std::numeric_limits<std::size_t>::max()) throwMaxBuffer();
        ++combined_size;
    }
    if (combined_size > maxBuffer || right.size() > maxBuffer - combined_size)
        throwMaxBuffer();

    std::u16string result;
    result.reserve(combined_size + right.size());
    result.append(left);
    if (insert_space) result.push_back(u' ');
    result.append(right);
    return result;
}

std::u16string substituteArgs(
    std::span<std::u16string const> arguments,
    std::u16string_view replacement,
    std::size_t maxBuffer) {
    std::u16string pending;
    std::u16string expanded;
    std::size_t i = 0;
    while (i < replacement.size()) {
        const auto character = replacement[i++];
        if (character == u'\\') {
            pending.push_back(character);
            if (i < replacement.size()) pending.push_back(replacement[i++]);
            continue;
        }
        if (character != u'#') {
            pending.push_back(character);
            continue;
        }

        if (i < replacement.size() && replacement[i] == u'#') {
            pending.push_back(replacement[i++]);
            continue;
        }
        if (i == replacement.size() || replacement[i] < u'1' || replacement[i] > u'9')
            throwIllegalMacroParameter();

        const auto argument_index = static_cast<std::size_t>(replacement[i++] - u'1');
        if (argument_index >= arguments.size()) throwIllegalMacroParameter();

        auto prefix = addArgs(expanded, pending, maxBuffer);
        expanded = addArgs(prefix, arguments[argument_index], maxBuffer);
        pending.clear();
    }
    return addArgs(expanded, pending, maxBuffer);
}

void checkMaxMacros(ExpansionBudget& budget, bool isMacro) {
    const bool counter_saturated =
        budget.macroCount == std::numeric_limits<std::size_t>::max();
    if (!counter_saturated) ++budget.macroCount;
    if (!counter_saturated && budget.macroCount <= budget.maxMacros) return;

    if (isMacro) {
        throwTexError(
            "MaxMacroSub1",
            u"MathJax maximum macro substitution count exceeded; is here a recursive macro call?");
    }
    throwTexError(
        "MaxMacroSub2",
        u"MathJax maximum substitution count exceeded; is there a recursive latex environment?");
}
}
