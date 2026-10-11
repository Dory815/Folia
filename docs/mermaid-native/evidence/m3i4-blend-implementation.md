# M3i4 pure blend implementation handoff

Status: Sol implementation awaiting Root's independent Source raw-bit corpus, semantic mutation and native matrix acceptance. This packet changes only `sampling/blending/highp.rs`, `sampling/blending/lowp.rs` and this report; it does not accept complete paint, SVG/Direct2D pixels, or Folia integration. Per the assignment, no build/test/commit was run.

The original pinned files are tiny-skia 0.12.0 `src/pipeline/highp.rs` SHA-256 `81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416` and `src/pipeline/lowp.rs` SHA-256 `3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88`. Both files retain the Source copyright and reference the existing `LICENSE-tiny-skia-blend` notice.

## Source-to-native ledger

| Original Source | Delivered body |
|---|---|
| highp `blend_fn` 405–416, `clear`, `source_atop`, `destination_atop`, `source_in`, `destination_in`, `source_out`, `destination_out`, `source_over`, `destination_over`, `modulate`, `multiply`, `screen`, `xor`, `plus` 418–433 | Same macro/function bodies in `blending/highp.rs`. The match dispatches original ordinal names to their functions; RGB are written R→G→B, alpha last. |
| highp `blend_fn2` 435–447; `darken`, `lighten`, `difference`, `exclusion`, `color_burn`, `color_dodge`, `hard_light`, `overlay`, `soft_light` 449–504 | Same functions and closure expressions, including eager SIMD candidates, `recip_fast`, `sqrt`, and separate source-over alpha. |
| highp `blend_fn3` 514–526; `hue_k`, `saturation_k`, `color_k`, `luminosity_k` 528–616; `sat`, `lum`, `set_sat`, `set_lum`, `clip_color` 618–667 | Same function and helper bodies. Original `clip_color` comparison/selection remains unchanged, including its counterintuitive true branch. |
| highp `inv`, `two`, `mad` 1347–1359 | Same helper bodies using the pinned `sampling/wide::f32x8`, without host scalar math or FMA substitution. |
| lowp `blend_fn` 464–475; 14 ordinary modes 477–492 | Same macro/function bodies in `blending/lowp.rs`, including exact `div255` placement and alpha-last write order. |
| lowp `blend_fn2` 495–507; `darken`, `lighten`, `exclusion`, `difference`, `hard_light`, `overlay` 509–532 | Same functions and u16 vector operation order; both candidates of `HardLight`/`Overlay` are still evaluated before blend. No widening, manual wrapping, saturation, or lazy branch was inserted. |
| lowp `div255`, `inv` 848–859 | Same helper bodies through original `sampling/wide::u16x16` provider. |
| highp/lowp `Pipeline` continuation | A private eight-register `Pipeline` adapter holds `r,g,b,a,dr,dg,db,da`; its `next_stage()` is the mechanical single-stage boundary and performs no action. The caller still owns precision and continuation. After the original function returns, all eight channels are copied back in original order. |
| original pointer tables `highp.rs:58–139`, `lowp.rs:71–154` | High dispatch supports exactly ordinals 18–44. Low dispatch supports 20 original non-null entries; `ColorBurn`, `ColorDodge`, `SoftLight`, `Hue`, `Saturation`, `Color`, and `Luminosity` return `SamplingError` rather than invoking Source `null_fn`. Every other ordinal also errors before register conversion or mutation. |

The Source audit in `m3i4-blend-source-audit.md` records the lowp premultiplied-range and debug-overflow limitation. Root's frozen lowp gold restricts `HardLight`/`Overlay` to safe original intermediate values. This implementation deliberately does not alter Source arithmetic to expand that domain. Remaining work includes Root's 4,686-case actual Source comparison and mutations, pattern stages, full blitter assembly/skips/coverage, completed scene and both backend renderers.
