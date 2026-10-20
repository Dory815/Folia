// MathJax 4.1.3 input/tex/bbm/BbmConfiguration.ts.
export module folia.math.input.tex.bbm.configuration;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.base_methods;
import std;
export namespace folia::math::tex {
struct BbmMethods {
    static DispatchOutcome ChooseFont(TexParser &, MacroInvocation const &);
    static DispatchOutcome ChangeBold(TexParser &, MacroInvocation const &);
    static constexpr MacroHandler MathFont = &BaseMethods::MathFont;
};
struct BbmConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
