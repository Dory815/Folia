// MathJax 4.1.3 input/tex/ParseMethods.ts; Apache-2.0.
export module folia.math.input.tex.base_parse_methods;
export import folia.math.input.tex.base_methods;
import std;
export namespace folia::math::tex {
struct ParseMethods {
    // One canonical callback table per loaded base-module instance.
    static Property initializeModule(ParserRuntime&);
};
}
