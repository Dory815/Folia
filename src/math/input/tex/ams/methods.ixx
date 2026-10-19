// MathJax 4.1.3 input/tex/ams/AmsMethods.ts.
export module folia.math.input.tex.ams.methods;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct AmsMethods { static Property initializeModule(ParserRuntime&); static Property get(ParserRuntime&,std::u16string_view); };
}
