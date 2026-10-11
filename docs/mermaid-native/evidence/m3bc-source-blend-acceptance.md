# M3b/M3c local Source blend consumer acceptance

Root locally accepts the typed 16-mode blend consumer, including a private
Direct2D GPU implementation of Source Hue/Saturation/Color/Luminosity. This
does not accept complete Mermaid compilation, text/font preparation, group
Auto/Isolate, embedded font SVG resources, the shared SVG backend or Folia
integration. Render-function acceptance remains 34/70.

## Independent Source evidence

Root's development-only blend-reference crate uses the unchanged usvg/resvg
0.47.0 path as an oracle. It has no native-backend imports. Pinned usvg's
16-mode enum hash is cb11738534ce989b8583827d55e3e01096831ad40075f7c8351365da7ece35b5;
tiny-skia 0.12 highp.rs hash is
81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416.
The generator resolves the locked dependencies and checks both hashes.

The 192 stimuli cover all sixteen modes and twelve foreground/background/
opacity combinations: distinct hues, opaque/translucent colors, gray, black,
white, zero alpha and zero opacity. Source expectations are independent of
native output. Two interior pixels (overlap and background outside foreground)
at 96/120/144/192 DPI give 1,536 compared BGRA pixels. The canonical fixture
SHA256 is 593da8e3ab104ae9e47d0107c3a1960c67cc1d221966567f6cacaca0c03164f7.
`node tools/mermaid-native/blend-oracle.cjs --check` reproduces it.

The original built-in mode-name mapping failed 24 pixel comparisons: Hue,
Color and Luminosity on six distinct configurations at four DPI values.
Root diagnosed Source's nonseparable f32 kernels and unusual clip_color
selection; its first branch retains c when captured mx>=0. The native shader
preserves those algorithms and source-over cross terms. Expectations and the
two-byte channel bound were retained. The final observed maximum error is
**2 bytes**, not a claim of bit-identical floating point or pixels.

## Final native boundary and root review

Normal retains ordered source-over batching; eleven separable modes use native
Blend effects. Four nonseparable modes use immutable private COM effects and
runtime-compiled embedded HLSL, each with two inputs. Background is input 0,
foreground input 1. No SVG serialization/parse, usvg/resvg invocation, CPU
readback or raster fallback exists in this consumer. SDK headers/files are not
required for runtime shader compilation. The translated source has BSD notices.

Root froze responsibilities before implementation, reviewed the four kernels,
mode selectors, COM references, pointer/count checks, rectangle union,
registration and HRESULT/cleanup forwarding. Independent execution corrected
the macro's direct windows-core dependency, COM OutRef Option return, effect
XML declaration/metadata and full shader SCENE_POSITION input slot. Registration
queries the actual factory under a mutex; shader bytes are process-cached,
effects and loading remain factory/context resources. The final WARP test
creates 192 renderers on the same device/factory, then releases each renderer
and input scene before drawing the retained graph at all four DPI values.

## Verification

Root commands use locked offline dependencies, no default features, direct2d,
and FOLIA_SCENE_TEST_FONT=C:/Windows/Fonts/arial.ttf. The ten explicit targets
scene_contracts, path_parity, edge_symbols_parity, node_shapes_parity,
text_presentation_parity, edge_label_geometry_parity, architecture_icons_parity,
c4_style_parity, label_data_parity and direct2d_warp passed **37 tests**.
The final direct2d_warp run passed all **11 tests**, preserving the original
ten and the strengthened Source pixel/registration/lifetime test. Logs:
build/m3bc-native-matrix.log and build/m3bc-final-warp.log.

Root's isolated mutation runner was rerun on the final strengthened test. All
**39 mutants compiled and failed assertions**, baseline 11 passed, no WARP
skip/crash accepted, and guarded working sources/fixtures remained byte-exact.
The original 18 are preserved; added mutations cover every new separable mode,
four Source kernel selectors, Source clipping quirk, luminosity weights, alpha
cross term, both input orders, output intersection and painter order. Results
are committed in m3bc-source-blend-mutations.json; logs remain in ignored
build/mermaid-direct2d-mutations and build/m3bc-final-mutations.log.

These are explicitly scoped native targets. The upstream crate's unvendored
example-dependent lib tests and full WinUI app are not claimed tested here.
No C++ source or production Folia route changed. Device loss/hardware coverage
and complete diagram/style/font/resource behavior remain required at the full
backend/application gate; this primitive packet does not waive them.
