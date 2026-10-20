// MathJax 4.1.3 input/tex/empheq/EmpheqConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.empheq.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct EmpheqMethods {
    static StackEntry Empheq(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static DispatchOutcome EmpheqMO(TexParser &, MacroInvocation const &);
    static DispatchOutcome EmpheqDelim(TexParser &, MacroInvocation const &);
};
struct EmpheqConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
