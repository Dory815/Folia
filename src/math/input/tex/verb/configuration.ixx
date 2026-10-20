// MathJax 4.1.3 input/tex/verb/VerbConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.verb.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
namespace VerbMethods {
DispatchOutcome Verb(TexParser &, MacroInvocation const &);
}
struct VerbConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
