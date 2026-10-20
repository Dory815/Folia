// MathJax 4.1.3 input/tex/textmacros/TextMacrosMethods.ts; Apache-2.0.
export module folia.math.input.tex.textmacros.methods;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.base_methods;
import std;
export namespace folia::math::tex {
struct TextMacrosMethods {
    static DispatchOutcome Comment(TexParser &, MacroInvocation const &);
    static DispatchOutcome Math(TexParser &, MacroInvocation const &);
    static DispatchOutcome MathModeOnly(TexParser &, MacroInvocation const &);
    static DispatchOutcome Misplaced(TexParser &, MacroInvocation const &);
    static DispatchOutcome OpenBrace(TexParser &, MacroInvocation const &);
    static DispatchOutcome CloseBrace(TexParser &, MacroInvocation const &);
    static DispatchOutcome OpenQuote(TexParser &, MacroInvocation const &);
    static DispatchOutcome CloseQuote(TexParser &, MacroInvocation const &);
    static DispatchOutcome Tilde(TexParser &, MacroInvocation const &);
    static DispatchOutcome Space(TexParser &, MacroInvocation const &);
    static DispatchOutcome SelfQuote(TexParser &, MacroInvocation const &);
    static DispatchOutcome Insert(TexParser &, MacroInvocation const &);
    static DispatchOutcome Accent(TexParser &, MacroInvocation const &);
    static DispatchOutcome Emph(TexParser &, MacroInvocation const &);
    static DispatchOutcome TextFont(TexParser &, MacroInvocation const &);
    static DispatchOutcome SetFont(TexParser &, MacroInvocation const &);
    static DispatchOutcome SetSize(TexParser &, MacroInvocation const &);
    static DispatchOutcome CheckAutoload(TexParser &, MacroInvocation const &);
    static constexpr MacroHandler Macro = &BaseMethods::Macro;
    static constexpr MacroHandler Spacer = &BaseMethods::Spacer;
    static constexpr MacroHandler Hskip = &BaseMethods::Hskip;
    static constexpr MacroHandler rule = &BaseMethods::rule;
    static constexpr MacroHandler Rule = &BaseMethods::Rule;
    static constexpr MacroHandler HandleRef = &BaseMethods::HandleRef;
    static constexpr MacroHandler UnderOver = &BaseMethods::UnderOver;
    static constexpr MacroHandler Lap = &BaseMethods::Lap;
    static constexpr MacroHandler Phantom = &BaseMethods::Phantom;
    static constexpr MacroHandler Smash = &BaseMethods::Smash;
    static constexpr MacroHandler MmlToken = &BaseMethods::MmlToken;
};
} // namespace folia::math::tex
