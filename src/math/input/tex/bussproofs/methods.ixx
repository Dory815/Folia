// MathJax 4.1.3 input/tex/bussproofs/BussproofsMethods.ts; Apache-2.0.
export module folia.math.input.tex.bussproofs.methods;
export import folia.math.input.tex.bussproofs.items;
import std;
export namespace folia::math::tex {
struct BussproofsMethods {
    static StackEntry Prooftree(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static DispatchOutcome Axiom(TexParser &, MacroInvocation const &);
    static DispatchOutcome Inference(TexParser &, MacroInvocation const &);
    static DispatchOutcome Label(TexParser &, MacroInvocation const &);
    static DispatchOutcome SetLine(TexParser &, MacroInvocation const &);
    static DispatchOutcome RootAtTop(TexParser &, MacroInvocation const &);
    static DispatchOutcome AxiomF(TexParser &, MacroInvocation const &);
    static DispatchOutcome FCenter(TexParser &, MacroInvocation const &);
    static DispatchOutcome InferenceF(TexParser &, MacroInvocation const &);
};
} // namespace folia::math::tex
