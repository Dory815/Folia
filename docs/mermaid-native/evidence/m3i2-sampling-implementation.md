# M3i2 executable sampling implementation handoff

Status: **awaiting Root's independent Source review and acceptance**. Only `third_party/mermaid-rs-renderer/src/scene/prepare/sampling/highp.rs`, `lowp.rs`, and this report were edited. No build, test, oracle, fixture, manifest, provider, backend or Folia file was changed or run by this agent. This packet implements the frozen resource-independent one-stage sampling boundary, not a completed paint pipeline or Mermaid renderer.

Pinned original Source: `tiny-skia-0.12.0/src/pipeline/highp.rs` SHA-256 `81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416`; `lowp.rs` SHA-256 `3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88`. The existing nine-file `sampling/wide` provider is Root-preserved from the pinned original and is used directly. The native `GradientProgram` and register types remain the frozen Root declarations.

## Highp Source → native body map

Every listed native arm omits only the original `p.next_stage()` call because `sample_high_stage` executes one caller-selected stage. The channel aliases map exactly to Source `p.r,g,b,a,dr,dg,db,da` and use original `f32x8` arithmetic. Highp Source line ranges include full function bodies.

| Native `PipelineStage` arm | Original `pipeline/highp.rs` | Mechanical adaptation |
| --- | --- | --- |
| `MoveSourceToDestination` | `move_source_to_destination`, `202–209` | Copy four source vectors to destination vectors. |
| `MoveDestinationToSource` | `move_destination_to_source`, `219–226` | Copy four destination vectors to source vectors. |
| `Clamp0` | `clamp_0`, `228–235` | Original vector `.max(0)` on all four source channels. |
| `ClampA` | `clamp_a`, `237–244` | Original vector `.min(1)` on all four source channels. |
| `Premultiply` | `premultiply`, `211–217` | Multiply RGB vectors by the unchanged alpha vector, sequentially. |
| `SeedShader` | `seed_shader`, `256–270` | `origin=[dx,dy]`; identical eight half-pixel centers; reset source blue/alpha and all destination registers as Source. |
| `Transform` | `transform`, `693–703` | `p.ctx.transform` → `program.transform`; retain nested `mad(f,m,a)=f*m+a` for x and y before assigning both. |
| `PadX1` | `pad_x1`, `894–899` | Original vector normalize. |
| `ReflectX1` | `reflect_x1`, `900–909` | Preserve repeated `(x-1)`, `two(v)=v+v`, vector floor/abs/normalize order. |
| `RepeatX1` | `repeat_x1`, `910–915` | Subtract original vector floor then normalize. |
| `Gradient` | `gradient` and `gradient_lookup`, `916–978` | Keep `idx` as original `u32x8`, first comparison at `i=1`, inclusive `>=`, eight explicit lane comparisons per threshold, per-channel gather then `mad`. Source `GradientColor.r/g/b/a` → `components[0/1/2/3]`. |
| `EvenlySpaced2StopGradient` | `evenly_spaced_2_stop_gradient`, `980–990` | `p.ctx.evenly_spaced_2_stop_gradient.factor/bias` → `program.two_stop.factor/bias.components`; four original `mad` calls. |
| `XYToUnitAngle` | `xy_to_unit_angle`, `992–1018` | Preserve Source polynomial constants, nested f32 vector operations, ordered sign masks and NaN-to-zero mask. |
| `XYToRadius` | `xy_to_radius`, `1020–1026` | Separate `x*x`, `y*y`, add, original vector sqrt. |
| `XYTo2PtConicalFocalOnCircle` | `xy_to_2pt_conical_focal_on_circle`, `1028–1034` | Original `x + y*y/x`, with no denominator repair. |
| `XYTo2PtConicalWellBehaved` | `xy_to_2pt_conical_well_behaved`, `1036–1044` | Original `sqrt(x*x+y*y)-x*p0`; `p0=program.radial_p[0]`. |
| `XYTo2PtConicalGreater` | `xy_to_2pt_conical_greater`, `1046–1054` | Original `sqrt(x*x-y*y)-x*p0`. |
| `XYTo2PtConicalSmaller` | `xy_to_2pt_conical_smaller`, `1056–1064` | Original vector negation before `-x*p0`. |
| `XYTo2PtConicalStrip` | `xy_to_2pt_conical_strip`, `1066–1074` | Original `x+sqrt(p0-y*y)`. |
| `Mask2PtConicalNan` | `mask_2pt_conical_nan`, `1076–1085` | Preserve unordered self-inequality, vector blend and bitwise-not/`cond_to_mask`; write raw mask to `program.radial_mask`. |
| `Mask2PtConicalDegenerates` | `mask_2pt_conical_degenerates`, `1087–1096` | Preserve inclusive `t<=0` or NaN mask before blend and mask conversion. |
| `ApplyVectorMask` | `apply_vector_mask`, `1098–1107` | Bitcast `program.radial_mask` to original `u32x8`, AND each RGBA vector's bit pattern, bitcast back. |
| `Alter2PtConicalCompensateFocal` | `alter_2pt_conical_compensate_focal`, `1109–1115` | Add `program.radial_p[1]`. |
| `Alter2PtConicalUnswap` | `alter_2pt_conical_unswap`, `1117–1121` | Original `1-t`. |
| `NegateX` | `negate_x`, `1123–1127` | Original provider's vector unary negation. |
| `ApplyConcentricScaleBias` | `apply_concentric_scale_bias`, `1129–1137` | Original separate multiply then add with `radial_p[0/1]`; no FMA. |
| `GammaExpand2` | `gamma_expand_2`, `1139–1145` | Square source RGB; alpha/destination unchanged. |
| `GammaExpandDestination2` | `gamma_expand_dst_2`, `1147–1153` | Square destination RGB only. |
| `GammaCompress2` | `gamma_compress_2`, `1155–1161` | Original vector sqrt on source RGB. |
| `GammaExpand22` | `gamma_expand_22`, `1163–1169` | Original provider vector `powf(2.2)` on source RGB. |
| `GammaExpandDestination22` | `gamma_expand_dst_22`, `1171–1177` | Same vector `powf(2.2)` on destination RGB. |
| `GammaCompress22` | `gamma_compress_22`, `1179–1185` | Original vector `powf(0.45454545)` on source RGB. |
| `GammaExpandSrgb` | `gamma_expand_srgb`/`srgb_expand`, `1187–1192,1201–1207` | Original threshold `<=0.04045`, both linear and power expression evaluated, mask selects result for source RGB. |
| `GammaExpandDestinationSrgb` | `gamma_expand_dst_srgb`/`srgb_expand`, `1187–1192,1209–1215` | Identical helper on destination RGB. |
| `GammaCompressSrgb` | `gamma_compress_srgb`/`srgb_compress`, `1194–1199,1217–1223` | Original threshold `<=0.0031308`, vector power `0.416666666`, arithmetic and blend on source RGB. |

