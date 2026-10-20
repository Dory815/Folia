// MathJax 4.1.3 input/tex/units/UnitsConfiguration.ts.
export module folia.math.input.tex.units.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct UnitsMethods {
    static DispatchOutcome Unit(TexParser &, MacroInvocation const &);
    static DispatchOutcome UnitFrac(TexParser &, MacroInvocation const &);
    static DispatchOutcome NiceFrac(TexParser &, MacroInvocation const &);
};
struct UnitsConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
