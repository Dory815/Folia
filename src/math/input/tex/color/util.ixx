// MathJax 4.1.3 input/tex/color/ColorUtil.ts; Apache-2.0.
export module folia.math.input.tex.color.util;
export import folia.math.input.tex.tex_error;
export import folia.math.input.tex.color.constants;
import std;

export namespace folia::math::tex {
class ColorModel {
    std::unordered_map<std::u16string,std::u16string> user_colors_;

    [[nodiscard]] std::u16string normalizeColor(
        std::u16string_view model, std::u16string_view definition) const;
    [[nodiscard]] std::u16string getColorByName(std::u16string_view name) const;
public:
    ColorModel() = default;
    ColorModel(ColorModel const&) = default;
    ColorModel(ColorModel&&) noexcept = default;
    ColorModel& operator=(ColorModel const&) = default;
    ColorModel& operator=(ColorModel&&) noexcept = default;

    [[nodiscard]] std::u16string getColor(
        std::u16string_view model, std::u16string_view definition) const;
    void defineColor(std::u16string_view model, std::u16string_view name,
                     std::u16string_view definition);
};
}
