// Source owner: MathJax 4.1.3 input/tex/cases/CasesConfiguration.ts.
export module folia.math.input.tex.cases.methods;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct CasesMethods {
    static Property initializeModule(ParserRuntime&);
    static ParseResult numCases(const FunctionReceiver&,ParserContext&,const ParseAtom&,ParseRest);
};
}
