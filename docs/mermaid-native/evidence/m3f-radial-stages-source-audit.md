# M3f radial stage Source audit (read-only prerequisite)

Status: Source and current-consumer audit only. M3e accepted the shared **geometry projection**, not stop preparation, radial sampling, Direct2D pixels, or the full Mermaid backend. This document proposes no frozen API and records no new test result.

## Pinned evidence

`third_party/mermaid-rs-renderer/Cargo.lock:697–715` pins `tiny-skia` **0.12.0** and `tiny-skia-path` **0.12.0**. The paths below are relative to `C:/Users/think book/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/`; hashes are full SHA-256 of the local pristine files, not crate checksums.

| Source file | SHA-256 | Relevant lines |
| --- | --- | --- |
| `tiny-skia-0.12.0/src/shaders/radial_gradient.rs` | `3ae883526cead4d5361f8b6974a91dece8f62fe3740a1712be8ce6219efdd74c` | `28–80`, `120–232`, `234–313`, `316–369` |
| `tiny-skia-0.12.0/src/shaders/gradient.rs` | `c8a87c87acb85764d5e13c95fd5f66531d24e2c145513e84adfa372d9a878847` | `16–18`, `28–36`, `51–103`, `106–258`, `261–267` |
| `tiny-skia-0.12.0/src/shaders/mod.rs` | `4f80ecb40d5a82985d97324748779cffd98252fca798c01c3f26969a1c27cbd3` | `63–92`, `95–145` |
| `tiny-skia-0.12.0/src/pipeline/mod.rs` | `fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4` | `66–150`, `252–263`, `308–339`, `347–375`, `395–480`, `501–542` |
| `tiny-skia-0.12.0/src/pipeline/highp.rs` | `81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416` | `58–139`, `211–217`, `256–269`, `693–702`, `894–989`, `1020–1137` |
| `tiny-skia-0.12.0/src/pipeline/lowp.rs` | `3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88` | `71–153`, `237–243`, `255–266`, `558–657`, `661–729`, `849–854` |
| `tiny-skia-0.12.0/src/pipeline/blitter.rs` | `a3189191d6ad4e1e03e738d21f5a26f310b7c00c33bae0021f0961054ad08f06` | `46–83`, `79–194` |
| `tiny-skia-0.12.0/src/color.rs` | `3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d` | `455–539` |
| `tiny-skia-path-0.12.0/src/scalar.rs` | `ffec71494c2c1c281300fab24efe4a33e900e770a836ea4c85aa4665e45b9e23` | `12`, `49–70` |

## Boundary between M3e geometry and stage construction

`RadialGradient::new` selects `None`, solid, thin ring, or a `Radial`/`Strip`/`Focal` geometry (`radial_gradient.rs:120–232,316–369`). M3e projects that selection and its `points_to_unit` matrix. The Source `FocalData::set` uses `is_focal_on_circle()` while preparing the matrix (`:28–67`), and the accepted native projection has that predicate (`scene/prepare/radial_geometry.rs:19–56`). Two other Source predicates are **stage decisions**, not completed M3e output: `is_well_behaved() = !is_focal_on_circle() && r1 > 1.0` and `is_natively_focal() = focal_x.is_nearly_zero()` (`radial_gradient.rs:70–80`). `is_focal_on_circle() = (1.0-r1).is_nearly_zero()`. The `Scalar` tolerance is inclusive `abs(value) <= 1/4096`, not the constructor's smaller `DEGENERATE_THRESHOLD = 1/32768` (`tiny-skia-path/scalar.rs:12,56–70`; `gradient.rs:16–18`). The final `Focal {r1,focal_x,is_swapped}` fields suffice to evaluate these predicates; they must be evaluated at stage preparation time under the same f32 semantics.

`RadialGradient::push_stages(cs,p)` initializes `TwoPointConicalGradientCtx {mask: zero, p0, p1}` before calling the generic `Gradient::push_stages` (`radial_gradient.rs:234–259`; context definition `pipeline/mod.rs:333–339`). It computes parameters in this exact branch order:

