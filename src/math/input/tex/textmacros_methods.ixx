// MathJax 4.1.3 input/tex/textmacros/TextMacrosMethods.ts.
// Native text-mode parser methods; canonical base-package aliases are shared.
export module folia.math.input.tex.textmacros.methods;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct TextMacrosMethods {
    static Property initializeModule(ParserRuntime&);
    static Property get(ParserRuntime&,std::u16string_view);
};
}
