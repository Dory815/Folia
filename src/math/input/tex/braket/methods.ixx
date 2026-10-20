// MathJax 4.1.3 input/tex/braket/BraketMethods.ts; Apache-2.0.
export module folia.math.input.tex.braket.methods;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.braket.items;
export import folia.math.input.tex.base_methods;
import std;
export namespace folia::math::tex {
struct BraketMethods {
    static DispatchOutcome Braket(TexParser &, MacroInvocation const &);
    static DispatchOutcome Bar(TexParser &, MacroInvocation const &);
    static constexpr MacroHandler Macro = &BaseMethods::Macro;
};
} // namespace folia::math::tex
