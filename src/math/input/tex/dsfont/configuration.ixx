// MathJax 4.1.3 input/tex/dsfont/DsfontConfiguration.ts.
export module folia.math.input.tex.dsfont.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct DsfontConfiguration {
    static DispatchOutcome ChooseFont(TexParser &, MacroInvocation const &);
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
