// MathJax 4.1.3 input/tex/mathtools/MathtoolsUtil.ts.
export module folia.math.input.tex.mathtools.util;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct MathtoolsUtil {
    static Property moduleExports(ParserRuntime&);
    static void setDisplayLevel(mml::NodeValue,std::u16string_view);
    static EqnArrayItem& checkAlignment(TexParser&,std::u16string_view);
    static void addPairedDelims(TexParser&,std::u16string,options::Value);
    static void spreadLines(mml::NodeValue,std::u16string_view);
    static std::u16string plusOrMinus(std::u16string_view,std::u16string);
    static mml::NodeValue getScript(TexParser&,std::u16string_view,std::u16string_view);
};
}
