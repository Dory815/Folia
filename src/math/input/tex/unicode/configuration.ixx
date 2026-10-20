// MathJax 4.1.3 input/tex/unicode/UnicodeConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.unicode.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct UnicodeMethods {
    static DispatchOutcome Unicode(TexParser &, MacroInvocation const &);
    static DispatchOutcome RawUnicode(TexParser &, MacroInvocation const &);
    static DispatchOutcome Char(TexParser &, MacroInvocation const &);
};
struct UnicodeConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
}
