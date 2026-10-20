// MathJax 4.1.3 input/tex/bbox/BboxConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.bbox.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
namespace BboxMethods {
DispatchOutcome BBox(TexParser &, MacroInvocation const &);
}
struct BboxConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
