// MathJax 4.1.3 input/tex/enclose/EncloseConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.enclose.configuration;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.parse_util;
import std;
export namespace folia::math::tex {
extern std::array<KeyValueOption, 7> const ENCLOSE_OPTIONS;
struct EncloseMethods {
    static DispatchOutcome Enclose(TexParser &, MacroInvocation const &);
};
struct EncloseConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
