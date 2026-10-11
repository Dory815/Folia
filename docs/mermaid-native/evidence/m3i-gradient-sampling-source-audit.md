# M3i gradient sampling Source dependency audit

Status: **read-only prerequisite for root freeze**. M3e geometry, M3f radial stage selection, M3g stop preparation and the M3h typed `GradientProgram` are preparation boundaries. This inventory covers what original tiny-skia 0.12.0 actually executes after that program, including whole-pipeline selection and the paint/blitter postpass. It neither freezes an API nor asserts sampled-pixel parity. No native implementation, build, test, fixture or commit was made here.

Root review: this inventory is not yet a closed numeric dependency audit.
In the current non-AVX2 build, f32x8 delegates many operations to f32x4;
`wide/f32x4_t.rs` SHA256
`d4f6b4b67900f2bb38dfeec2d39d615986d280486678b57dc1aec6afc8d480a9`
and its integer/cast/scalar dependencies require explicit profiling before
freeze. Its SSE2 min/max (87–129) and floor/cast paths (61–71 and downstream
methods) cannot be replaced by generic Rust scalar arithmetic without Source
observations. Source compile also has an empty-stage early return (mod.rs:
395–404) producing a High/default-context pipeline before eligibility selection.
The native effective transform is a stored Transform with conditional assignment,
not an Option value. These are prerequisites, not native acceptance exceptions.

## Pinned evidence

The files below are under `C:/Users/think book/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/tiny-skia-0.12.0/src/`; all digests are full SHA-256 of the local pinned files.

| Source file | SHA-256 | Relevant lines |
| --- | --- | --- |
| `pipeline/mod.rs` | `fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4` | stage enum `68–150`; context `252–339`; builder/transform `347–375`; compile highp/lowp and tail variants `395–480`; run `501–542` |
| `pipeline/highp.rs` | `81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416` | stage table/start `24–200`; premultiply/clamp/seed/load/store `211–305`; transform `693–702`; spread/lookup `894–989`; all conical stages `1020–1137`; gamma stages `1139–1223`; mask conversion `1230–1242`; 8888 load/store/unnorm `1244–1344`; `mad` `1357–1359` |
| `pipeline/lowp.rs` | `3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88` | stage table `71–154`; start `164–217`; premultiply/seed/load/store `237–345`; transform/spread/lookup/radius `558–711`; f32→u16 `713–730`; 8888 store `787–836`; `div255` `848–854`; `mad` `899–902` |
| `pipeline/blitter.rs` | `a3189191d6ad4e1e03e738d21f5a26f310b7c00c33bae0021f0961054ad08f06` | fast reject/opacity decision `46–77`; anti-horizontal, rect and mask complete stage suffixes `79–194` |
| `shaders/gradient.rs` | `c8a87c87acb85764d5e13c95fd5f66531d24e2c145513e84adfa372d9a878847` | wrapper stage order `106–258` |
| `shaders/radial_gradient.rs` | `3ae883526cead4d5361f8b6974a91dece8f62fe3740a1712be8ce6219efdd74c` | radial context/pre/post stage selection `234–313` |
| `shaders/mod.rs` | `4f80ecb40d5a82985d97324748779cffd98252fca798c01c3f26969a1c27cbd3` | radial opacity conservatism and dispatch `63–92` |
| `shaders/pattern.rs` | `3a527adb4fad67fe147dec4d041233f35fce9fc985bdf085ab1afc4326323185` | contrasting named clamp insertion `164–165` |
| `color.rs` | `3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d` | ColorSpace destination/compression selectors `522–545` |
| `wide/f32x8_t.rs` | `e12514c4ebb32118dd6fd125f4973a3e6f8447e3cc564c98c01c461b75ec2429` | floor/normalize `45–65`; compare/blend/abs/max/min `75–175`; round/round_int/trunc_int/sqrt/vector powf `185–282` |
| `wide/f32x16_t.rs` | `a4907134acfaa60fbc06c324c8595259795fae7aecd740ad268ba439155c38a9` | split-lane abs, blend, normalize, floor/sqrt/round and u16 conversion `21–108` |
| `wide/u32x8_t.rs` | `7def946fed1b7a6d6f239249bb4450af79ce08ebd159370c89fd4b3584f94888` | unsigned lane bitcast/comparison `45–79`, addition `101–113`, bitwise masks `115–143` used by highp validity |
| `wide/u16x16_t.rs` | `fbbf313213582c8663471cb3739612840e9544d4a08ae78c1123fbbd2a6e8f54` | lowp integer addition `133–157`, bitwise combination `187–216`, shift `243–260` used by `div255` and packed color |

