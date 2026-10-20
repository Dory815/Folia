// MathJax 4.1.3 input/tex/boldsymbol/BoldsymbolConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.boldsymbol.configuration;
export import folia.math.input.tex.parser;
import std;

export namespace folia::math::tex {
mml::MmlNode &createBoldToken(NodeFactory &, mml::NodeKind, NodeDefinition,
                              std::optional<std::u16string_view>);
void rewriteBoldTokens(std::span<mml::MmlNode *const>);
void rewriteBoldTokens(TexFilterData &);
namespace BoldsymbolMethods {
DispatchOutcome Boldsymbol(TexParser &, MacroInvocation const &);
}
struct BoldsymbolConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
