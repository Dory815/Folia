# Shared fraction placement packet

Owner: Luna, canonical `output/common/wrappers/mfrac.ixx/.cpp`, partition
`folia.math.output.common:mfrac`, namespace `folia::math::output::common`.
The earlier standalone placement allocation is withdrawn. The current code
implements numerical records/equations only; complete native CommonWrapper
orchestration and its layout-to-Scene consumer remain unimplemented.

This is the single numerical implementation of the Source fraction family.
It combines CommonMfrac measurement with the placement currently located in
Source SVGMfrac. It has no parser, MML, DOM, SVG XML or runtime-object dependency.
The eventual CommonWrapper resolves attributes/child boxes, calls this service
and emits its completed placements into Scene. There is no parallel Scene
wrapper hierarchy or backend fraction calculation.

Inputs are owned native records: numerator/denominator BBox values; named
fraction font parameters `axis_height`, `num1/2/3`, `denom1/2`, `rule_thickness`,
`nulldelimiterspace`; display style; resolved line thickness; with-delimiters;
numerator/denominator alignment (`left/center/right`). BBox `scale`, `rscale`,
L/R and signed dimensions preserve their distinct Source uses. Do not replace
these with a single guessed scale or a generic property dictionary.

The pure result contains its completed BBox, child origins, optional rule
geometry, content width for percentage-width reconciliation and child wrap
widths. Child scene construction, inherited parent transforms, stretching
lookup and serialization are not its responsibility.

Bevelled fractions have a two-phase Source dependency: compute the required
slash height and numerator/denominator offsets from child metrics; common font
stretching supplies the actual slash BBox; then finish placement using that box
and the previously computed data. This is dependency-ordered layout, not two
independent layout passes. Return the bevel request explicitly rather than
fabricating a slash box or calling a backend. Ordinary ruled fractions and
zero-thickness atop fractions are complete functions using the same inputs.

Preserve `getTUV`, `getUVQ`, `getBevelData`, BBox combines, `getAlignX`, rule
padding and all Source placement equations. Percentage-width reconciliation
remains in common wrapper orchestration; return its required data rather than
pretend that a pure function has mutated child wrappers.

Acceptance: independently execute the pinned Common/SVG fraction methods over
display/inline, ruled/atop/bevel, unequal scale/rscale, L/R, all alignments,
with-delimiters, tall/short/deep children and signed boundary cases. Compare
full native BBox and placements, not one hardcoded formula SVG. Generate paired
fixture data from the same Source observations so inputs and expected outputs
cannot diverge during manual copying. No native Renderer/profile completion is
claimed from this packet. Root promotes/builds/runs/commits; candidates remain
in their assigned ignored draft scope until reviewed.