The present native `third_party/mermaid-rs-renderer/src/scene/prepare/gradient_program_types.rs` (SHA-256 `2cc186046caf23df7ae082b91127db30c76c12c00f56fefbb6c23a501159a176`) contains bool return, ordered `GradientStage`, optional effective transform as identity/default, two-stop/general contexts, radial p and eight zero mask lanes. `gradient_program.rs` (SHA-256 `00f29fede108d21fac1bc35200852d78aa56b21c8a6537f7440da3f87673267d`) is the preparation caller; it does not sample pixels. Existing Direct2D paint (`backends/direct2d/paint.rs`, SHA-256 `02c21b2d5ae5d8456ed24ee019b18f9399f8cd4d1ac361b02d53ee2986aa9abf`) still creates only a solid or native **linear** brush. These hashes describe inspected state, not M3h acceptance.

## Runtime path and precision selection

`Gradient::push_stages` emits `SeedShader`, optional `Transform`, radial pre-stages, spread, exactly one lookup, optional `Premultiply`, then radial post-mask (`gradient.rs:113–258`; `radial_gradient.rs:259–311`). A false return on transform inversion prevents the blitter from compiling/drawing this shader (`pipeline/blitter.rs:79–84,119–124,156–161`), even though SeedShader has already been appended to the builder. M3h preserves that partial preparation; a sampling consumer must honor the return flag.

`RasterPipelineBuilder::compile` tests **every final stage** via the actual lowp function-pointer table: `force_hq_pipeline || any(lowp::STAGES[stage] == null_fn)` chooses the **entire** highp program; otherwise the entire program uses lowp (`pipeline/mod.rs:395–480`). The compiler appends `just_return` and substitutes tail load/store/source-over handlers; `RasterPipeline::run` dispatches the chosen kind (`:501–542`). Highp has 8 lanes, lowp 16, with separate full-width and tail execution (`highp.rs:24,145–200`; `lowp.rs:37,164–217`). A GPU consumer cannot choose precision from M3f `geometry_lowp_compatible` alone. Plain radial `XYToRadius` is lowp-capable, but concentric bias, every Strip/Focal stage and masks are lowp `null_fn` (`lowp.rs:124–144`). Nonlinear destination gamma stages and several blend stages are also `null_fn` (`:71–154`), and `force_hq_pipeline` always overrides eligibility.

The final list differs among anti-horizontal coverage, rectangle and mask blits (`pipeline/blitter.rs:79–194`). After shader stages it may add `MaskU8`, `Scale1Float`/`ScaleU8` **or** post-blend `Lerp1Float`/`LerpU8`, `LoadDestination`, selected destination gamma expansion, a blend stage, output gamma compression and `Store`. Rectangle SourceOver without a mask has a separate `SourceOverRgba` path; `Source` can omit destination loading/blend (`:119–153`). `Shader::is_opaque` is always false for radial because some conical positions are undefined (`shaders/mod.rs:63–75`), affecting SourceOver reduction in `blitter.rs:57–61`; M3g `colors_are_opaque` only controls the shader's premultiply stage. Previously accepted scene blend modes do not themselves establish this pipeline's coverage/load/compress/store order.

## Every gradient-program stage actually sampled

The Source highp/lowp stage tables align with enum order (`highp.rs:58–139`; `lowp.rs:71–154`). Unless a row says otherwise, t is held in highp `p.r`, with transformed y in `p.g`; lowp packs f32x16 x/y in pairs of u16x16 registers until lookup (`lowp.rs:255–266,558–657`). The original numeric order matters even if an algebraic simplification looks equivalent.

| Frozen stage / concern | Original highp operation and lines | Original lowp operation and lines |
| --- | --- | --- |
| `SeedShader` | Seed `(dx + [0.5..7.5], dy+0.5)`, set blue to one, alpha and destination registers to zero (`highp.rs:256–269`). | Seed 16 pixel centers `[0.5..15.5]`, split f32x16 coordinates across registers (`lowp.rs:255–266`). |
| `Transform` | Nested `mad(x,sx,mad(y,kx,tx))` and `mad(x,ky,mad(y,sy,ty))`; `mad(f,m,a)=f*m+a` (`highp.rs:693–702,1357–1359`). | Same f32x16 mathematical order via `join`, nested `mad`, then `split` (`lowp.rs:558–571,899–902`). |
| `PadX1` | `t.normalize()` (`highp.rs:894–899`). | f32x16 normalize before split (`lowp.rs:573–579`). |
| `ReflectX1` | `abs((t-1)-2*floor((t-1)*0.5)-1).normalize()`, with `two(v)=v+v` (`highp.rs:900–908,1351–1354`). | Same expression through f32x16 and `two(x)=x+x`, then normalize (`lowp.rs:581–592`). |
| `RepeatX1` | `(t-floor(t)).normalize()` (`highp.rs:910–914`). | Same expression in f32x16 (`lowp.rs:594–600`). |
| `EvenlySpaced2StopGradient` | Per channel `mad(t, factor, bias)` in f32x8; leaves unpremultiplied f32 RGBA (`highp.rs:980–989`). | Same f32x16 `mad` then `round_f32_to_u16` to byte-valued lanes (`lowp.rs:634–647,713–730`). |
| `Gradient` | For each `i=1..ctx.len`, increment lane index on **`t >= t_values[i]`**, gather factor/bias by that index, then per-channel `mad(t,F,B)` (`highp.rs:916–978`). NaN t makes each ordered `>=` false. | Same index rule in 16 f32 lanes with u16 indices, gather vectors, `mad`, then `round_f32_to_u16` (`lowp.rs:602–632,659–730`). Only `len`, not padded factor/bias length, controls comparisons. |
| `Premultiply` | Float RGB `*=alpha` after lookup (`highp.rs:211–217`). | Integer RGB `div255(rgb*alpha)` after lookup's byte rounding (`lowp.rs:237–243,848–854`). |

