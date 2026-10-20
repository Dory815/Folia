// MathJax 4.1.3 input/tex/action/ActionConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.action.configuration;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.base_methods;
import std;
export namespace folia::math::tex {
struct ActionMethods {
    static DispatchOutcome Toggle(TexParser &, MacroInvocation const &);
    static DispatchOutcome Mathtip(TexParser &, MacroInvocation const &);
    static constexpr MacroHandler Macro = &BaseMethods::Macro;
};
struct ActionConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
