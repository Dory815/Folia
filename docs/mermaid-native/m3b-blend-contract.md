# M3b root-frozen complete Source blend-mode consumption

This prerequisite extends the completed typed scene's blend vocabulary and its
Direct2D consumer. It does not accept complete text preparation, group isolation,
font SVG resources, full diagram compilation, an SVG backend or Folia wiring.
Root owns declarations, Source oracle, stimuli/expected pixels, manifests,
mutants, builds, review and acceptance. Sol implements only assigned bodies.

The initial built-in nonseparable path failed independent pixel comparison.
M3c-source-blend-contract.md supersedes its four-mode implementation direction;
the shared enum, input order and acceptance requirements here remain unchanged.

## Frozen interface and responsibility

Root extends scene/paint.rs BlendMode to exactly the 16 variants in pinned
usvg 0.47.0 src/tree/mod.rs:207–224: Normal, Multiply, Screen, Overlay, Darken,
Lighten, ColorDodge, ColorBurn, HardLight, SoftLight, Difference, Exclusion,
Hue, Saturation, Color, Luminosity. LayerStyle, transforms, opacity, clips,
resource ownership and scene validation are unchanged. This is one shared
resolved enum for both eventual backends, not backend-specific styles.

After declarations are written, Sol may edit ONLY:

- src/backends/direct2d/paint.rs: replace multiply helper by blend(context,
  background,foreground,mode,location), with exhaustive native D2D1_BLEND_MODE
  mapping for the 15 non-Normal modes. Preserve effect lifetime, SetValue/GetOutput
  error forwarding and input order. D2D has no Normal blend enum: that helper
  branch records background then foreground with the existing source-over
  drawing helpers. NativeRenderer's Normal batching remains unchanged.
- src/backends/direct2d.rs: Normal retains existing ordered batching. Every
  other resolved enum enters the same blend helper with completed background
  as input 0 and foreground as input 1. Preserve flush/order and state/resource
  handling. No text/layout/style parsing or SVG serialization/decoding here.
- docs/mermaid-native/m3b-blend-implementation.md: exact implementation notes.

No other files, schemas, tests, expected pixels, manifests, dependencies,
acceptance, application integration, commits or delegated work are assigned.

COLR's Source reductions remain those in colr-source-parity-policy.md. This
consumer does not reinterpret unsupported Source composite operators as newly
supported blend modes. Group auto/isolate behavior remains a separate required
boundary; the current offscreen grouping must not be described as full parity.

## Root acceptance rules

Use the unchanged Source usvg/resvg path only in an independent oracle. Compare
all 16 modes against actual WARP pixels on controlled interior pixels, covering
multiple foreground/background hues, alpha/opacity and layer order. Both
backends must consume the same typed mode; D2D must never invoke the oracle or
manufacture/parse SVG. Preserve the original ten WARP tests and native Rust
matrix. Add compiling mutants for mode substitution, swapped inputs and
order/opacity omissions, with unchanged Source expectations; compile failure,
crash or skipped WARP is failure. Record exact Source/dependency provenance and
precision bounds. If a D2D effect differs from Source, diagnose/fix the shared
or backend boundary; do not loosen expected values to accept a different result.

Only root's review and completed evidence may mark this packet locally accepted.
