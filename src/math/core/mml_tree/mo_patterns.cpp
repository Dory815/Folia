// Native match/callback bindings for the five source-defined MmlMo patterns.
// MathJax 4.1.3 mo.ts; Apache-2.0 (LICENSE-MathJax.txt).
module folia.math.core.mml_tree.mo_patterns;
import std;

namespace folia::math::mml {
bool matchMoPattern(MoPattern kind, std::u16string_view text) {
    const MoPatternSpec& spec = moPatternSpec(kind);
    if (text.size() < spec.minimum) return false;
    if (spec.maximum && text.size() > *spec.maximum) return false;

    for (const char16_t unit : text) {
        const bool allowed = std::any_of(spec.units.begin(), spec.units.end(),
            [unit](const MoCodeUnitRange& range) {
                return range.first <= unit && unit <= range.last;
            });
        if (!allowed) return false;
    }
    return true;
}

Pattern makeMoPattern(MoPattern kind) {
    // Validate before producing a callback so an invalid enum cannot escape as
    // a seemingly valid, deferred failure. Capture only the independent value.
    (void)moPatternSpec(kind);
    return [kind](std::u16string_view text) {
        return matchMoPattern(kind, text);
    };
}
}