Highp private helpers also map directly: `cond_to_mask` `1229–1242` (all eight words by original nonzero test), `two` `1351–1354`, and `mad` `1356–1359`. In particular `mad` is multiply followed by add, with the vendored provider's target-feature semantics; no scalar `min/max/floor/round/powf` substitutes are introduced.

## Lowp Source → native body map

The native `u16x16` channels retain exactly the original split/join bit encoding for f32x16 coordinates. The source and destination register names map to the same eight frozen channel slots; `origin=[dx,dy]` supplies the original integer location. `p.next_stage()` is omitted only at the one-stage boundary.

| Native `PipelineStage` arm | Original `pipeline/lowp.rs` | Mechanical adaptation |
| --- | --- | --- |
| `MoveSourceToDestination` | `move_source_to_destination`, `219–226` | Copy raw u16 source channels to destination. |
| `MoveDestinationToSource` | `move_destination_to_source`, `228–235` | Copy raw u16 destination channels to source. |
| `Premultiply` | `premultiply`, `237–243` | Three original `div255(rgb*alpha)` integer operations; retain alpha. |
| `SeedShader` | `seed_shader`, `255–267` | Original 16 half-pixel centers in two `f32x8` halves, `dx/dy as f32`, original `split` into four raw u16 registers. Destination channels remain untouched as in Source. |
| `Transform` | `transform`, `558–571` | Original `join` x/y, nested f32x16 `mad`, `split` into raw registers; source transform → `program.transform`. |
| `PadX1` | `pad_x1`, `573–579` | Join x, normalize through provider, split; y unchanged. |
| `ReflectX1` | `reflect_x1`, `581–592` | Original two/floor/abs/normalize chain in f32x16, then split. |
| `RepeatX1` | `repeat_x1`, `594–600` | Original f32x16 floor/normalize chain, then split. |
| `Gradient` | `gradient`/`gradient_lookup`, `602–632,659–711` | Original 16 u16 indices, loop from 1 and inclusive `>=`; explicit lane increments, factor/bias gather per channel, f32x16 `mad`, original `round_f32_to_u16`. Source color fields → `components[0..3]`. |
| `EvenlySpaced2StopGradient` | `evenly_spaced_2_stop_gradient`, `634–647` | Four nested-context f32x16 `mad` calls then original rounding helper. |
| `XYToRadius` | `xy_to_radius`, `649–657` | Join x/y, `(x*x+y*y).sqrt()` with provider, split both x and original y back. |

Lowp helper mappings: `round_f32_to_u16` `713–730` preserves separate RGB normalize and unnormalized alpha, multiplication by 255 then addition of 0.5 before original provider's `save_to_u16x16`; `div255` `848–854` is `(v+255)>>8`, not division or NEON alternative; `split`, `join`, and `mad` map to `871–902` with byte-preserving `bytemuck` casts/copies and multiply-then-add arithmetic. No scalar f32 coordinate conversion replaces the original packed registers.

## Explicit boundary and review points

Both entry points reject every stage outside their exact owned set with `SamplingError { precision, stage }` **before** converting, modifying, or writing registers/context. They load all eight original register vectors, execute one stage, then store all eight back, leaving untouched channels at their prior raw values. Context fields not written by Source remain unchanged; highp only writes `radial_mask` in its two mask stages. The one-stage API has no continuation cursor, destination pixel access, or `next_stage` side effect.

This code consumes `GradientProgram`'s existing transform/two-stop/general-stop/conical fields. It does not prepare stops again, choose high versus low precision, append a stage, or reconstruct a second context cache. Source's per-stage arithmetic can still produce NaN, infinity and signed-zero outputs. Raw Source captures, unsupported-stage immutability, unchanged provider pins, compile/mutation matrix and actual WARP checks are Root's acceptance work; no result is asserted here.
