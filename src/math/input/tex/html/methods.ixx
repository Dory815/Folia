// MathJax 4.1.3 input/tex/html/HtmlMethods.ts.
export module folia.math.input.tex.html.methods;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct HtmlMethods {
    static DispatchOutcome Data(TexParser &, MacroInvocation const &);
    static DispatchOutcome Href(TexParser &, MacroInvocation const &);
    static DispatchOutcome Class(TexParser &, MacroInvocation const &);
    static DispatchOutcome Style(TexParser &, MacroInvocation const &);
    static DispatchOutcome Id(TexParser &, MacroInvocation const &);
};
} // namespace folia::math::tex
