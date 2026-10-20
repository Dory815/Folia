// MathJax 4.1.3 input/tex/mathtools/MathtoolsUtil.ts; Apache-2.0.
export module folia.math.input.tex.mathtools.util;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct MathtoolsUtil {
    static void setDisplayLevel(mml::MmlNode &, std::u16string_view);
    static EqnArrayItem &checkAlignment(TexParser &, std::u16string_view);
    static void addPairedDelims(TexParser &, std::u16string, std::vector<TexArgument>);
    static void spreadLines(mml::MmlNode &, std::u16string_view);
    static std::u16string plusOrMinus(std::u16string_view, std::u16string);
    static mml::MmlNode &getScript(TexParser &, std::u16string_view, std::u16string_view);
};
} // namespace folia::math::tex
