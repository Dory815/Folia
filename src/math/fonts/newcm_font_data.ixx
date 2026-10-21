export module folia.math.fonts.newcm_font_data;
export import folia.math.output.common.font_data;
export import folia.math.output.svg.font_data;
import std;

export namespace folia::math::fonts {
// These records reside in read-only program data. Registering a table borrows
// its storage and never expands its glyph bodies into heap objects.
struct NewcmFontGlyphRow {
    char const* variant;
    std::uint32_t code;
    double metrics[7];
    bool path_defined;
    std::uint32_t composition[2];
    std::uint8_t composition_size;
    char const* path;
    char const* serialized_path;
    bool content_defined;
    char const* content;
};
struct NewcmSvgAliasRow {
    char const* from_variant;
    std::uint32_t from_codepoint;
    char const* to_variant;
    std::uint32_t to_codepoint;
};
struct NewcmGlyphData {
    std::optional<output::common::FontGlyph> glyph;
    std::optional<output::svg::SvgFontGlyphSource> svg;
};
class NewcmFontRequest final : public output::common::FontGlyphLookup,
                               public output::svg::SvgFontGlyphLookup {
    struct State;
    std::unique_ptr<State> state_;
public:
    NewcmFontRequest();
    ~NewcmFontRequest();
    NewcmFontRequest(NewcmFontRequest const&) = delete;
    NewcmFontRequest& operator=(NewcmFontRequest const&) = delete;
    void addRows(std::span<NewcmFontGlyphRow const> rows);
    void addAliases(std::span<NewcmSvgAliasRow const> rows);
    void setGlyphCount(std::size_t count);
    // One loading API: a single key is a batch of one. Returned views are valid
    // only for this request; completed Scene glyph resources own their values.
    std::vector<NewcmGlyphData const*> read(
        std::span<output::svg::SvgFontGlyphKey const> keys) const;
    std::vector<output::svg::SvgFontGlyphKey> keys() const;
    std::vector<std::pair<output::svg::SvgFontGlyphKey, output::svg::SvgFontGlyphKey>> aliases() const;
    std::size_t loadedGlyphCount() const noexcept;
    std::size_t loadedResourceCount() const noexcept;
    std::size_t glyphCount() const noexcept override;
    output::common::FontGlyph const* glyph(std::string_view variant, std::uint32_t codepoint) const override;
    output::svg::SvgFontGlyphSource const* find(output::svg::SvgFontGlyphKey const& key) const override;
};

// Installs metadata and one request-owned lookup over the pinned read-only
// records. Generated data is never evaluated as JavaScript.
void installNewcmFontData(output::common::FontRepositoryBuilder& builder,
                          output::svg::SvgFontResourcesBuilder& svgBuilder);
}
