// MathJax 4.1.3 input/tex/bboldx/BboldxMethods.ts.
export module folia.math.input.tex.bboldx.methods;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.base_methods;
import std;
export namespace folia::math::tex {
struct BboldxMethods {
    static constexpr MacroHandler Macro = &BaseMethods::Macro;
    static DispatchOutcome ChooseFont(TexParser &, MacroInvocation const &);
    static DispatchOutcome ChooseTextFont(TexParser &, MacroInvocation const &);
    static DispatchOutcome mathchar0miNormal(TexParser &, TokenInput const &);
    static DispatchOutcome delimiterNormal(TexParser &, TokenInput const &);
    static DispatchOutcome mathchar0miBold(TexParser &, TokenInput const &);
    static DispatchOutcome delimiterBold(TexParser &, TokenInput const &);
};
} // namespace folia::math::tex
