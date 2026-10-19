// MathJax 4.1.3 input/tex/newcommand/NewcommandConfiguration.ts.
export module folia.math.input.tex.newcommand.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct NewcommandConfiguration { static void initializeModule(ParserRuntime&); };
void NewcommandConfig(ParserConfiguration&,TeX&);
}
