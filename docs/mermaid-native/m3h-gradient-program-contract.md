# M3h Root freeze — complete generic gradient wrapper and color expansion

This packet completes portable preparation shared by SVG and Direct2D. Root
owns types, Source observers, stimuli, tests, mutations and acceptance. Sol may
edit only `src/scene/prepare/gradient_color_space.rs`,
`src/scene/prepare/gradient_program.rs` under the Mermaid crate and
`docs/mermaid-native/evidence/m3h-gradient-program-implementation.md`.
No API/type/module changes, fixtures, expectations, tests, builds, commits,
additional delegation, scene/backend/Folia edits are authorized for the agent.

Pinned tiny-skia 0.12.0 responsibilities: complete `Gradient::push_stages`
(`shaders/gradient.rs:106–258`), `ColorSpace::expand_channel/expand_color`
(`color.rs:479–502`), complete `math::approx_powf` (`math.rs:27–56`),
`GradientColor::new/From<Color>`, `GradientCtx::push_const_color`, and the
`RasterPipelineBuilder::push_transform` condition/default contexts. Hashes and
all numeric boundaries are in `evidence/m3h-gradient-context-source-audit.md`
and the Root oracle. Source licenses must be retained in implementation files.

`gradient_program_types.rs` freezes one `GradientProgram`: return flag, ordered
typed stages, effective transform, separate Source-default two-stop and general
contexts, radial p0/p1 and eight zero mask lanes. `GradientColor` is raw f32,
not normalized. General factors/biases retain padding; t-values do not. No f64,
byte colors, host brush interpolation, stop merging or second backend-specific
preparation is allowed. Reuse accepted M3g stops and M3f radial plans without
recomputing either. Numeric services use pinned tiny-skia-path values/helpers.

The signature projects original pre/post callbacks onto an optional M3f plan:
radial context exists before generic wrapper entry; before/after stages append
at exactly Source callback sites. None is the original empty callbacks. This
packet accepts prepared M3f plans; it does not claim arbitrary callback code or
an extensible user pipeline. Whole-pipeline precision/sampling remains required.

Preserve SeedShader even when inversion fails. Keep Source transform composition
order and finite/nonidentity omission; omitted transform stays Source identity.
Color contexts stay default on inversion failure. Preserve all four ColorSpace
modes, original approx_powf operation sequence/constants/casts/rounding/branch
returns, normalization rules, inclusive 0.04045 cutoff and unchanged alpha.
The inverse-failure Source warning is retained through the pinned native log
facade; Root observes actual Source/native warning messages independently.
Exactly two stops choose two-stop path; other counts use general path. Endpoint
dummy exclusion compares original colors before expansion. Equal-position
intervals still update left color/position. Keep factor/bias operation order,
nonfinite raw contexts, len before padding to 16, premultiply before post mask.

Before delegation Root executes original bodies in an ignored pristine copy;
only exact observer/export additions are allowed and every `.rs` is guarded.
No native crate is linked. `gradient-program-oracle.cjs --check` reproduces
13,556 initial programs (13,476 valid colors; 13,392 successful wrappers), 1,296 color
expansions and 8,010 power operations. Fixture SHA256:
`79b614e47553c46dc2b424177c5e1fa3edd7e919834b061bfdb39fd077e6be42`.
Program stimuli preserve every M3g Source input crossed with all four spaces,
21 independently observed M3f stage orders, and added singular/nonfinite/
identity/composed transforms across every spread. Colors include cutoff neighbors,
signed zero/subnormals and invalid construction. Power includes special values,
zero/one early returns, varied exponents and deterministic finite inputs.

Root test compares every returned flag, stage order, transform bit, both
contexts including inactive defaults/padding, t-values and radial context.
Three compiled stub tests must fail before delegation. Acceptance additionally
requires compiling semantic mutations detected by unchanged assertions,
independent diff review, Source reproducibility and relevant full native matrix.
Neither agent completion nor this freeze passes backend/Folia acceptance.

The frozen mutation families are: powf replacement, lost zero/one early return,
power rounding truncation, wrong exponent/FullSRGB cutoff/operation order,
expanded alpha, dropped SeedShader on inverse failure, composition reversal,
missing finite/identity guard, misplaced pre/post callbacks, unconditional Pad,
wrong spread mapping, uniform-only two-stop choice, endpoint color comparison
after expansion, lost equal-position color update, factor/bias clamping,
wrong bias sign, len measured after padding, padded t-values, lost inactive
defaults, premultiplication ignored or moved after mask, and lost radial context.
Root may add independent Source witnesses when a semantic mutant survives;
expected values still come exclusively from execution of original Source.

Root compiled the three unimplemented native tests before delegation: exit 101,
zero passed/three failed, each at its frozen stub. The Source adapter independently
compiled and executed successfully. Logs are ignored build evidence; neither
the deliberately red native baseline nor a compiler error is a passed comparison.

## Root review and final independent extension

Root review found that the Sol implementation omitted the original failure
warning. Root restored it and binds the already locked log 0.4.33 facade without
changing other native dependency versions. The Source observer adapter records
warnings from the actual original body; the native test compares them. No Source
method body changed. The initial independent fields/rows remain unchanged.

Root extends Source stimuli by 432 endpoint-color witnesses across all four
spaces/all spreads: different original small/subnormal colors can collapse to
the same expanded f32 color. Endpoint exclusion must still compare originals.
Final corpus: **13,988 programs, 1,296 colors, 8,010 powers**, 13,908 valid color
constructions and 13,824 successful wrappers. Final fixture SHA256:
`bfcb703bea4b279735c60806b14f55208683c13d1f3c69ce0f8de37675da83f3`.
This independently generated extension strengthens the frozen comparison; no
native value supplies an expectation.

The initial `a.round() as u32` → `a as u32` mutant survived and is not counted
as detected. Root Source analysis identifies equivalent output: the last add
uses `27.728.../(4.842...-f)`, with finite f in [0,1), so that operand lies in
[4,8). A positive result at least 1 is a binary32 multiple of 2^-23; a positive
result below 1 requires cancellation of two magnitudes in [4,8), hence is a
multiple of their 2^-21 spacing. Multiplication by 2^23 therefore yields an
integer before the guarded cast. Nonfinite/negative branches never expose a
fractional positive cast operand. Removing round is output-equivalent here;
the reviewed faithful implementation nevertheless keeps Source round exactly.
The effective power-bit mutant changes the mantissa exponent instead. The
surviving initial run remains recorded separately, not relabeled as a pass.
