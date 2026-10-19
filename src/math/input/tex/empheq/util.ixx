// MathJax 4.1.3 input/tex/empheq/EmpheqUtil.ts.
export module folia.math.input.tex.empheq.util;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct EmpheqUtil {
    static options::Value initializeModule(ParserRuntime&);
    static options::Value call(ParserRuntime&,std::u16string_view,std::span<const options::Value>);
    static mml::NodeValue cellBlock(std::u16string, mml::NodeValue, TexParser&, std::u16string);
    static mml::NodeValue topRowTable(mml::NodeValue, TexParser&);
    static void rowspanCell(mml::NodeValue, std::u16string, mml::NodeValue, TexParser&, std::u16string);
    static void left(mml::NodeValue, mml::NodeValue, std::u16string, TexParser&, std::u16string={});
    static void right(mml::NodeValue, mml::NodeValue, std::u16string, TexParser&, std::u16string={});
    static void adjustTable(BaseItem&,TexParser&);
    static Property splitOptions(ParserRuntime&,std::u16string,options::Value allowed=options::Null{});
    static bool checkEnv(ParserRuntime&,std::u16string);
};
}
