// MathJax 4.1.3 Token.ts::Token. Apache-2.0.
export module folia.math.input.tex.token;
export import folia.math.core.mml_tree.attributes;
import std;
export namespace folia::math::tex {
struct Token final {
    std::u16string token;
    std::optional<std::u16string> character;
    std::optional<mml::AttributeSet> attributes;
    Token(std::u16string, std::optional<std::u16string>,
          std::optional<mml::AttributeSet> = std::nullopt);
};
}