M3h's context contains unpremultiplied raw f32 factors/biases that may exceed `[0,1]` or be nonfinite; early context clamping is not equivalent to Source lookup/store (`pipeline/mod.rs:281–323`; `gradient.rs:199–250`). Pad is omitted for nonuniform stops, so general `Gradient` search handles out-of-range t and duplicate hard stops. Two-stop selection is based on stop count, not uniformity. Store quantization is **not** part of M3h context and differs between highp and lowp.

## All 13 radial stages and masks

M3f selects their order, while highp executes them (`highp.rs:1020–1137`). Source `TwoPointConicalGradientCtx` starts with zero `mask` and prepared `p0/p1` (`radial_gradient.rs:234–257`; `pipeline/mod.rs:333–339`). The lowp table contains only `XYToRadius`; the remaining 12 radial entries are `null_fn` (`lowp.rs:132–144`).

| Radial stage | Exact highp lane operation |
| --- | --- |
| `XYToRadius` | `sqrt(x*x+y*y)` (`:1020–1026`); lowp has the same f32x16 expression (`lowp.rs:649–657`). |
| `ApplyConcentricScaleBias` | `t=t*p0+p1` with the separate multiply/add expression (`highp.rs:1129–1137`). |
| `XYTo2PtConicalStrip` | `t=x+sqrt(p0-y*y)` (`:1066–1074`). |
| `Mask2PtConicalNan` | `is_degenerate=(t != t)`, replace t by zero for true lanes; set mask to all-one bits for valid lanes, zero for invalid via `cond_to_mask` (`:1076–1085,1230–1242`). Negative non-NaN t is retained. |
| `XYTo2PtConicalFocalOnCircle` | `t=x+y*y/x` (`:1028–1034`). |
| `XYTo2PtConicalWellBehaved` | `t=sqrt(x*x+y*y)-x*p0` (`:1036–1044`). |
| `XYTo2PtConicalSmaller` | `t=-sqrt(x*x-y*y)-x*p0` (`:1056–1064`). |
| `XYTo2PtConicalGreater` | `t=sqrt(x*x-y*y)-x*p0` (`:1046–1054`). |
| `Mask2PtConicalDegenerates` | Invalid when `t<=0` **or** `t!=t`; replace t by zero and set validity mask by bit conversion (`:1087–1096`). Negative infinity is invalid; positive infinity is not excluded by this predicate. |
| `NegateX` | `t=-t` (`:1123–1127`). |
| `Alter2PtConicalCompensateFocal` | `t=t+p1` (`:1109–1115`). |
| `Alter2PtConicalUnswap` | `t=1.0-t` (`:1117–1121`). |
| `ApplyVectorMask` | Bitwise AND the saved u32 mask into each float RGBA lane **after** spread, lookup and optional premultiply (`:1098–1107`). Replacing invalid t by zero without this stage would incorrectly show the first stop. |

The vector mask's initial zero value is not an instruction to clear ordinary Radial/well-behaved Focal: those branches omit `ApplyVectorMask`. Strip and non-well-behaved Focal first set mask in the corresponding pre-stage and apply it only at the post-color position (`radial_gradient.rs:263–311`). NaN, infinities, signed zero and negative discriminants must follow these operations and comparisons; this audit supplies no alternate clipping rule.

## Vector numeric and final pixel boundary

`f32x8::normalize` is max-with-zero then min-with-one (`wide/f32x8_t.rs:63–65`), not a Source `NormalizedF32` clamp. Its AVX and fallback `max/min` implementations explicitly note differences for NaN and signed zero (`:153–175`); `cmp_ne` is unordered true on NaN in AVX (`:85–94`), and `blend` is sign-mask selection (`:137–145`). Highp masks convert boolean lanes to all-one/all-zero u32 words and bitcast RGBA for masking (`highp.rs:1076–1107,1230–1242`). `f32x8::floor` uses truncation/compare fallback in some target configurations, and `round_int`/`trunc_int` note NaN/out-of-range differences between AVX and fallback (`wide/f32x8_t.rs:45–57,195–218`). `f32x16::floor` uses round-plus-compare instead (`wide/f32x16_t.rs:69–76`). Therefore Source observation must pin actual build features/target ISA and record raw lane/pixel output; a generic scalar `floor`, `min`, or `powf` is not an observed substitute.

