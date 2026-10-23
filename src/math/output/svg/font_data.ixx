export module folia.math.output.svg.font_data;
import std;

export namespace folia::math::output::svg {
struct SvgFontGlyphKey {
    // Exact semantic variant, never the potentially shared SVG cache ID.
    std::string variant;
    std::uint32_t codepoint{};
    friend auto operator<=>(SvgFontGlyphKey const&, SvgFontGlyphKey const&) = default;
};

struct SvgFontGlyphSource {
    // path_defined distinguishes an absent p option from a present empty p.
    bool path_defined{};
    // Exact Source wrapper output: nonempty p becomes M+p+Z; empty p becomes
    // an empty string. path_defined distinguishes empty p from absent p.
    std::string path_data;
    std::optional<std::string> content_data; // exact Source c recipe when present
};
class SvgFontGlyphLookup {
public:
    virtual ~SvgFontGlyphLookup() = default;
    virtual SvgFontGlyphSource const* find(SvgFontGlyphKey const& key) const = 0;
};

class SvgFontResourcesBuilder;
class SvgFontResources {
    std::map<SvgFontGlyphKey, SvgFontGlyphSource> glyphs_;
    std::map<SvgFontGlyphKey, SvgFontGlyphKey> aliases_;
    std::shared_ptr<SvgFontGlyphLookup const> source_;
    explicit SvgFontResources(decltype(glyphs_) glyphs,
                              decltype(aliases_) aliases, std::shared_ptr<SvgFontGlyphLookup const> source)
        : glyphs_(std::move(glyphs)), aliases_(std::move(aliases)), source_(std::move(source)) {}
    friend class SvgFontResourcesBuilder;
public:
    SvgFontResources() = default;
    SvgFontGlyphSource const* find(SvgFontGlyphKey const& key) const;
    std::size_t size() const noexcept;
    auto const& entries() const noexcept { return glyphs_; }
    auto const& aliases() const noexcept { return aliases_; }
};

class SvgFontResourcesBuilder {
    std::map<SvgFontGlyphKey, SvgFontGlyphSource> glyphs_;
    std::map<SvgFontGlyphKey, SvgFontGlyphKey> aliases_;
    std::shared_ptr<SvgFontGlyphLookup const> source_;
public:
    void add(SvgFontGlyphKey key, SvgFontGlyphSource source);
    // Exact generated alias for an effective Source key whose path bytes are
    // shared with another pinned key. This never probes a fallback variant.
    void addAlias(SvgFontGlyphKey key, SvgFontGlyphKey target);
    void setSource(std::shared_ptr<SvgFontGlyphLookup const> source) { source_ = std::move(source); }
    SvgFontResources freeze() &&;
};
}
