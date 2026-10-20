// MathJax 4.1.3 input/tex/mathtools/MathtoolsMethods.ts; Apache-2.0.
export module folia.math.input.tex.mathtools.methods;
export import folia.math.input.tex.ams.methods;
export import folia.math.input.tex.newcommand.methods;
import std;
export namespace folia::math::tex {
struct MathtoolsMethods {
    static constexpr double LEGACYPRIORITY = 4;
    static const std::array<HandlerRegistration, 1> LEGACYCONFIG;
    static StackEntry MtMatrix(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static StackEntry MtSmallMatrix(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static StackEntry MtMultlined(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static StackEntry SpreadLines(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static StackEntry Cases(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static DispatchOutcome HandleShove(TexParser &, MacroInvocation const &);
    static DispatchOutcome MathLap(TexParser &, MacroInvocation const &);
    static DispatchOutcome Cramped(TexParser &, MacroInvocation const &);
    static DispatchOutcome MtLap(TexParser &, MacroInvocation const &);
    static DispatchOutcome MathMakeBox(TexParser &, MacroInvocation const &);
    static DispatchOutcome MathMBox(TexParser &, MacroInvocation const &);
    static DispatchOutcome UnderOverBracket(TexParser &, MacroInvocation const &);
    static DispatchOutcome Aboxed(TexParser &, MacroInvocation const &);
    static DispatchOutcome MakeAboxedCommand(TexParser &, MacroInvocation const &);
    static DispatchOutcome ArrowBetweenLines(TexParser &, MacroInvocation const &);
    static DispatchOutcome VDotsWithin(TexParser &, MacroInvocation const &);
    static DispatchOutcome ShortVDotsWithin(TexParser &, MacroInvocation const &);
    static DispatchOutcome FlushSpaceAbove(TexParser &, MacroInvocation const &);
    static DispatchOutcome FlushSpaceBelow(TexParser &, MacroInvocation const &);
    static DispatchOutcome PairedDelimiters(TexParser &, MacroInvocation const &);
    static DispatchOutcome DeclarePairedDelimiter(TexParser &, MacroInvocation const &);
    static DispatchOutcome DeclarePairedDelimiterX(TexParser &, MacroInvocation const &);
    static DispatchOutcome DeclarePairedDelimiterXPP(TexParser &, MacroInvocation const &);
    static DispatchOutcome CenterColon(TexParser &, MacroInvocation const &);
    static DispatchOutcome Relation(TexParser &, MacroInvocation const &);
    static DispatchOutcome NArrow(TexParser &, MacroInvocation const &);
    static DispatchOutcome SplitFrac(TexParser &, MacroInvocation const &);
    static DispatchOutcome XMathStrut(TexParser &, MacroInvocation const &);
    static DispatchOutcome Prescript(TexParser &, MacroInvocation const &);
    static DispatchOutcome NewTagForm(TexParser &, MacroInvocation const &);
    static DispatchOutcome UseTagForm(TexParser &, MacroInvocation const &);
    static DispatchOutcome SetOptions(TexParser &, MacroInvocation const &);
    static constexpr EnvironmentBodyHandler Array = &BaseMethods::Array;
    static constexpr MacroHandler Macro = &BaseMethods::Macro;
    static constexpr MacroHandler xArrow = &AmsMethods::xArrow;
    static constexpr MacroHandler HandleRef = AmsMethods::HandleRef;
    static constexpr EnvironmentBodyHandler AmsEqnArray = &AmsMethods::AmsEqnArray;
    static constexpr MacroHandler MacroWithTemplate = &NewcommandMethods::MacroWithTemplate;
};
} // namespace folia::math::tex