| Kind | `p0`, `p1` (f32 Source order) | Geometry stages before spread |
| --- | --- | --- |
| `Radial` with `radius1 == 0.0` | `(1.0, 0.0)` | `XYToRadius` only |
| Other `Radial` | `d_radius = radius2-radius1`; `p0 = max(radius1,radius2)/d_radius`; `p1 = -radius1/d_radius` | `XYToRadius`, then `ApplyConcentricScaleBias` iff the resulting pair differs from `(1.0,0.0)` |
| `Strip` | `(scaled_r0*scaled_r0, 0.0)` | `XYTo2PtConicalStrip`, `Mask2PtConicalNan` |
| `Focal` | `(1.0/r1, focal_x)` | Table below |

These are Source `radial_gradient.rs:235–275`. The context mask is zero only until a Strip or non-well-behaved Focal mask stage sets it; no mask stage is appended for ordinary Radial/well-behaved Focal (`:253–257,305–311`).

Focal stage selection and the **post-formula** operations are ordered, not mutually interchangeable (`radial_gradient.rs:276–311`):

| Precedence | Formula stage | Following conditional stages, in their actual order |
| --- | --- | --- |
| `is_focal_on_circle()` | `XYTo2PtConicalFocalOnCircle` | See common sequence below; since this is not well-behaved, mask is included. |
| Else `is_well_behaved()` | `XYTo2PtConicalWellBehaved` | See common sequence; no degenerate mask. |
| Else `is_swapped || (1.0-focal_x)<0.0` | `XYTo2PtConicalSmaller` | See common sequence. |
| Else | `XYTo2PtConicalGreater` | See common sequence. |

The common sequence is: `Mask2PtConicalDegenerates` if **not** well-behaved; `NegateX` if `(1.0-focal_x)<0.0`; `Alter2PtConicalCompensateFocal` if **not** natively focal; `Alter2PtConicalUnswap` if swapped. The generic gradient then adds spread/color/premultiply; only **after** all those does Focal append `ApplyVectorMask` if not well-behaved. Strip likewise appends `ApplyVectorMask` after color, while Radial never does (`radial_gradient.rs:287–311`; `gradient.rs:252–258`). Swapped does not itself imply `NegateX`; those conditions must remain separate. Nearly-zero predicates use the inclusive scalar threshold; `(1-f)<0` is a strict sign comparison.

## Complete stage order and parameters

`Gradient::push_stages` is the common wrapper (`gradient.rs:106–258`):

1. Append `SeedShader`. Invert the stored gradient transform; on failure warn and return `false` (no shader fallback). Compute `inverse_transform.post_concat(points_to_unit)`, and `push_transform` appends `Transform` only if this composed matrix is finite and non-identity (`gradient.rs:113–125`; `pipeline/mod.rs:370–375`). This finite/identity check is distinct from the earlier inverse failure.
2. Execute the radial `pre` closure above. It transforms the seeded position into scalar `t`, possibly setting a vector validity mask.
3. Append tile stage: `ReflectX1` for Reflect, `RepeatX1` for Repeat, or `PadX1` for Pad **only if** `has_uniform_stops`; irregular/hard stops leave `t` unclamped for the general lookup (`gradient.rs:127–143`). Uniformity is computed after endpoint dummy insertion, monotonic clamping, and pinning the final position to `1.0` (`gradient.rs:51–103`). The predicate uses `is_nearly_equal` at the scalar threshold, not bitwise equality.
4. Exactly two normalized stops use `EvenlySpaced2StopGradient` with RGBA `factor = expanded(c1)-expanded(c0)`, `bias = expanded(c0)`. More stops build a `GradientCtx` (`factor`, `bias`, `t_values`, `len`) and use `Gradient`; the initial constant color, intervals with `t_l < t_r`, final constant color, duplicate endpoint treatment, `>=` search, and zero padding to 16 entries are significant (`gradient.rs:145–250`; `pipeline/mod.rs:308–331`). `ColorSpace::expand_color` transforms RGB and preserves alpha (`color.rs:455–501`).
5. Append `Premultiply` iff `colors_are_opaque == false`, then radial `post` mask. `Gradient::apply_opacity` mutates stop alpha and recomputes `colors_are_opaque`; stage preparation therefore consumes the final stop alpha (`gradient.rs:252–267`). `Shader::is_opaque` nevertheless conservatively reports false for **all** radial shaders because conical locations can be undefined (`shaders/mod.rs:63–75`). The blitter may add masks, coverage, blend, colorspace conversion and store **after** shader stages (`pipeline/blitter.rs:79–194`).

