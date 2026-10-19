// MathJax 4.1.3 input/tex/mathtools/MathtoolsMethods.ts.
export module folia.math.input.tex.mathtools.methods;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct MathtoolsMethods {
    static Property initializeModule(ParserRuntime&);
    static constexpr double LEGACYPRIORITY=4;
};
}
