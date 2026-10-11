# M3i4 tiny-skia pure blend stages: read-only Source audit

Status: prerequisite for Root's freeze and independently executed Source comparison. No API, implementation, fixture, build or acceptance decision is made here. M3i3 borrowed resources and M3i2 arithmetic stages were accepted separately; this report concerns only original stage ordinals **18–44**, not fused `SourceOverRgba` ordinal 45 or scene-level Direct2D blend effects.

## Pinned evidence and execution profile

All locations are in the pristine registry crate `tiny-skia-0.12.0/src/`, under `C:/Users/think book/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/`. SHA-256: `pipeline/mod.rs` `fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4`; `pipeline/highp.rs` `81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416`; `pipeline/lowp.rs` `3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88`; `wide/u16x16_t.rs` `fbbf313213582c8663471cb3739612840e9544d4a08ae78c1123fbbd2a6e8f54`; `wide/f32x8_t.rs` `e12514c4ebb32118dd6fd125f4973a3e6f8447e3cc564c98c01c461b75ec2429`; `wide/f32x4_t.rs` `d4f6b4b67900f2bb38dfeec2d39d615986d280486678b57dc1aec6afc8d480a9`; `pipeline/blitter.rs` `a3189191d6ad4e1e03e738d21f5a26f310b7c00c33bae0021f0961054ad08f06`; `blend_mode.rs` `841dcc8eb96fe11fc2316fa0a4ff47c29fd837990ce8c1d8be5fcf08f6c86d19`.

The pinned `std+simd` x64 MSVC profile uses `f32x8` as two `f32x4` SSE vectors unless AVX is explicitly enabled; `wide/f32x8_t.rs:12–29,153–176` and `f32x4_t.rs:16–24,87–132` decide min/max behavior. `u16x16` remains a scalar `[u16;16]` on x64 (`u16x16_t.rs:15–22,24–45,133–175`); its `+`, `-`, `*` delegate to each lane's **ordinary** `u16` operation, so checked debug overflow and release wrapping are profile-sensitive. The AArch64 NEON branch is not the pinned profile. Highp blend arithmetic uses original `f32x8` operations and `recip_fast` where called; scalar f32 or host graphics blend formulas are not an equivalent observation source. `highp.rs:1347–1359` defines `inv(v)=1-v`, `two(v)=v+v`, `mad(f,m,a)=f*m+a`; `lowp.rs:848–868` defines `div255(v)=(v+255)>>8`, `inv(v)=255-v`, and lowp lerp. The original complete-pipeline precision gate tests the final lowp pointer table and force-high flag (`mod.rs:395–480`); one stage does not select precision itself.

## Exhaustive stage ledger

`mod.rs:89–115` fixes these names and ordinals; `highp.rs:58–139` and `lowp.rs:71–154` fix pointer support. All highp entries are executable. Lowp is executable except seven explicit `null_fn` entries (`32,33,40–44`), which select highp when compiled in a nonempty sequence. Each row below gives the actual Source function and defining line(s), not a substitute formula.

| Ordinal / Stage | highp function and lines | lowp pointer/function and lines | Specific dependency/behavior |
|---|---|---|---|
| 18 DestinationAtop | `destination_atop` 420 | `destination_atop` 479 | `blend_fn`; reads `s,d,sa,da` |
| 19 DestinationIn | `destination_in` 422 | `destination_in` 481 | `blend_fn`; destination scaled by source alpha |
| 20 DestinationOut | `destination_out` 424 | `destination_out` 483 | `blend_fn`; `inv(sa)` |
| 21 DestinationOver | `destination_over` 426 | `destination_over` 485 | `blend_fn`; highp `mad`, lowp `div255` |
| 22 SourceAtop | `source_atop` 419 | `source_atop` 478 | `blend_fn`; `inv(sa)` |
| 23 SourceIn | `source_in` 421 | `source_in` 480 | `blend_fn`; source scaled by destination alpha |
| 24 SourceOut | `source_out` 423 | `source_out` 482 | `blend_fn`; `inv(da)` |
| 25 SourceOver | `source_over` 425 | `source_over` 484 | `blend_fn`; register-only, unlike fused ordinal 45 |
| 26 Clear | `clear` 418 | `clear` 477 | `blend_fn`; zero source output |
| 27 Modulate | `modulate` 427 | `modulate` 486 | `blend_fn`; source×destination on all channels |
| 28 Multiply | `multiply` 428 | `multiply` 487 | `blend_fn`; three premultiplied terms |
| 29 Plus | `plus` 433 | `plus` 492 | `blend_fn`; highp min 1, lowp min 255 |
| 30 Screen | `screen` 429 | `screen` 488 | `blend_fn`; source+dest−product |
| 31 Xor | `xor` 430 | `xor` 489 | `blend_fn`; cross inverse-alpha terms |
| 32 ColorBurn | `color_burn` 454–462 | **`null_fn`** 105 | `blend_fn2`; highp comparisons + `recip_fast`; no lowp body |
| 33 ColorDodge | `color_dodge` 464–472 | **`null_fn`** 106 | `blend_fn2`; highp comparisons + `recip_fast`; no lowp body |
| 34 Darken | `darken` 449 | `darken` 509 | `blend_fn2`; max of alpha-weighted products |
| 35 Difference | `difference` 451 | `difference` 513–514 | `blend_fn2`; twice the min product |
| 36 Exclusion | `exclusion` 452 | `exclusion` 511 | `blend_fn2`; twice source×destination |
| 37 HardLight | `hard_light` 474–479 | `hard_light` 516–523 | `blend_fn2`; source threshold, eager integer candidates in lowp |
| 38 Lighten | `lighten` 450 | `lighten` 510 | `blend_fn2`; min of alpha-weighted products |
| 39 Overlay | `overlay` 481–486 | `overlay` 525–532 | `blend_fn2`; destination threshold, eager integer candidates in lowp |
| 40 SoftLight | `soft_light` 488–504 | **`null_fn`** 113 | `blend_fn2`; highp `sqrt` plus three-way selection |
| 41 Hue | `hue` / `hue_k` 528–549 | **`null_fn`** 114 | `blend_fn3`; `sat`, `set_sat`, `set_lum`, `clip_color` |
| 42 Saturation | `saturation` / `saturation_k` 551–572 | **`null_fn`** 115 | `blend_fn3`; same helper family, different source/dest channels |
| 43 Color | `color` / `color_k` 574–594 | **`null_fn`** 116 | `blend_fn3`; `set_lum`, `clip_color` |
| 44 Luminosity | `luminosity` / `luminosity_k` 596–616 | **`null_fn`** 117 | `blend_fn3`; `set_lum`, `clip_color` |

