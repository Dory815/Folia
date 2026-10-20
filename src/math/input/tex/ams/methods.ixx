// MathJax 4.1.3 input/tex/ams/AmsMethods.ts; Apache-2.0.
export module folia.math.input.tex.ams.methods;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.base_methods;
import std;
export namespace folia::math::tex {
struct AmsMethods {
    static StackEntry AmsEqnArray(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static StackEntry AlignAt(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static StackEntry Multline(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static StackEntry XalignAt(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static StackEntry FlalignArray(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static DispatchOutcome HandleDeclareOp(TexParser &, MacroInvocation const &);
    static DispatchOutcome HandleOperatorName(TexParser &, MacroInvocation const &);
    static DispatchOutcome SideSet(TexParser &, MacroInvocation const &);
    static DispatchOutcome operatorLetter(TexParser &, TokenInput const &);
    static DispatchOutcome MultiIntegral(TexParser &, MacroInvocation const &);
    static DispatchOutcome xArrow(TexParser &, MacroInvocation const &);
    static DispatchOutcome HandleShove(TexParser &, MacroInvocation const &);
    static DispatchOutcome CFrac(TexParser &, MacroInvocation const &);
    static DispatchOutcome Genfrac(TexParser &, MacroInvocation const &);
    static DispatchOutcome HandleTag(TexParser &, MacroInvocation const &);
    static constexpr MacroHandler HandleNoTag = &BaseMethods::HandleNoTag;
    static constexpr MacroHandler HandleRef = &BaseMethods::HandleRef;
    static constexpr MacroHandler Macro = &BaseMethods::Macro;
    static constexpr MacroHandler Accent = &BaseMethods::Accent;
    static constexpr MacroHandler Tilde = &BaseMethods::Tilde;
    static constexpr MacroHandler Spacer = &BaseMethods::Spacer;
    static constexpr MacroHandler NamedOp = &BaseMethods::NamedOp;
    static constexpr EnvironmentBodyHandler Array = &BaseMethods::Array;
    static constexpr EnvironmentBodyHandler EqnArray = &BaseMethods::EqnArray;
    static constexpr EnvironmentBodyHandler Equation = &BaseMethods::Equation;
};
} // namespace folia::math::tex
