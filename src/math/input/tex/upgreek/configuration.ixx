// MathJax 4.1.3 input/tex/upgreek/UpgreekConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.upgreek.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
DispatchOutcome mathchar0miNormal(TexParser &, TokenInput const &);
struct UpgreekConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
