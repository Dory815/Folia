// MathJax 4.1.3 input/tex/newcommand/NewcommandMethods.ts; Apache-2.0.
export module folia.math.input.tex.newcommand.methods;
export import folia.math.input.tex.base_methods;
import std;
export namespace folia::math::tex {
struct NewcommandMethods {
    static DispatchOutcome NewCommand(TexParser &, MacroInvocation const &);
    static DispatchOutcome NewEnvironment(TexParser &, MacroInvocation const &);
    static DispatchOutcome MacroDef(TexParser &, MacroInvocation const &);
    static DispatchOutcome Let(TexParser &, MacroInvocation const &);
    static DispatchOutcome MacroWithTemplate(TexParser &, MacroInvocation const &);
    static StackEntry BeginEnv(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static constexpr MacroHandler Macro = &BaseMethods::Macro;
};
} // namespace folia::math::tex
