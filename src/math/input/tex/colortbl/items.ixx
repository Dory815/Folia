// MathJax 4.1.3 input/tex/colortbl/ColortblConfiguration.ts: ColorArrayItem.
export module folia.math.input.tex.colortbl.items;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class ColorArrayItem final : public ArrayItem {
public:
    struct ColorData { std::u16string cell,row; std::vector<std::optional<std::u16string>> col; };
    ColorData color;
    bool hasColor=false;
    std::string kind()const override;
    void EndEntry()override;
    void EndRow()override;
    mml::NodeValue createMml()override;
    static tree::FactoryClassRef<BaseItem> initializeModule(ParserRuntime&);
};
}
