// MathJax 4.1.3 input/tex/amscd/AmsCdMethods.ts.
export module folia.math.input.tex.amscd.methods;
export import folia.math.input.tex.parser;
import folia.math.input.tex.ams.items;
import std;
export namespace folia::math::tex {
struct AmsCdMethodSet { FunctionRef cd, arrow, cell, minWidth, minHeight; };
struct AmsCdMethods {
    static Property initializeModule(ParserRuntime&);
    static AmsCdMethodSet create(ParserRuntime&);
};
}
