// MathJax 4.1.3 input/tex/color/ColorConfiguration.ts.
export module folia.math.input.tex.color.configuration;
export import folia.math.input.tex.parser;
export import folia.math.util.map_bindings;
import std;
export namespace folia::math::tex {
struct ColorConfiguration { static ConfigurationRef initializeModule(
    ParserRuntime&,std::shared_ptr<options::MapContextBindings>); };
}
