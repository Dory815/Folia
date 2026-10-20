// MathJax 4.1.3 input/tex/newcommand/NewcommandConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.newcommand.configuration;
export import folia.math.input.tex.newcommand.mappings;
export import folia.math.input.tex.newcommand.items;
export namespace folia::math::tex {
void NewcommandConfig(ParserConfiguration &, TeX &);
struct NewcommandConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
