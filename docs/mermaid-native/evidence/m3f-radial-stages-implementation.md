# M3f radial stage projection — implementation report

Status: **awaiting root independent Source acceptance**. This packet implements only the frozen `prepare_radial_stages(RadialKind) -> RadialStagePlan` in `third_party/mermaid-rs-renderer/src/scene/prepare/radial_stages.rs`. It does not prepare gradient stops, sample colors, render radial pixels, change a backend, or complete Mermaid/Folia integration. Sol did not build, run tests/oracles, edit fixtures, or commit.

Pinned reference: `tiny-skia` 0.12.0 `src/shaders/radial_gradient.rs`, SHA-256 `3ae883526cead4d5361f8b6974a91dece8f62fe3740a1712be8ce6219efdd74c`; `tiny-skia-path` 0.12.0 `src/scalar.rs`, SHA-256 `ffec71494c2c1c281300fab24efe4a33e900e770a836ea4c85aa4665e45b9e23`; lowp table `tiny-skia` 0.12.0 `src/pipeline/lowp.rs`, SHA-256 `3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88`. The only native state input remains root-frozen `RadialKind`.

| Source operation | Target mapping |
| --- | --- |
| `FocalData::is_focal_on_circle` `radial_gradient.rs:70–72` | Private `FocalData::is_focal_on_circle`: `(1.0 - r1).is_nearly_zero()` using `tiny_skia_path::Scalar`. |
| `FocalData::is_well_behaved` `:74–76` | Private method: `!is_focal_on_circle() && r1 > 1.0`, retaining short-circuit order. |
| `FocalData::is_natively_focal` `:78–80` | Private method: `focal_x.is_nearly_zero()`. Source's inclusive `Scalar` threshold is used, including threshold neighbors and NaN behavior; no custom epsilon. |
| `push_stages` radial `p0/p1` `:235–245` | Exact `radius1 == 0.0` fast branch `(1.0,0.0)`; otherwise `d_radius = radius2 - radius1`, `p0 = radius1.max(radius2) / d_radius`, `p1 = -radius1 / d_radius` in that order. Negative zero, zero denominator, NaN and infinity are not pre-filtered. |
| Strip parameters `:247–249` | `(scaled_r0 * scaled_r0, 0.0)` with f32 multiplication. |
| Focal parameters `:250` | `(1.0 / r1, focal_x)` without denominator validation. |
| Two-point context `:253–257` | Frozen plan carries the exact `p0/p1`; the Source vector mask is runtime sampling state and is outside this preparation packet. |
| Radial pre-stages `:263–271` | `XYToRadius`; append `ApplyConcentricScaleBias` only if `(p0,p1) != (1.0,0.0)`, using ordinary Rust float tuple comparison. |
| Strip pre-stages `:272–275` | `XYTo2PtConicalStrip`, then `Mask2PtConicalNan`; its `ApplyVectorMask` is placed in `after_color`, per `:305–306`. |
| Focal formula `:276–285` | Precedence is on-circle, else well-behaved, else swapped or strict `(1.0-focal_x)<0.0` to Smaller, else Greater. This mirrors the Source's separate branches. |
| Focal post-formula `:287–301` | Append, independently and in order: degenerates mask if not well-behaved; `NegateX` if strict negative `1.0-focal_x`; focal compensation if not natively focal; unswap if swapped. |
| Focal post-color `:305–311` | Append `ApplyVectorMask` only when not well-behaved. It remains after color lookup and optional premultiplication, which a later packet must represent. |
| lowp table `lowp.rs:124–144` | `geometry_lowp_compatible` is true only when `before_color` consists solely of `XYToRadius` and `after_color` is empty. Other radial entries are `null_fn`. This flag does not decide final whole-pipeline precision, which also depends on non-radial stages and force-HQ. |

The projection intentionally does not add Source `Gradient::push_stages` seed/transform/spread/stops/premultiply, highp/lowp pixel kernels, or the final validity mask value. Source keeps these as separate responsibilities; the `before_color` and `after_color` vectors preserve their insertion points. The only precision qualification supplied here is radial-stage lowp eligibility, as frozen in the contract.

Open questions for root acceptance: none requiring an API change from this implementation. The root oracle should verify bit-exact non-NaN `p0/p1` and NaN category, full stage order, and lowp flag against the already frozen 3,266 actual-Source observations. The file has not been compiled or tested by Sol under the task boundary; root retains all independent verification and mutant review.
