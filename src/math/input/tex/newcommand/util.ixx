// MathJax 4.1.3 input/tex/newcommand/NewcommandUtil.ts.
export module folia.math.input.tex.newcommand.util;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
inline constexpr std::u16string_view NEW_DELIMITER=u"new-Delimiter";
inline constexpr std::u16string_view NEW_COMMAND=u"new-Command";
inline constexpr std::u16string_view NEW_ENVIRONMENT=u"new-Environment";
inline constexpr double NEWCOMMAND_PRIORITY=-100;
struct NewcommandUtil {
    static std::u16string GetCSname(TexParser&,std::u16string);
    static std::u16string GetCsNameArgument(TexParser&,std::u16string);
    static Property GetArgCount(TexParser&,std::u16string);
    static std::variant<double,PropertyArray> GetTemplate(TexParser&,std::u16string,std::u16string);
    static std::u16string GetParameter(TexParser&,std::u16string,Property);
    static double MatchParam(TexParser&,std::u16string_view);
    static void checkProtectedMacros(TexParser&,std::u16string_view);
    static void addDelimiter(TexParser&,std::u16string,std::u16string,Property);
    static void addMacro(TexParser&,std::u16string,ParseMethod,Property,std::u16string token={});
    static void addEnvironment(TexParser&,std::u16string,ParseMethod,Property);
    static void undefineMacro(TexParser&,std::u16string);
    static void undefineDelimiter(TexParser&,std::u16string);
};
}
