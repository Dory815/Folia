# M3c Source blend implementation report

Sol implemented `source_blend.rs` and `source_blend.hlsl` within the frozen
packet. Root independently reviewed, repaired, built and verified them. Local
acceptance is in evidence/m3bc-source-blend-acceptance.md; full backend and
Folia acceptance remain open. Root owns callers, features, licensing and tests.

## Source operation mapping

Pinned tiny-skia 0.12 `src/pipeline/highp.rs` SHA256
`81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416`:

| Source | Native shader |
| --- | --- |
| `hue_k` 531–550 | `main` kernel 0: `s.rgb*s.a`, destination saturation times `s.a`, destination luminosity times `s.a`, clip at `s.a*d.a`, then Source premultiplied cross terms and alpha. |
| `saturation_k` 554–573 | `main` kernel 1: `d.rgb*s.a`, source saturation times `d.a`, destination luminosity times `s.a`, same clip and finish. |
| `color_k` 577–595 | `main` kernel 2: `s.rgb*d.a`, destination luminosity times `s.a`, same clip and finish. |
| `luminosity_k` 599–616 | `main` kernel 3: `d.rgb*s.a`, source luminosity times `d.a`, same clip and finish. |
| `sat` 619–621 | `sat3`: `max(r,max(g,b))-min(r,min(g,b))`. |
| `lum` 624–626 | `lum3`: left-associative `r*.30+g*.59+b*.11`. |
| `set_sat` 629–641 | `set_sat`: snapshot min/max/span, zero branch, then per-channel `(c-min)*s/span`. |
| `set_lum` 644–649 | `set_lum`: compute one luminosity difference, add to each component. |
| `clip_color` 652–668 | `clip_color`/ `clip_one`: snapshot min/max/luminosity; preserve the Source quirk that captured `mx>=0` retains `c`, while `mx<0` computes the first correction; captured `mx>a` computes the second correction using the possibly modified channel; floor each at zero. |

The HLSL uses t0/s0 for the completed background and t1/s1 for the
foreground, supplied `TEXCOORD0.xy` and `TEXCOORD1.xy`, with `SV_POSITION`,
`SCENE_POSITION` and `SV_TARGET`. Four fixed preprocessor variants are compiled from the
embedded HLSL source with `D3DCompile(main, ps_4_0)` and strictness,
IEEE strictness and optimization level zero. The Source-derived shader points
to the root-provided `LICENSE-tiny-skia-blend` BSD notice.

## Native protocol mapping

`source_blend.rs` owns fixed CLSIDs 0011–0014 and shader GUIDs 0021–0024
under the frozen prefix, in Hue/Saturation/Color/Luminosity order. Four factory
callbacks create one immutable-kernel COM object. Registration is serialized
through one mutex, queries `ID2D1Factory1::GetRegisteredEffects` on the
actual supplied factory, registers only missing IDs and leaves successful
earlier registrations for a retry after partial failure. Compilation bytes are
cached process-wide; shader loading happens in each effect context.

`Initialize` validates its interface references, loads the matching pixel
shader and installs its own draw transform; `SetDrawInfo` selects the same
GUID with `PIXEL_OPTIONS_NONE`. `PrepareForRender` preserves immutable state
and `SetGraph` returns `E_NOTIMPL`. The draw transform advertises two inputs,
maps output bounds identically to each input, maps inputs to their conservative
union with an empty opaque subrect, and forwards invalid rectangles for indices
zero and one. Count/pointer validation precedes raw-pointer dereferences.
`blend` sets background at input 0 and foreground at input 1; `GetOutput`
returns an owned image with the effect graph retained by Direct2D. Compilation
failures retain HRESULT and all diagnostic bytes as hex in `NativeRenderError`.

## Open verification points for root

Root's independent compile/runtime review corrected the COM OutRef result to
Option<IUnknown>, added the existing pinned windows-core 0.61.2 dependency for
the implement macro, completed XML metadata and its required declaration, and
added the full-shader SCENE_POSITION input slot. Without the latter, the initial
full shader sampled the wrong coordinates even though it compiled and drew.
The unchanged 192-case Source/WARP test then passed all 1,536 sampled pixels at
the original two-byte tolerance. The native Rust matrix also passed 37 tests,
including all original ten WARP tests. Mutation acceptance remains root-owned
and pending; this report alone does not accept the packet.

Root subsequently completed the final strengthened WARP test and all 39
compiling mutation checks. Local acceptance is recorded separately in
evidence/m3bc-source-blend-acceptance.md; the full backend remains open.

- Compile and WARP verification remain pending, including whether the Windows
  0.61.3 generated `#[implement]`/D2D registration ABI accepts the minimal
  effect XML and callback signatures as authored.
- Device-side HLSL float scheduling, NaN `max` semantics and premultiplied
  texture sampling need comparison against the pinned Source pixel oracle;
  these have not been asserted equivalent by this packet.
- The effect makes a conservative bounding union even when an input rectangle
  is empty. This is safe for coverage but can enlarge work; root should inspect
  actual WARP bounds and clipping behavior.
