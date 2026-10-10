// Root-frozen native service for the five no-flags UTF-16 MmlMo patterns.
// MathJax 4.1.3 mo.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mo_patterns;
export import folia.math.core.mml_tree.leaf_host;
import std;
export namespace folia::math::mml {
enum class MoPattern { pseudoScripts, primes, opPattern, mathaccents, mathaccentsWithWidth };
struct MoCodeUnitRange { char16_t first,last; };
struct MoPatternSpec {
    std::u16string_view source;
    std::string_view flags;
    std::span<const MoCodeUnitRange> units;
    std::size_t minimum;
    std::optional<std::size_t> maximum;
};
// Source strings and class ranges are root-generated immutable data. They are
// not a second semantic node model or a general JavaScript regex interpreter.
const MoPatternSpec& moPatternSpec(MoPattern);
bool matchMoPattern(MoPattern,std::u16string_view);
Pattern makeMoPattern(MoPattern);
}
