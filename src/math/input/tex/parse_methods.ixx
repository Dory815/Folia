// MathJax 4.1.3 input/tex/ParseMethods.ts; Apache-2.0.
export module folia.math.input.tex.parse_methods;
export import folia.math.input.tex.parser;
import std;

export namespace folia::math::tex {
struct ParseMethods {
    static DispatchOutcome variable(TexParser &, TokenInput const &);
    static DispatchOutcome digit(TexParser &, TokenInput const &);
    static DispatchOutcome controlSequence(TexParser &, TokenInput const &);
    static DispatchOutcome lcGreek(TexParser &, TokenInput const &);
    static DispatchOutcome ucGreek(TexParser &, TokenInput const &);
    static DispatchOutcome mathchar0mi(TexParser &, TokenInput const &);
    static DispatchOutcome mathchar0mo(TexParser &, TokenInput const &);
    static DispatchOutcome mathchar7(TexParser &, TokenInput const &);
    static DispatchOutcome delimiter(TexParser &, TokenInput const &);
    static DispatchOutcome environment(TexParser &, std::u16string_view, EnvironmentBodyHandler,
                                       std::span<TexArgument const>);
};
} // namespace folia::math::tex
