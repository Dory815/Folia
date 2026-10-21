/*************************************************************
 * Read-only generation-order plan for MathJax SVG local glyph caches.
 *************************************************************/
export module folia.math.output.svg.font_cache;
export import folia.math.output.scene;
import std;

export namespace folia::math::output::svg {
using CacheScopeId = std::size_t;

struct CacheDefinition {
    std::string id;
    scene::Glyph const *glyph{};
};

struct CacheScope {
    std::vector<CacheDefinition> definitions;
    std::unordered_map<scene::Glyph const *, std::string> references;
    std::unordered_map<scene::ActionRegion const *, CacheScopeId> tooltips;
};

class FontCachePlan final {
    // Glyph and ActionRegion pointers borrow from the input Scene. Keep this
    // plan shorter-lived than that Scene; do not retain it in a session.
    std::vector<CacheScope> scopes_;

  public:
    FontCachePlan(scene::Scene const &, std::span<std::string const> prefixes);
    [[nodiscard]] static std::size_t scopeCount(scene::Scene const &);
    [[nodiscard]] CacheScope const &scope(CacheScopeId) const;
};
} // namespace folia::math::output::svg