`blend_fn` in highp `405–416` and lowp `464–475` evaluates **R, then G, then B, then A** and writes the source registers, leaving destination registers intact. For each channel it reads the current source/destination alpha; alpha is replaced last. `blend_fn2` highp `435–447` / lowp `495–507` applies its color closure to RGB, but alpha is *always* source-over (`highp mad(da,inv(a),a)`; lowp `a+div255(da*inv(a))`). `blend_fn3` highp `514–526` calculates all four output channels together and has no lowp counterpart. The nonseparable helper order is `sat` 618–621, `lum` 623–626, `set_sat` 628–641, `set_lum` 643–649, `clip_color` 651–667; notably `clip_color`'s first SIMD selection is literally `mx.cmp_ge(0).blend(c, adjusted)`, so the true branch retains `c`. Eager computation and original SIMD min/max/NaN semantics matter for raw register parity. None of these 27 stages dereferences a pixmap, mask, coverage context or a gradient; they operate on the already populated source and destination register banks.

## Lowp valid-premultiplied range and checked-overflow boundary

The normal RGBA-pixel path supplies byte channels in `0..=255`, with `r,g,b <= a` for both source and destination. The public premultiplied-byte constructor enforces that relationship while the unchecked constructor and controlled raw-register observer can bypass it (`color.rs:95–123`, SHA-256 `3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d`). Original lowp `LoadDestination` widens each byte to u16 (`lowp.rs:269–276,741–772`), while shader/coverage stages may alter the source before blending. Lowp formulas rely on the intended byte/premultiplied range: `inv(v)=255-v` is only nonnegative when alpha ≤255; `div255` assumes a u16 intermediate that has not overflowed. Raw arbitrary `u16` inputs are therefore a separate observer domain, not evidence that a valid paint path requires clipping or wrapping.

Even the valid premultiplied range does **not** by itself prevent debug overflow in lowp `HardLight`/`Overlay`. Their `cmp_le(...).blend(candidate_true,candidate_false)` constructs **both** candidates before selection (`lowp.rs:516–532`; `u16x16_t.rs:120–123`). With `s=d=sa=da=255`, the `2*s*d` candidate computes 130050, exceeding u16 even though the other candidate may be selected. With `s=d=0, sa=da=255`, `2*(sa-s)*(da-d)` similarly exceeds u16; the subsequent subtraction can also be negative if evaluated without prior overflow. These are deductions from the exact Source expression and ordinary x64 scalar u16 operators, **not** observations from an executed fixture in this audit. A debug Source observer must record/handle such panics according to the original profile; silently changing to saturating, widened, wrapping or branch-lazy arithmetic would change Source behavior. Other lowp modes likewise require original operand order and range assumptions; for arbitrary raw-register input, any `u16` addition, subtraction, or multiplication may trigger checked overflow in debug or wrap in release. The ordinary premultiplied bound makes the simple Porter-Duff, modulate, screen, plus, darken/lighten, difference/exclusion and multiply expressions fit their intended `div255` domain, but that is a range argument, not a blanket promise for malformed raw input or intermediate shader overshoot.

## Consumer relationship and remaining gates

Source `BlendMode::to_stage` maps real paint modes to these stages (`blend_mode.rs:94–126`), except `Source` has no stage; `Destination` and a solid/opaque `DestinationIn` may fast-reject, opaque `SourceOver` can reduce to `Source`, and solid `Source`/Clear may use memset (`pipeline/blitter.rs:46–77`). The original blitter separately builds anti-horizontal, rectangle and AA-mask pipelines (`blitter.rs:79–193`). It inserts `MaskU8` after shader if present; chooses coverage **pre-scale** versus **post-blend lerp** using `should_pre_scale_coverage` (`blend_mode.rs:68–92`); loads destination, optionally expands destination color space, blends, compresses and stores. Rectangle SourceOver may use the already accepted fused ordinal 45 instead of pure ordinal 25 (`blitter.rs:130–151`). `blit_anti_h`, `blit_rect`, and `blit_mask` still determine skips, memset, coverage and run order (`blitter.rs:263–384`). Thus migrating 18–44 only fills the register-level blend operation; it cannot certify assembled paint, destination bytes, AA behavior, full SVG scene preparation, Direct2D execution or Folia integration.

The existing scene Direct2D consumer (`third_party/mermaid-rs-renderer/src/backends/direct2d/paint.rs:208–228` and `source_blend.rs:115–120`) exposes a **different scene-level 16-mode blend path** between images. Its earlier WARP/Source pixel evidence in `docs/mermaid-native/architecture.md` is not an oracle for all 27 original raster-pipeline stages or their highp/lowp register semantics. Conversely acceptance of these pure stages would not establish Direct2D output parity. Root retains interface freeze, actual original Source observer and golds, native failure gate, mutation checks and full consumer acceptance.
