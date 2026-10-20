// MathJax 4.1.3 input/tex/gensymb/GensymbConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.gensymb.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
DispatchOutcome mathcharUnit(TexParser &, TokenInput const &);
struct GensymbConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
