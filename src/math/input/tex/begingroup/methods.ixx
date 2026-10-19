// MathJax 4.1.3 input/tex/begingroup/BegingroupMethods.ts.
export module folia.math.input.tex.begingroup.methods;
export import folia.math.input.tex.begingroup.stack;
import std;
export namespace folia::math::tex {
struct BegingroupMethods {
    static Property initializeModule(ParserRuntime&);
    static Property get(ParserRuntime&,std::u16string_view);
};
}
