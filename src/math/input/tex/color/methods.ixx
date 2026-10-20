// MathJax 4.1.3 input/tex/color/ColorMethods.ts; Apache-2.0.
export module folia.math.input.tex.color.methods;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
NodeDefinition padding(std::u16string_view);
struct ColorMethods {
    static DispatchOutcome Color(TexParser &, MacroInvocation const &);
    static DispatchOutcome TextColor(TexParser &, MacroInvocation const &);
    static DispatchOutcome DefineColor(TexParser &, MacroInvocation const &);
    static DispatchOutcome ColorBox(TexParser &, MacroInvocation const &);
    static DispatchOutcome FColorBox(TexParser &, MacroInvocation const &);
};
} // namespace folia::math::tex