Source vector `f32x8::powf` (`wide/f32x8_t.rs:251–282`) is **not** M3h's scalar `math::approx_powf`: it derives exponent/mantissa from lane bitcasts, runs vector polynomial arithmetic, clamps/rounds into bit patterns and selects the original lane for zero/one. The post-blend highp gamma `GammaExpand22`, `GammaCompress22`, `srgb_expand` and `srgb_compress` use this vector powf (`highp.rs:1163–1223`). `srgb_expand` compares `<=0.04045` and evaluates both linear and power expressions before a mask blend; compression compares `<=0.0031308` with `x*12.92` or `x.powf(0.416666666)*1.055-0.055` (`:1187–1199`). Gamma2 expansion squares RGB and compression takes sqrt (`:1139–1161`). Alpha is untouched by gamma. The `ColorSpace::expand_dest_stage` and `compress_stage` mode mappings are in `color.rs:530–545`; `expand_stage` `:522–529` is a separate selector and is **not** called by this gradient wrapper/blitter path. All gamma entries are lowp `null_fn` (`lowp.rs:145–153`). Destination/load/blend order must be read with the three blitter suffixes above, not inferred solely from `GradientProgram`.

`Clamp0` and `ClampA` are implemented by highp as separate max-zero/min-one stages (`highp.rs:228–244`) and lowp marks them `null_fn` (`lowp.rs:75–76`), but the Source gradient wrapper/blitter above do **not** append those named stages; Source pattern shader does (`shaders/pattern.rs:164–165`). Gradient outputs are instead clamped at the relevant quantization/store boundary. Highp `unnorm` does `max(0).min(1)*255` then `round_int`, and 8888 store casts those i32 lanes to u8 (`highp.rs:1295–1344`). Lowp lookup calls `round_f32_to_u16`: RGB `normalize()*255+0.5`, alpha `alpha*255+0.5` **without that RGB normalize**, then `f32x16::save_to_u16x16` uses Rust float-to-u16 casts (no extra rounding), and lowp 8888 store casts u16 to u8 (`lowp.rs:713–730,787–836`; `wide/f32x16_t.rs:87–108`). Lowp integer premultiply uses `(v+255)>>8` (`lowp.rs:848–854`), not highp float multiplication. Full/tail store variants differ in addressing and lane count (`highp.rs:282–289,1295–1339`; `lowp.rs:279–286,787–836`). Final comparisons need premultiplied RGBA bytes and visible bounds/tails, not merely a mathematical t/color formula.

## Source-only observation seam for root to freeze

The previous M3h oracle observes program/context preparation. A later independent observer must **execute original Source stage bodies**. The narrow original seams are `RasterPipelineBuilder::compile`/`RasterPipeline::run` (`pipeline/mod.rs:395–542`) plus `highp::start` or `lowp::start` (`highp.rs:145–200`, `lowp.rs:164–217`), supplied with the actual Source gradient and paint/blitter context. Read-only observer/export additions in a pristine, hash-guarded Source copy could report: the emitted complete stage list and chosen pipeline kind; seed/transformed x/y; t after each radial stage and spread; saved mask bits; general lookup indices and f32 RGBA or lowp rounded u16 RGBA; post-mask/premultiply values; destination gamma/blend/compression; and final premultiplied 8888 bytes. Such observations must be captured from execution of the original functions rather than an agent reimplementation of their equations. Running full/tail widths, both highp/lowp and feature/ISA configurations is necessary to expose the vector and quantization boundaries. The root must decide exact observer design, frozen inputs, error semantics and acceptance.

Independent case families follow Source branches: M3h programs with all four color spaces and spreads; two-stop and hard/multi-stop contexts; plain/concentric Radial, Strip, all Focal paths and all validity masks; pixels on both sides of t=0/1 and focal/discriminant boundaries; NaN/±infinity/±0/subnormal f32 stage inputs where Source can produce them; force-HQ on/off; all three blitter variants, destination colors, alpha/coverage/mask, blend modes, full and tail lengths. Some raw nonfinite values cannot be constructed through safe `Color` or scene builders, while stage arithmetic can still produce them. Root's oracle should distinguish raw stage observation from valid paint-path pixels. No agent-created fixture values, substituted scalar equations, raster CPU fallback, D2D brush approximation or SVG reparse is an acceptance oracle.
