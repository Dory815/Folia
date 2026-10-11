# M3i5 pinned tiny-skia pattern sampling: read-only Source audit

Status: prerequisite for Root's borrowed-input/API freeze and actual Source oracle. Only this report is added. No native function, fixture, manifest, build, test, commit, pixel result or acceptance is supplied here. The source is pristine `tiny-skia 0.12.0` under `C:/Users/think book/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/tiny-skia-0.12.0/src/`; the pinned execution profile remains `std+simd` x64 MSVC.

## Source identity and actual ordinals

| Original file | SHA-256 | Relevant lines |
|---|---|---|
| `pipeline/mod.rs` | `fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4` | 70–150, 154–169, 252–270, 341–345, 370–375, 395–542 |
| `pipeline/highp.rs` | `81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416` | 24–47, 59–110, 146–199, 309–331, 693–892, 1245–1292 |
| `pipeline/lowp.rs` | `3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88` | 72–125, 165–217 |
| `shaders/pattern.rs` | `3a527adb4fad67fe147dec4d041233f35fce9fc985bdf085ab1afc4326323185` | 17–25, 68–186 |
| `shaders/mod.rs` | `4f80ecb40d5a82985d97324748779cffd98252fca798c01c3f26969a1c27cbd3` | 25–39, 50–93, 95–110, 127–144 |
| `pipeline/blitter.rs` | `a3189191d6ad4e1e03e738d21f5a26f310b7c00c33bae0021f0961054ad08f06` | 19–35, 79–211, 263–384 |
| `pixmap.rs` | `484d49d1f4c3c3c8fb3fab3e1b1535292de828c0f258c3cab9bda02c7d9626fc` | 306–377 |
| `wide/f32x8_t.rs` | `e12514c4ebb32118dd6fd125f4973a3e6f8447e3cc564c98c01c461b75ec2429` | 12–29, 45–65, 153–176 |
| `wide/f32x4_t.rs` | `d4f6b4b67900f2bb38dfeec2d39d615986d280486678b57dc1aec6afc8d480a9` | 16–24, 87–132 |
| `wide/i32x8_t.rs` | `7bfc17eacd1442e2008a3e213f04118d7c33150f5902091a59b50340e30cfe06` | integer vector index operations used by `gather_ix` |

The original `Stage` order (`pipeline/mod.rs:70–150`) gives **Gather=11, Transform=46, Reflect=47, Repeat=48, Bilinear=49, Bicubic=50**. `highp::STAGES` names actual functions at `highp.rs:59–110`; `lowp::STAGES` has `null_fn` for 11 and 47–50 (`lowp.rs:84,120–123`). Any nonempty compiled program containing any of those five therefore chooses highp, regardless of otherwise lowp-compatible stages (`pipeline/mod.rs:406–480`); `lowp::start` has no `pixmap_src` argument (`mod.rs:526–539`). `Transform` 46 exists in both precisions and was previously accepted for gradient execution, but pattern-specific inverse-transform production and resource binding remain separate. **PadX1=51, ReflectX1=52, RepeatX1=53 are gradient one-dimensional tile stages**, not the pattern x/y image stages; no `Pad` image stage exists. Nearest Pad relies on `Gather`'s clamp (`pattern.rs:139–145`).

## Complete pattern producer sequence

`Pattern<'a>` retains borrowed `PixmapRef<'a>`, filter quality, spread mode, normalized opacity and transform (`pattern.rs:68–94`). `Pattern::new` clamps opacity with `NormalizedF32::new_clamped`; `Shader::transform` post-concatenates a supplied transform into pattern transform (`shaders/mod.rs:95–110`), while `Shader::apply_opacity` multiplies and bounds it (`shaders/mod.rs:127–144`). `Shader::push_stages` delegates to `Pattern::push_stages` and returns its boolean, where false means draw nothing (`shaders/mod.rs:79–93`).

`Pattern::push_stages` **first** inverts the pattern transform; failure warns and returns false before `SeedShader` (`pattern.rs:97–106`). Success pushes `SeedShader` and calls `RasterPipelineBuilder::push_transform(inverse)`, which appends `Transform` and stores `ctx.transform` only for finite nonidentity input (`pipeline/mod.rs:370–375`). Source then copies quality and demotes it to Nearest when inverse is identity or translation (`pattern.rs:110–114`). It still contains a subsequent Bilinear/integer-translation test (`116–123`); retain the actual branch/order in any producer transplant even though the prior translation demotion appears to make that nested case redundant. No mipmapping/minification stage exists (`125`). The three resulting routes are:

