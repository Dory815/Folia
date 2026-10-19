// MathJax 4.1.3 input/tex/bussproofs/BussproofsMethods.ts.
export module folia.math.input.tex.bussproofs.methods;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.bussproofs.items;
import std;
export namespace folia::math::tex {
struct BussproofsMethodSet { Property methods=Missing{}; };
struct BussproofsMethods { static BussproofsMethodSet create(ParserRuntime&); };
}