The Source stage enum, context and order reside in `pipeline/mod.rs:68–150,252–339`; shader dispatch is `shaders/mod.rs:79–92`. `RasterPipelineBuilder::compile` selects a whole highp program when `force_hq_pipeline` is true **or any appended stage** has a lowp `null_fn`; otherwise it selects whole lowp, with tail substitutions for load/store (`pipeline/mod.rs:395–480`). The blitter constructs separate anti-horizontal, rectangle and mask programs and passes `paint.force_hq_pipeline` into each (`pipeline/blitter.rs:79–194`). Thus the kind alone does not always determine precision: downstream blend/colorspace stages can force highp.

## Actual highp kernels and validity behavior

Highp seeds pixel centers `(dx+0.5,dy+0.5)` in eight f32 lanes; `Transform` applies the composed matrix by nested `mad` expressions (`highp.rs:256–269,693–702`). The relevant scalar kernels are exactly (`highp.rs:1020–1137`, with `x=p.r`, `y=p.g`, `p0/p1` from the context):

| Stage | Assignment to `t` (`p.r`) / context |
| --- | --- |
| `XYToRadius` | `sqrt(x*x+y*y)` |
| `ApplyConcentricScaleBias` | `t*p0+p1` (separate multiply and add in Source) |
| `XYTo2PtConicalStrip` | `x+sqrt(p0-y*y)` |
| `XYTo2PtConicalFocalOnCircle` | `x+y*y/x` |
| `XYTo2PtConicalWellBehaved` | `sqrt(x*x+y*y)-x*p0` |
| `XYTo2PtConicalGreater` | `sqrt(x*x-y*y)-x*p0` |
| `XYTo2PtConicalSmaller` | `-sqrt(x*x-y*y)-x*p0` |
| `NegateX` | `-t` |
| `Alter2PtConicalCompensateFocal` | `t+p1` |
| `Alter2PtConicalUnswap` | `1.0-t` |
| `Mask2PtConicalNan` | If `t` is NaN, set `t=0` and validity mask false; otherwise true. It does **not** mask all negative `t`. |
| `Mask2PtConicalDegenerates` | If `t<=0` or NaN, set `t=0` and validity mask false; otherwise true. |
| `ApplyVectorMask` | Bitwise AND of each resulting RGBA lane with the saved validity mask, after color interpolation and optional premultiply. |

`Mask2PtConicalNan` is `highp.rs:1076–1085`; `Mask2PtConicalDegenerates` is `:1087–1096`; bitwise final mask is `:1098–1107`. Masking the *color after lookup* matters: replacing invalid `t` by zero alone would wrongly retain the first stop's color. Formula overflow/NaN and signed-zero behavior follow these vector operations, rather than a preemptive geometric rejection.

Highp spread is `PadX1: normalize(t)`, `RepeatX1: normalize(t-floor(t))`, and Reflect's exact `abs((t-1)-2*floor((t-1)*0.5)-1).normalize()` expression (`highp.rs:894–914`). General stop lookup counts `t >= t_values[i]` from index 1 then evaluates `mad(t,factor[idx],bias[idx])`; two-stop evaluation likewise uses `mad` on expanded RGBA (`:916–989`). Highp premultiply multiplies float RGB by alpha (`:211–217`). Exact treatment of NaN at spread/lookup must be compared at this sequence, not silently replaced with an abstract gradient clamp.

## lowp/highp selection and limits

`lowp.rs:124–144` supports `PadX1`, `ReflectX1`, `RepeatX1`, `Gradient`, `EvenlySpaced2StopGradient`, and **only** `XYToRadius` among the radial formula stages. Its `ApplyConcentricScaleBias`, every Strip/Focal formula, mask, compensation, unswap, and `NegateX` entries are `null_fn`. Therefore a plain `Radial` path without scale/bias is lowp-eligible, provided later stages also support lowp and `force_hq_pipeline` is false. A `Radial` requiring concentric scale/bias, every Strip, and every Focal selects highp as a whole; `null_fn` is a compile-time selector, not a stage that is actually executed (`pipeline/mod.rs:406–454`). Additional color-space expansion/compression stages are lowp `null_fn` (`lowp.rs:145–153`), so non-linear `ColorSpace` or another unsupported downstream stage can make even plain radial highp (`color.rs:522–539`; `pipeline/blitter.rs:90–113,130–150`).

