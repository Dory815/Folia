// MathJax 4.1.3 input/tex/amscd/AmsCdMethods.ts; Apache-2.0.
export module folia.math.input.tex.amscd.methods;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct AmsCdMethods {
    static StackEntry CD(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static DispatchOutcome arrow(TexParser &, MacroInvocation const &);
    static DispatchOutcome cell(TexParser &, MacroInvocation const &);
    static DispatchOutcome minCDarrowwidth(TexParser &, MacroInvocation const &);
    static DispatchOutcome minCDarrowheight(TexParser &, MacroInvocation const &);
};
} // namespace folia::math::tex
