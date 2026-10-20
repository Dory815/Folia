// MathJax 4.1.3 input/tex/mathtools/MathtoolsTags.ts; Apache-2.0.
export module folia.math.input.tex.mathtools.tags;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class MathtoolsTags final : public TagForm {
  public:
    using Format = std::array<std::u16string, 3>;
    using Formats = std::unordered_map<std::u16string, Format>;
    explicit MathtoolsTags(Formats forms = {});
    Formats mtFormats;
    std::optional<Format> mtCurrent;
    std::optional<TagFormatValue> formatTag(std::u16string_view) const override;
};
void MathtoolsTagFormat(ParserConfiguration &, TeX &);
} // namespace folia::math::tex