| Resulting quality | Source stage sequence and Context writes | Exact Source |
|---|---|---|
| Nearest | Writes `limit_x={scale:width as f32, inv_scale:1.0/(width as f32)}` and corresponding height to `limit_y`. Pad adds no stage; Repeat adds 48; Reflect adds 47. Then adds Gather 11. | `pattern.rs:127–146`; `mod.rs:341–345` |
| Bilinear | Writes `sampler={spread_mode,inv_width:1.0/(width as f32),inv_height:1.0/(height as f32)}`; adds Bilinear 49. | `pattern.rs:147–154`; `mod.rs:265–270` |
| Bicubic | Writes same sampler; adds Bicubic 50, then Clamp0 2 and ClampA 3 because filter overshoots. | `pattern.rs:155–167` |

After the route, opacity other than one writes `ctx.current_coverage` and appends Scale1Float 16; then `ColorSpace::expand_stage` may append a gamma expansion, **after sampling and opacity**, and returns true (`pattern.rs:169–186`). The existing Root-accepted Stage 6 seed, 46 transform, 16 coverage, 2/3 clamp, and color expansion bodies do not by themselves construct this pattern program. Pattern is always reported nonopaque by `Shader::is_opaque` (`shaders/mod.rs:63–74`), influencing blitter fast paths; do not infer source-image opacity from bytes for this branch.

## Highp sampling bodies and all direct dependencies

All stages use highp's `f32x8` coordinate registers `r=x`, `g=y`; highp `Pipeline` additionally borrows `PixmapRef`, mutable destination and Context, with `tail`, `dx`, `dy` (`highp.rs:24–47`). Source `highp::start` invokes full vectors of eight and a tail vector of the same width (`146–199`). Only destination load/store/fused stages have alternate tail functions (`mod.rs:419–439`), so pattern sampling itself executes **all eight lanes even for a tail**. The later destination store decides which lanes write.

| Stage/helper | Reads / writes and exact numeric order | Source |
|---|---|---|
| Gather 11 | `gather_ix(pixmap_src,r,g)` then `PixmapRef::gather(ix)` then `load_8888` to source RGBA registers. `gather_ix` obtains exclusive width/height as `ulp_sub(width/height as f32)`, clamps `x,y` with vector `.max(0).min(ulp_sub)`, converts each via `trunc_int`, and computes `(y * width + x)` in `i32x8`, bitcasts to `u32x8`. `ulp_sub` subtracts 1 from the float bit pattern; do not substitute `width-epsilon` or a scalar floor/round. | `highp.rs:309–331`; `mod.rs:154–169`; `highp.rs:1245–1280` |
| Reflect 47 | Reads `ctx.limit_x.{scale,inv_scale}` to replace `r`, then `ctx.limit_y` to replace `g`; `exclusive_reflect` uses `((v-limit)-(limit+limit)*floor((v-limit)*(inv_limit*0.5))-limit).abs()`. No source-pixel read yet. | `highp.rs:708–724` |
| Repeat 48 | Same x then y fields/order; `exclusive_repeat(v,limit,inv_limit)=v-floor(v*inv_limit)*limit`. No source-pixel read yet. | `highp.rs:726–739` |
| Bilinear 49 | Takes `x=r,y=g`, computes `(x+0.5).fract()` and `(y+0.5).fract()` through provider's `floor`, forms weights `[1-f,f]`; calls `sampler_2x2` and replaces source RGBA. Tap walk begins at `(cx-0.5,cy-0.5)`, y-major outer/two x taps inner; each sample tiled/clamped/gathered, weighted by `wx[i]*wy[j]`, accumulated with Source `mad` order. | `highp.rs:741–753,797–833` |
| Bicubic 50 | Same fractions; weights `[far(1-f),near(1-f),near(f),far(f)]` via original nested `mad` polynomial constants. Tap walk begins `(cx-1.5,cy-1.5)`, y-major outer/four x taps inner; 16 tiled/clamped gathers, weighted RGBA accumulation. Its caller, not the stage body, adds Clamp0/ClampA. | `highp.rs:755–795,835–871`; `pattern.rs:155–167` |
| `sample`, `tile`, `gather_ix`, `PixmapRef::gather`, `load_8888` | `sample` tiles x and y **per tap** from `ctx.sampler.spread_mode`, source width/height and inverse dimensions; Pad leaves coordinates, Repeat/Reflect call the exclusive helpers; then `gather_ix` hard-clamps and loads premultiplied byte RGBA scaled by `1.0/255.0`. Nearest Repeat/Reflect instead tile once before Gather with `limit_x/y`. | `highp.rs:873–892,317–331,1245–1280`; `mod.rs:154–169` |

