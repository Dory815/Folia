// Native C++23 contract for ParseUtil.addArgs, substituteArgs and checkMaxMacros.
// Source: MathJax 4.1.3 ts/input/tex/ParseUtil.ts (Apache-2.0).
export module folia.math.input.tex.parse_util;
import std;
export import folia.math.input.tex.tex_error;

export namespace folia::math::tex::parse_util {
struct ExpansionBudget final {
    std::size_t maxMacros;
    std::size_t maxBuffer;
    std::size_t macroCount = 0;
};

std::u16string addArgs(
    std::u16string_view left,
    std::u16string_view right,
    std::size_t maxBuffer);

std::u16string substituteArgs(
    std::span<std::u16string const> arguments,
    std::u16string_view replacement,
    std::size_t maxBuffer);

void checkMaxMacros(ExpansionBudget& budget, bool isMacro = true);
}
