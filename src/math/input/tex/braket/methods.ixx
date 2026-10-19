// MathJax 4.1.3 input/tex/braket/BraketMethods.ts.
export module folia.math.input.tex.braket.methods;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.braket.items;
import std;
export namespace folia::math::tex {
struct BraketMethodSet { FunctionRef braket, bar; Property macro; };
struct BraketMethods { static BraketMethodSet create(ParserRuntime&); };
}
