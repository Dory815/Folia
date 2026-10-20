// MathJax 4.1.3 input/tex/empheq/EmpheqUtil.ts; Apache-2.0.
export module folia.math.input.tex.empheq.util;
export import folia.math.input.tex.parse_util;
import std;
export namespace folia::math::tex {
struct EmpheqUtil {
    static mml::AttributeSet
    splitOptions(std::u16string_view,
                 std::optional<std::span<KeyValueOption const>> allowed = std::nullopt);
    static std::size_t columnCount(mml::MmlNode const &);
    static mml::MmlNode &cellBlock(std::u16string_view, mml::MmlNode &, TexParser &,
                                   std::u16string_view env = {});
    static mml::MmlNode &topRowTable(mml::MmlNode const &, TexParser &);
    static void rowspanCell(mml::MmlNode &, std::u16string_view, mml::MmlNode const &, TexParser &,
                            std::u16string_view env = {});
    static void left(mml::MmlNode &, mml::MmlNode const &, std::u16string_view, TexParser &,
                     std::u16string_view env = {});
    static void right(mml::MmlNode &, mml::MmlNode const &, std::u16string_view, TexParser &,
                      std::u16string_view env = {});
    static void adjustTable(BeginItem &, TexParser &);
    inline static constexpr std::array<std::u16string_view, 6> allowEnv{
        u"equation", u"align", u"gather", u"flalign", u"alignat", u"multline"};
    static bool checkEnv(std::u16string_view);
};
} // namespace folia::math::tex
