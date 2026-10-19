// Direct finite-set matcher for the five pinned source patterns.
module folia.math.core.mml_tree.mo_patterns;
import std;

namespace folia::math::mml {
bool matchMoPattern(MoPattern kind, std::u16string_view text) {
    const auto& spec = moPatternSpec(kind);
    if (text.size() < spec.minimum || (spec.maximum && text.size() > *spec.maximum)) return false;
    for (const char16_t unit : text) {
        const bool allowed = std::any_of(spec.units.begin(), spec.units.end(),
            [unit](const MoCodeUnitRange& range) { return range.first <= unit && unit <= range.last; });
        if (!allowed) return false;
    }
    return true;
}
}
