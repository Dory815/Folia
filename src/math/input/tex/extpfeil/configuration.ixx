// MathJax 4.1.3 input/tex/extpfeil/ExtpfeilConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.extpfeil.configuration;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.ams.methods;
import std;
export namespace folia::math::tex {
namespace ExtpfeilMethods {
DispatchOutcome NewExtArrow(TexParser &, MacroInvocation const &);
inline constexpr MacroHandler xArrow = &AmsMethods::xArrow;
} // namespace ExtpfeilMethods
struct ExtpfeilConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
