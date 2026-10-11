# M3c root-frozen private Source nonseparable Direct2D effect

M3b's independent oracle exposed 24 pixel differences. No acceptance or Folia
switch is authorized from the pending built-in implementation. This packet
repairs the native consumer boundary without changing shared scene semantics.

## Frozen private interface and ownership

Root declares direct2d/source_blend.rs and imports it from the backend. It has
exactly these crate-private entries:

```
register(factory: &ID2D1Factory) -> Result<(), NativeRenderError>
blend(context: &ID2D1DeviceContext, background: &ID2D1Image,
      foreground: &ID2D1Image, mode: BlendMode, location: &str)
      -> Result<ID2D1Image, NativeRenderError>
```

Root wires register in NativeRenderer::new after querying the factory; paint's
blend dispatch sends only Hue/Saturation/Color/Luminosity to this module.
The remaining modes use the pending M3b paths. No public scene/render API
changes. The native path may not serialize/parse SVG, call usvg/resvg/tiny-skia,
read back pixels, paint through CPU, or reinterpret missing work as Normal.

Sol owns ONLY source_blend.rs implementation, source_blend.hlsl and
docs/mermaid-native/m3c-source-blend-implementation.md. Root owns all other
edits, manifests, license notices, expected pixels, tests, acceptance and
commits. No additional delegation is assigned.

## Native resource contract

Four fixed effect CLSIDs have trailing values 0011,0012,0013,0014 in
e3e05271-5946-4ea9-9432-abcd0000XXXX, ordered Hue/Saturation/Color/Luminosity.
Shader GUIDs use 0021,0022,0023,0024 in the same prefix. Four private factory
callbacks construct one two-input COM effect/draw-transform type with immutable
kernel id. No public mode property or mutable constant-buffer protocol.

Use windows 0.61.3 implement and generated COM traits. Factory registration
checks the actual factory's GetRegisteredEffects under a shared registration
mutex, then registers missing GUIDs; no process-global factory-identity cache.
Never unregister per draw. Partial registration failure propagates and later
calls can retry missing registrations. Shader bytes may be cached process-wide,
but shader loading and effects belong to their effect context/factory. Inputs
are completed background at 0, foreground at 1. Returned images own their
effect graph; no borrowed COM pointer escapes and no reference cycle is needed.

Compile four immutable kernel variants from include_str!(source_blend.hlsl)
using D3DCompile, entry main, ps_4_0, strictness and IEEE strictness, optimization
level zero initially. Preserve compile HRESULT and diagnostic bytes in the
native error. Root adds the Fxc binding feature. This stage uses authored full
shader entry semantics SV_POSITION, TEXCOORD0 and TEXCOORD1 and register pairs
t0/s0,t1/s1; samples use supplied .xy coordinates. No SDK include, SDK path,
filesystem shader source or checked-in binary is permitted at runtime.
Do not copy the SDK helper header. No shader linking optimization is required.
SetPixelShader uses PIXEL_OPTIONS_NONE. Initialize loads the matching bytes and
installs this single transform; SetDrawInfo selects its shader. Fixed graph
SetGraph returns E_NOTIMPL. PrepareForRender need not mutate immutable state.

GetInputCount is 2. Output-to-input mapping requests the same rectangle from
both images. Input-to-output mapping returns a conservative union and empty
opaque subrect; invalid rectangles propagate for either valid input index.
Validate pointer/count protocols before dereferencing, propagate all HRESULTs,
and preserve existing record/EndDraw cleanup. No changes to existing device
identity checks, thread confinement or normal painter batching.

## Exact Source algorithm boundary

Source highp.rs SHA256
81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416.
Translate four f32 premultiplied kernels and sat/lum/set_sat/set_lum/clip_color,
preserving operation order and .30/.59/.11 weights. Hue starts source.rgb*a;
Color starts source.rgb*da. All kernels finish source.rgb*(1-da) +
destination.rgb*(1-a) + adjusted and a+da-a*da.

Important root correction to the read-only audit: mask.blend(t,f) chooses t
when true. clip_color's first selection retains c when captured mx>=0; only
otherwise uses l+(c-l)*l/(l-mn). Its second selection uses
l+(c-l)*(a-l)/(mx-l) when captured mx>a, else retains c, then floors at zero.
Capture mn/mx/l before all channel mutations. Preserve Source behavior rather
than replacing it by a standard HSL conversion. Branch on zero saturation to
avoid making an unselected zero-denominator result observable. Root supplies
BSD notice for the translated tiny-skia/Skia algorithms.

## Acceptance

The unchanged 192-case Source fixture covers all sixteen modes at four DPI
values, two sampled pixels each (1,536 pixel comparisons), with maximum channel
error 2 bytes. It must pass alongside all original ten WARP tests and native
Rust matrix. Root adds threshold/transparent/out-of-bounds cases where needed,
and compiling mutants for mode/kernel, backdrop input, clipping and opacity.
Registration reuse and renderer/device lifetime must be exercised. Compile
failure, WARP skip/crash, expected-value edits and tolerance changes are not
acceptance. This packet alone does not accept group Auto/Isolate, full font
resources/text, diagram compilation, the SVG backend or Folia integration.
