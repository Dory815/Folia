// MathJax 4.1.3 input/tex/boldsymbol/BoldsymbolConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.boldsymbol.configuration;
export import folia.math.input.tex.parser;
import std;

export namespace folia::math::tex {
mml::MmlNode& createBoldToken(NodeFactory&, mml::NodeKind, NodeDefinition, std::u16string_view);
void rewriteBoldTokens(std::span<mml::MmlNode* const>);
// These real package entry points await the native parser/configuration services.
void rewriteBoldTokens(TexFilterData&);
namespace BoldsymbolMethods {
    DispatchOutcome Boldsymbol(TexParser&, MacroInvocation const&);
}
struct BoldsymbolConfiguration {
    static std::shared_ptr<Configuration const> initializeModule(ConfigurationHandler&);
};
}
