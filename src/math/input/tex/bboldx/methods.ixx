// MathJax 4.1.3 input/tex/bboldx/BboldxMethods.ts.
export module folia.math.input.tex.bboldx.methods;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct BboldxMethodSet { Property table; };
struct BboldxMethods { static BboldxMethodSet initializeModule(ParserRuntime&); static Property get(ParserRuntime&,std::u16string_view); };
}
