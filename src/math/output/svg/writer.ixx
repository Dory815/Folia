export module folia.math.output.svg.writer;
export import folia.math.output.scene;
export import folia.math.output.svg.font_data;
export import folia.math.output.svg.font_cache;
import std;

export namespace folia::math::output::svg {
class UnsupportedError final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

enum class FontCacheMode { none, local, global };

struct SvgWriteOptions {
    FontCacheMode font_cache{FontCacheMode::local};
    bool use_xlink{true};
    bool include_math_container{true};
    std::vector<folia::math::output::scene::SemanticAttribute> container_attributes{
        {"class", "MathJax"}, {"jax", "SVG"}, {"overflow", "linebreak"}};
    // Immutable Source path data for the Scene GlyphKey values. The catalog is
    // supplied by the owner; this serializer neither loads nor caches fonts.
    folia::math::output::svg::SvgFontResources const* font_resources{};
    // Complete resolved Source prefixes in root/nested processMath order.
    // The renderer allocates these identities; the writer only projects them.
    std::vector<std::string> local_cache_prefixes;
};

// Serializes completed geometry and metadata. All values that affect placement
// or font selection must already be present in Scene.
[[nodiscard]] std::string write(
    folia::math::output::scene::Scene const& scene,
    SvgWriteOptions const& options = {});
}
