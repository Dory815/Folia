// Candidate immutable typed font repository contract.
export module folia.math.output.common.font_data;
import std;
import folia.math.output.scene.types;
export import folia.math.output.common.font_path;

export namespace folia::math::output::common {
struct FontGlyphMetrics {
    double h{}, d{}, w{}, ic{}, sk{}, dx{}, oc{};
};
struct FontGlyph {
    FontGlyphMetrics metrics;
    bool path_defined{};
    // Source geometry may be a valid prefix when its path data ends in an
    // invalid segment. SVG retains original bytes; this diagnostic describes
    // only the typed geometry prefix and does not affect glyph selection.
    std::optional<FontOutlineError> outline_error;
    // Source SvgFontData `c` recipes are typed Unicode sequences. Placement
    // expands them through the same selected variant; no markup is retained.
    std::vector<std::uint32_t> composition;
    std::shared_ptr<output::scene::GlyphResource const> resource;
};
struct GlyphReference {
    std::string variant;
    std::uint32_t codepoint{};
    friend bool operator==(GlyphReference const&, GlyphReference const&) = default;
};
struct ResolvedFontGlyph {
    FontGlyph const* data{};
    output::scene::GlyphKey key;
};
struct FontVariant {
    std::string name;
    // Exact Source SvgFontData variant.cacheID, used by writer cache identity.
    std::string cache_id;
    // Source createVariant builds a link's OWN chars as a shallow overlay on
    // the requested variant's inherit chain. Link fallback is never recursive.
    std::optional<std::string> link_overlay_variant;
    std::optional<std::string> inherit_variant;
    std::map<std::uint32_t, FontGlyph> glyphs;
    // Source's synthetic SMP entries identify one exact target variant/key.
    std::map<std::uint32_t, GlyphReference> remap;
};
enum class StretchDirection : std::uint8_t { none, horizontal, vertical };
struct DelimiterRecipe {
    StretchDirection direction{StretchDirection::none};
    std::vector<double> sizes;
    // Source semantics: variants and stretchv index size/stretch variants;
    // schar and stretch are Unicode glyph ids; c is the equivalent character.
    std::vector<std::uint32_t> variants, schar, stretch, stretchv;
    std::optional<double> ext, min;
    std::optional<std::uint32_t> c;
    std::optional<std::array<double, 2>> heightDepth;
    std::optional<std::array<double, 3>> heightDepthWidth;
    std::optional<std::array<double, 2>> fullExtent;
};
struct FontParameters {
    std::map<std::string, double, std::less<>> values;
    double required(std::string_view name) const {
        const auto it = values.find(name);
        if (it == values.end()) throw std::out_of_range("required font parameter is absent");
        return it->second;
    }
    std::optional<double> find(std::string_view name) const noexcept {
        const auto it = values.find(name);
        return it == values.end() ? std::nullopt : std::optional<double>{it->second};
    }
};
struct FontFaceData {
    std::string name;
    // Source profile keys exclude separately registered extension variants.
    std::size_t source_profile_glyph_count{};
    std::size_t source_dynamic_module_count{};
    std::size_t source_provider_trace_events{};
    std::map<std::string, std::size_t, std::less<>> extension_glyph_counts;
    FontParameters parameters;
    std::vector<std::string> size_variants;
    std::vector<std::string> stretch_variants;
    std::map<std::string, FontVariant, std::less<>> variants;
    std::map<std::uint32_t, DelimiterRecipe> delimiters;
    std::map<std::uint32_t, std::u16string> operatorRemap;
    std::map<std::uint32_t, std::u16string> accentRemap;
    std::map<std::uint32_t, std::u16string> numberRemap;
    std::size_t glyphCount() const noexcept;
    std::size_t sourceProfileGlyphCount() const noexcept { return source_profile_glyph_count; }
};

class FontRepository {
    std::map<std::string, std::shared_ptr<FontFaceData const>, std::less<>> faces_;
    explicit FontRepository(decltype(faces_) faces) : faces_(std::move(faces)) {}
    friend class FontRepositoryBuilder;
public:
    FontRepository() = default;
    FontFaceData const& face(std::string_view name) const;
    FontGlyph const* glyph(std::string_view face, std::string_view variant,
                           std::uint32_t codepoint) const noexcept;
    std::optional<ResolvedFontGlyph> resolveGlyph(std::string_view face,
                           std::string_view variant, std::uint32_t codepoint) const;
    std::optional<GlyphReference> remap(std::string_view face,
                           std::string_view variant, std::uint32_t codepoint) const;
    DelimiterRecipe const* delimiter(std::string_view face,
                           std::uint32_t codepoint) const noexcept;
    std::size_t faceCount() const noexcept { return faces_.size(); }
    std::size_t glyphCount() const noexcept;
};

class FontRepositoryBuilder {
    std::map<std::string, std::shared_ptr<FontFaceData>, std::less<>> faces_;
public:
    void addFace(FontFaceData face);
    void addGlyph(std::string_view face, std::string_view variant,
                  std::uint32_t codepoint, FontGlyph glyph);
    void setVariantInheritance(std::string_view face, std::string_view variant,
                               std::optional<std::string> inherit,
                               std::optional<std::string> linkOverlay = {});
    void addRemap(std::string_view face, std::string_view variant,
                  std::uint32_t codepoint, GlyphReference target);
    void addParameter(std::string_view face, std::string key, double value);
    void addDelimiter(std::string_view face, std::uint32_t codepoint,
                      DelimiterRecipe recipe);
    FontRepository freeze() &&;
};
}