The arithmetic depends on original `f32x8`/`f32x4` min/max, floor, fract, integer truncation and `mad` (`f*m+a`, not a mandated fused instruction). SSE min/max can differ from scalar Rust for NaN and signed zero (`wide/f32x8_t.rs:153–176`, `f32x4_t.rs:87–132`); nonfinite coordinates must be observed rather than normalized by a new API. The final hard clamp is required even after Repeat/Reflect because tiling is exclusive and sampling converts to discrete pixel indices (`highp.rs:704–706,317–325`). `PixmapRef::gather` reads all eight computed indices separately and in lane order (`mod.rs:154–169`); a weighted filter may visit the same pixel repeatedly, which does not license reordering floating-point accumulation.

## Borrowed source image and native interface requirements (not a frozen API)

`PixmapRef<'a>` stores immutable `&'a [u8]` and `IntSize`; `from_bytes` rejects zero/invalid size and insufficient storage, expects **premultiplied RGBA byte order**, and permits storage at least the required size (`pixmap.rs:306–325`). `width`, `height`, `data`, `pixels` expose a contiguous row-major image; gather index is `y*width+x` in pixels with **no independent source row stride** (`pixmap.rs:338–377`, `mod.rs:154–169`). Any shared typed consumer boundary must therefore preserve a live immutable source byte borrow, validated nonzero width/height and length, source premultiplication/byte order, actual `width` and `height` separately from mutable destination dimensions/stride, and exact Context state: `limit_x`, `limit_y`, `sampler.spread_mode`, `sampler.inv_width/height`, inverse `transform`, and opacity coverage. The source borrow must outlive all three compiled blitter pipelines and every run. It must not alias mutable destination storage unsafely; an image snapshot/copy, if needed for self-draw, is a producer decision requiring separate Source parity evidence. This paragraph identifies required data/ownership, **not** a proposed Rust signature or validation policy.

Current native `sampling/resources/types.rs` owns only borrowed **destination**/mask/AA contexts, uniform color and coverage; `gradient_program_types.rs` owns gradient transforms and stops. Neither currently binds `PixmapRef` bytes/dimensions, `TileCtx` x/y, or `SamplerCtx`, and the existing `sampling/highp.rs` `Transform` is consumed through `GradientProgram`. These are the concrete missing fields and producer integration seams. The shared `pipeline_control_types.rs` already names all five ordinals and M3i1 whole-sequence precision selection; no new stage values or lowp pattern substitutes are needed. Root must decide any native context/borrow arrangement after original Source observation; this audit does not freeze it.

## Actual blitter relationship and acceptance limit

`RasterPipelineBlitter::new` calls `paint.shader.push_stages` **three times**, one for anti-horizontal, rectangle and AA-mask programs, returning None on any false (`pipeline/blitter.rs:79–193`). Each then appends optional clip `MaskU8`, coverage branch, destination load/blend/color-space/store according to paint. It binds `pixmap_src` to `Pattern.pixmap`, otherwise a 1×1 dummy (`196–200`), and retains that borrowed source alongside mutable destination (`19–27,202–211`). `blit_anti_h`, `blit_rect`, `blit_mask` pass the same source to `RasterPipeline::run` (`263–384`), which passes it only to highp start (`pipeline/mod.rs:501–542`). Fast rejects, source-over reduction, memset and clip/AA paths remain separate blitter concerns (`blitter.rs:36–77,334–382`). A standalone Gather/Bilinear/Bicubic CPU stage can prove its own numerical behavior under observed inputs, but cannot replace the Source three-path paint/blitter assembly, complete typed scene, SVG/D2D consumer rendering, or Folia acceptance.
