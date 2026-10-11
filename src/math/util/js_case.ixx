// Root-frozen Unicode string primitive; no MathJax/parser/layout algorithms.
export module folia.math.util.js_case;
import std;
export namespace folia::math::js_case {
std::u16string lowerCase(std::u16string_view);
}
namespace folia::math::js_case::detail {
std::span<const char32_t> lowerMapping(char32_t);
unsigned caseProperties(char32_t);
}
