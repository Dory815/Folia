/*************************************************************
 * Generation-order cache planning for MathJax 4.1.3 FontCache.ts.
 *************************************************************/
module folia.math.output.svg.font_cache;
import std;

namespace folia::math::output::svg {
namespace {
struct Event {
    std::uint64_t order{};
    scene::GlyphRun const *run{};
    scene::ActionRegion const *region{};
};

void collectEvents(scene::Group const &group, std::vector<Event> &events) {
    for (auto const &entry : group.entries) {
        if (auto const *primitive = std::get_if<scene::Primitive>(&entry)) {
            if (auto const *run = std::get_if<scene::GlyphRun>(primitive);
                run && !run->glyphs.empty()) {
                events.push_back({run->resource_order, run, nullptr});
            }
        } else if (auto const *child = std::get_if<std::shared_ptr<scene::Group const>>(&entry)) {
            if (!*child)
                throw std::invalid_argument("FontCachePlan encountered a null Scene group");
            collectEvents(**child, events);
        } else {
            auto const &region = std::get<scene::ActionRegion>(entry);
            if (region.tooltip)
                events.push_back({region.resource_order, nullptr, &region});
        }
    }
}

std::size_t countScopes(scene::Scene const &sceneValue) {
    std::size_t count = 1;
    std::vector<Event> events;
    collectEvents(sceneValue.root(), events);
    for (auto const &event : events) {
        if (!event.region || !event.region->tooltip)
            continue;
        const auto nested = countScopes(*event.region->tooltip);
        if (nested > std::numeric_limits<std::size_t>::max() - count)
            throw std::overflow_error("FontCachePlan scope count overflow");
        count += nested;
    }
    return count;
}

bool containsGlyphs(scene::Scene const &sceneValue) {
    std::vector<Event> events;
    collectEvents(sceneValue.root(), events);
    for (auto const &event : events) {
        if (event.run)
            return true;
        if (event.region && event.region->tooltip && containsGlyphs(*event.region->tooltip))
            return true;
    }
    return false;
}

std::string glyphId(std::string_view prefix, scene::Glyph const &glyph) {
    constexpr char digits[] = "0123456789ABCDEF";
    std::string code;
    auto value = glyph.key.codepoint;
    do {
        code.push_back(digits[value & 0xFu]);
        value >>= 4;
    } while (value != 0);
    std::ranges::reverse(code);
    std::string id{"MJX-"};
    id.reserve(id.size() + prefix.size() + glyph.key.variant_cache_id.size() + code.size() + 1);
    id.append(prefix);
    id += glyph.key.variant_cache_id;
    id.push_back('-');
    id += code;
    return id;
}

struct ActiveCache {
    CacheScopeId bucket{};
    std::string prefix;
    std::unordered_set<std::string> identifiers;
};

void processScene(scene::Scene const &sceneValue, CacheScopeId invocation,
                  std::span<std::string const> prefixes, bool prefixesAvailable,
                  std::vector<CacheScope> &scopes, ActiveCache &active) {
    std::vector<Event> events;
    collectEvents(sceneValue.root(), events);
    std::ranges::sort(events, {}, &Event::order);

    for (auto const &event : events) {
        if (event.run) {
            for (auto const &glyph : event.run->glyphs) {
                auto id = glyphId(active.prefix, glyph);
                scopes.at(invocation).references.insert_or_assign(&glyph, id);
                if (active.identifiers.insert(id).second)
                    scopes.at(active.bucket).definitions.push_back({std::move(id), &glyph});
            }
            continue;
        }

        auto const &region = *event.region;
        if (scopes.size() == std::numeric_limits<CacheScopeId>::max())
            throw std::overflow_error("FontCachePlan scope id space exhausted");
        const auto nested = scopes.size();
        scopes.emplace_back();
        auto [entry, inserted] = scopes.at(invocation).tooltips.emplace(&region, nested);
        (void)entry;
        if (!inserted)
            throw std::logic_error("FontCachePlan encountered a repeated tooltip event");

        active.bucket = nested;
        active.identifiers.clear();
        active.prefix = prefixesAvailable ? prefixes[nested] : std::string{};
        processScene(*region.tooltip, nested, prefixes, prefixesAvailable, scopes, active);
        // MathJax clears the local ID after processMath but leaves the nested
        // definitions/cache active for subsequent outer glyph generation.
        active.prefix.clear();
    }
}
} // namespace

FontCachePlan::FontCachePlan(scene::Scene const &source, std::span<std::string const> prefixes) {
    const auto expectedScopes = scopeCount(source);
    const bool hasGlyphs = containsGlyphs(source);
    if (hasGlyphs && prefixes.size() != expectedScopes)
        throw std::invalid_argument("FontCachePlan prefixes must match all processMath scopes");
    if (!hasGlyphs && !prefixes.empty() && prefixes.size() != expectedScopes)
        throw std::invalid_argument("FontCachePlan prefixes must be empty or match all scopes");

    const bool prefixesAvailable = !prefixes.empty();
    scopes_.reserve(expectedScopes);
    scopes_.emplace_back();
    ActiveCache active{.bucket = 0,
                       .prefix = prefixesAvailable ? prefixes.front() : std::string{},
                       .identifiers = {}};
    processScene(source, 0, prefixes, prefixesAvailable, scopes_, active);
    if (scopes_.size() != expectedScopes)
        throw std::logic_error("FontCachePlan scope traversal disagrees with its count pass");
}

std::size_t FontCachePlan::scopeCount(scene::Scene const &source) {
    return countScopes(source);
}

CacheScope const &FontCachePlan::scope(CacheScopeId id) const {
    if (id >= scopes_.size())
        throw std::out_of_range("FontCachePlan scope does not exist");
    return scopes_[id];
}
} // namespace folia::math::output::svg
