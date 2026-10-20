// MathJax 4.1.3 input/tex/begingroup/BegingroupMethods.ts; Apache-2.0.
export module folia.math.input.tex.begingroup.methods;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.begingroup.stack;
export import folia.math.input.tex.base_methods;
import std;

export namespace folia::math::tex {
struct BegingroupMethods {
    static DispatchOutcome begingroup(TexParser &, MacroInvocation const &);
    static DispatchOutcome endgroup(TexParser &, MacroInvocation const &);
    static DispatchOutcome reset(TexParser &, MacroInvocation const &);
    static DispatchOutcome sandbox(TexParser &, MacroInvocation const &);
    static DispatchOutcome global(TexParser &, MacroInvocation const &);
    static constexpr MacroHandler macro = &BaseMethods::Macro;
};
}
