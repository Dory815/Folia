# M3i5 borrowed pattern-stage implementation handoff

Status: Sol implementation awaits Root's independent 4,032-case original Source register comparison, effective mutations, constructor boundary review, and native regression matrix. This packet accepts neither the pattern producer nor complete blitter, SVG/Direct2D pixel rendering, shared scene, or Folia wiring. Only `sampling/pattern/highp.rs` and this report were edited; no build/test/commit was run.

The pinned original is tiny-skia 0.12.0 `src/pipeline/highp.rs`, SHA-256 `81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416`. The body retains original copyright with the repository's `LICENSE-tiny-skia-blend` notice. It uses the Root-preserved `sampling/wide::{f32x8,i32x8,u32x8}` and Root-frozen borrowed `PatternContext`/`SourcePixmap` APIs; no alternative float, pixel buffer or platform path was introduced.

| Original Source operation | Delivered mapping |
|---|---|
| `gather` 309–314 | `Gather` 11 dispatches the original body using borrowed `SourcePixmap`; all eight lanes are evaluated, including tail invocations. |
| `gather_ix`, `ulp_sub` 316–331 | Transplanted unchanged vector clamp/truncation/index arithmetic and float-bit decrement; only the pixmap parameter type becomes frozen `SourcePixmap<'_>`. |
| `PixmapRef::gather` in `pipeline/mod.rs:154–169` | Private `gather_pixels` views the immutable `SourcePixmap.data()` as RGBA `[u8;4]` pixels through `bytemuck::cast_slice`, then indexes lanes 0→7 in the original order. It uses validated source width/height and no extra source stride. The cast retains the original typed-view behavior for excess bytes. |
| `reflect`, `exclusive_reflect` 708–724 | `Reflect` 47 reads frozen `limit_x`, then `limit_y`; original f32x8 expressions and order retained. |
| `repeat`, `exclusive_repeat` 726–739 | `Repeat` 48 reads x and y tile contexts in Source order, without recomputing inverse limits. |
| `bilinear` 741–753, `sampler_2x2` 797–833 | `Bilinear` 49 keeps `(coord+0.5).fract()` weights, y-major/two-x-tap loop, per-tap `sample`, and channel `mad` accumulation order. |
| `bicubic`, `bicubic_near`, `bicubic_far` 755–795, `sampler_4x4` 835–871 | `Bicubic` 50 keeps both original polynomial forms and constants, y-major/four-x-tap loop and 16 per-tap accumulations. Clamp0/ClampA remain caller stages 2/3. |
| `sample`, `tile` 873–892 | Per-tap Pad/Repeat/Reflect uses frozen `SamplerContext` spread and cached inverse dimensions, then the original `gather_ix` hard clamp and borrowed gather. The only type adaptation is `SamplerCtx` → `SamplerContext`. |
| `load_8888` 1245–1280, `mad` 1356–1359 | Pixel component getters map directly to `[R,G,B,A]` byte indices; eight scalar byte→f32 multiplications per channel retain original order and `1.0/255.0` constant. `mad` remains `f*m+a`, not an explicit fused operation. |

The private eight-register `Pipeline` adapter holds the exact `r,g,b,a,dr,dg,db,da` values; its no-op `next_stage` is the frozen one-stage boundary. Unsupported ordinals error before register conversion or state mutation. Source image/context are only borrowed/read; destination and external storage are untouched by these five stages. Pattern quality demotion, transform inversion, opacity/color-space stage assembly, three original blitter paths, final destination writes, and backends remain separate required work.
