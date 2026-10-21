export module folia.math.fonts.newcm_font_data;
export import folia.math.output.common.font_data;
export import folia.math.output.svg.font_data;
import std;

export namespace folia::math::fonts {
// These records reside in read-only program data. Registering a table borrows
// its storage and never expands its glyph bodies into heap objects.
struct NewcmFontGlyphRow {
    char const *variant;
    std::uint32_t code;
    double metrics[7];
    bool path_defined;
    std::uint32_t composition[2];
    std::uint8_t composition_size;
    char const *path;
    char const *serialized_path;
    bool content_defined;
    char const *content;
};
struct NewcmSvgAliasRow {
    char const *from_variant;
    std::uint32_t from_codepoint;
    char const *to_variant;
    std::uint32_t to_codepoint;
};
struct NewcmGlyphRecord {
    NewcmFontGlyphRow const *glyph{};
    NewcmFontGlyphRow const *svg{};
};
// Immutable borrowed records/indexes; no glyph body or outline is constructed.
class NewcmFontTables final {
    struct State;
    std::unique_ptr<State> state_;

  public:
    NewcmFontTables();
    ~NewcmFontTables();
    void addRows(std::span<NewcmFontGlyphRow const> rows);
    void addAliases(std::span<NewcmSvgAliasRow const> rows);
    void setGlyphCount(std::size_t count);
    std::size_t glyphCount() const noexcept;
    bool contains(output::svg::SvgFontGlyphKey const &key) const;
    std::vector<NewcmGlyphRecord> lookup(std::span<output::svg::SvgFontGlyphKey const> keys) const;
    std::vector<output::svg::SvgFontGlyphKey> keys() const;
    std::vector<std::pair<output::svg::SvgFontGlyphKey, output::svg::SvgFontGlyphKey>>
    aliases() const;
};
output::common::FontFaceData installNewcmFontData(NewcmFontTables &tables);
} // namespace folia::math::fonts
