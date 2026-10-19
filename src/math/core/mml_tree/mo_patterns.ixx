// Finite UTF-16 match sets for the five source-defined MmlMo patterns.
export module folia.math.core.mml_tree.mo_patterns;
import std;

export namespace folia::math::mml {
enum class MoPattern { pseudoScripts, primes, opPattern, mathaccents, mathaccentsWithWidth };
struct MoCodeUnitRange { char16_t first, last; };
struct MoPatternSpec {
    std::u16string_view source;
    std::string_view flags;
    std::span<MoCodeUnitRange const> units;
    std::size_t minimum;
    std::optional<std::size_t> maximum;
};
const MoPatternSpec& moPatternSpec(MoPattern);
bool matchMoPattern(MoPattern, std::u16string_view);
}
