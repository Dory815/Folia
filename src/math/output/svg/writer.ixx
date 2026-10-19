export module folia.math.output.svg.writer;
export import folia.math.output.scene;
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
    // Already-resolved MathJax local cache ID. The renderer owns the monotonic
    // source counter; this pure writer cannot allocate cache identities.
    std::string local_id;
};

// Serializes completed geometry and metadata. All values that affect placement
// or font selection must already be present in Scene.
[[nodiscard]] std::string write(
    folia::math::output::scene::Scene const& scene,
    SvgWriteOptions const& options = {});
}
