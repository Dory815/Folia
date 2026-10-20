// MathJax 4.1.3 input/tex/textcomp/TextcompMappings.ts.
export module folia.math.input.tex.textcomp.mappings;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct TextcompMappings {
    static DispatchOutcome Insert(TexParser &, MacroInvocation const &);
    static void initializeModule(MapHandler &);
};
} // namespace folia::math::tex
