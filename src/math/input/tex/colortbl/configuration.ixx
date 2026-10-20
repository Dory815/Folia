// MathJax 4.1.3 input/tex/colortbl/ColortblConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.colortbl.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct ColorData {
    std::u16string cell, row;
    std::vector<std::optional<std::u16string>> col;
};
class ColorArrayItem : public ArrayItem {
  public:
    using ArrayItem::ArrayItem;
    ColorData color;
    bool hasColor{};
    void EndEntry() override;
    void EndRow() override;
    mml::MmlNode &createMml() override;
};
DispatchOutcome TableColor(TexParser &, MacroInvocation const &);
void colortblConfig(ParserConfiguration &, TeX &);
struct ColortblConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
