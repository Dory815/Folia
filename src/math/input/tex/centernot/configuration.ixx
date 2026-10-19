// MathJax 4.1.3 input/tex/centernot/CenternotConfiguration.ts.
export module folia.math.input.tex.centernot.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct CenternotMethods {
    static Property initializeModule(ParserRuntime&);
    static FunctionRef filterCenterOver(ParserRuntime&);
};
struct CenternotConfiguration { static ConfigurationRef initializeModule(ParserRuntime&); };
}
