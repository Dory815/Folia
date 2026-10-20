// MathJax 4.1.3 input/tex/mathtools/MathtoolsConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.mathtools.configuration;
export import folia.math.input.tex.mathtools.mappings;
export import folia.math.input.tex.mathtools.items;
export import folia.math.input.tex.mathtools.tags;
import std;
export namespace folia::math::tex {
void configMathtools(ParserConfiguration &, TeX &);
void fixPrescripts(TexFilterData &);
struct MathtoolsConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
