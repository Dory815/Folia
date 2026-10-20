// MathJax 4.1.3 input/tex/cancel/CancelConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.cancel.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct CancelMethods {
    static DispatchOutcome Cancel(TexParser &, MacroInvocation const &);
    static DispatchOutcome CancelTo(TexParser &, MacroInvocation const &);
};
struct CancelConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
