// MathJax 4.1.3 input/tex/colortbl/ColortblConfiguration.ts: TableColor.
export module folia.math.input.tex.colortbl.methods;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct ColortblMethodSet { FunctionRef tableColor; };
struct ColortblMethods { static ColortblMethodSet create(ParserRuntime&); };
}