Lowp processes 16 pixel centers, keeps transformed coordinates and `t` as f32 lanes through its transform/spread/lookup stages, then rounds color channels to u16 byte values (`lowp.rs:255–266,558–729`). Lowp `Premultiply` uses integer `div255(rgb*alpha)` where `div255(v)=(v+255)>>8`, whereas highp multiplies float channels before later storage (`lowp.rs:237–243,849–854`; `highp.rs:211–217`). Its `XYToRadius` is `sqrt(x*x+y*y)` over f32x16 (`lowp.rs:649–657`). A portable D2D shader cannot infer highp/lowp merely from `RadialKind`; it needs the final Source-like stage-list/paint conditions if exact pixel parity is the goal. This audit does not choose an emulation policy.

## Repository boundary and uncovered work

Current repository hashes and observations (SHA-256):

| Native file | SHA-256 | Current role / gap |
| --- | --- | --- |
| `third_party/mermaid-rs-renderer/src/scene/prepare/radial_geometry.rs` | `1846b28f5dce8f781e913fe84f19d8eedd3e01390b39d565f2aa54b4fb4fcc42` | `:60–149` prepares M3e geometry only; private Focal `:54–56` has on-circle predicate used for matrix. No stage predicates or sampling. |
| `third_party/mermaid-rs-renderer/src/scene/prepare/radial_types.rs` | `5696f03be5c267ef9d718a22892d31992d577cd402b6b703cbe26243dfc55c9a` | `:5–16` preserves `Radial/Strip/Focal`, `points_to_unit`, and thin-ring marker, but carries no stops, tile mode, opacity, color space, shader program, or validity mask. |
| `third_party/mermaid-rs-renderer/src/scene/prepare/mod.rs` | `39d3921d7b0f8e6750e91206445e6204c1ca7b17ab1a60a9bf44992adcf3f86e` | `:2–3` exposes only types and geometry preparation. |
| `third_party/mermaid-rs-renderer/src/scene/paint.rs` | `d84c9ba2641b4e78927368021469c621409640510ebb6ba19247879279cc791c` | `:15–44` has `Paint::None/Solid/LinearGradient`, f64 linear stops and spread; no typed radial resource. |
| `third_party/mermaid-rs-renderer/src/scene/mod.rs` | `ef058f1752eda8c26923d598047539bf3d002e127ca2552958d5708b8e4cb2b0` | `:89–114` stores and exposes `Vec<LinearGradient>` only. |
| `third_party/mermaid-rs-renderer/src/backends/direct2d/paint.rs` | `02c21b2d5ae5d8456ed24ee019b18f9399f8cd4d1ac361b02d53ee2986aa9abf` | `:103–151` consumes only solid/linear D2D brushes; `:127–148` uses D2D stop collection and linear brush, not the Source conical stages or validity mask. |
| `third_party/mermaid-rs-renderer/src/backends/direct2d.rs` | `dfdc9de4ff63981022c477eb45cf5767e8ff58ebbc64de73c1e8a7091a7d52dc` | `:13` includes the paint backend; no radial consumer is wired. |

The accepted M3e `thin_ring` flag is an instruction to replace stops with `(0,start),(1,start),(1,end)` before `Gradient::new` (`radial_gradient.rs:150–175`), not a replacement for stop interpolation. A complete shared typed radial path still needs Source stop normalization and color expansion; full stage parameter/branch preparation; Source-equivalent `t`/spread/lookup/premultiply/validity-mask behavior in a consumer; resolved radial paint resource and both SVG/D2D consumers; and independent comparisons at stage and pixel boundaries. The current D2D linear brush's stop precision/interpolation settings do not establish conical parity. No SVG serialization/reparse or approximation is justified by this audit; root must freeze the next interface and acceptance separately.
