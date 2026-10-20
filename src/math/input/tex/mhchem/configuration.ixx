// MathJax 4.1.3 input/tex/mhchem/MhchemConfiguration.ts.
export module folia.math.input.tex.mhchem.configuration;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.base_methods;
export import folia.math.input.tex.ams.methods;
export import folia.math.input.tex.node_util;
import std;

export namespace folia::math::tex {
enum class MhchemReplacementKind { text, arrow };
struct MhchemReplacement {
    std::u16string_view pattern;
    std::u16string_view replacement;
    MhchemReplacementKind kind;
};

std::span<MhchemReplacement const> mhchemReplacements() noexcept;
std::u16string applyMhchemReplacement(std::u16string, MhchemReplacement const &,
                                      TokenMap const &macros, TokenMap const &characters);
std::u16string applyMhchemReplacements(std::u16string, TokenMap const &macros,
                                       TokenMap const &characters);

struct MhchemConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
struct MhchemMethods {
    static DispatchOutcome Machine(TexParser &, MacroInvocation const &);
    static constexpr MacroHandler Macro = &BaseMethods::Macro;
    static constexpr MacroHandler xArrow = &AmsMethods::xArrow;
};
struct MhchemUtils {
    static DispatchOutcome relmo(TexParser &, TokenInput const &);
};
} // namespace folia::math::tex
