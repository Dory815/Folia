// MathJax 4.1.3 input/tex/newcommand/NewcommandUtil.ts; Apache-2.0.
export module folia.math.input.tex.newcommand.util;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
inline constexpr std::u16string_view NEW_DELIMITER = u"new-Delimiter";
inline constexpr std::u16string_view NEW_COMMAND = u"new-Command";
inline constexpr std::u16string_view NEW_ENVIRONMENT = u"new-Environment";
inline constexpr double NEWCOMMAND_PRIORITY = -100;
struct NewcommandUtil {
    static std::u16string GetCSname(TexParser &, std::u16string_view);
    static std::u16string GetCsNameArgument(TexParser &, std::u16string_view);
    static std::optional<std::u16string> GetArgCount(TexParser &, std::u16string_view);
    static MacroDefinition GetTemplate(TexParser &, std::u16string_view, std::u16string_view);
    static std::u16string GetParameter(TexParser &, std::u16string_view,
                                       std::optional<std::u16string_view>);
    static bool MatchParam(TexParser &, std::u16string_view);
    static DefinitionLayer &checkGlobal(TexParser &, std::span<DefinitionKey const>);
    static void checkProtectedMacros(TexParser &, std::u16string_view);
    static void addDelimiter(TexParser &, std::u16string, std::optional<std::u16string>,
                             std::optional<mml::AttributeSet>);
    static void addMacro(TexParser &, std::u16string, CommandDefinition);
    static void addEnvironment(TexParser &, std::u16string, EnvironmentDefinition);
    static void undefineMacro(TexParser &, std::u16string_view);
    static void undefineDelimiter(TexParser &, std::u16string_view);
};
} // namespace folia::math::tex
