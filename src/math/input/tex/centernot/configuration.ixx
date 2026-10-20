// MathJax 4.1.3 input/tex/centernot/CenternotConfiguration.ts.
export module folia.math.input.tex.centernot.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
DispatchOutcome CenterOver(TexParser &, MacroInvocation const &);
void filterCenterOver(TexFilterData &);
struct CenternotConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
