# M2e Architecture icons — awaiting root acceptance

The implementation is confined to
`third_party/mermaid-rs-renderer/src/scene/compile/architecture_icons.rs`.
Its authority is pristine `mermaid-rs-renderer` 0.3.1
`render.rs::architecture_icon_svg`, lines 2633–2813, SHA256
`026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4`.
It returns ordered typed shapes or a lambda `TextPresentation` request; it does
not construct or parse SVG/XML/path strings.

| Source operation or branch | Source lines | Target lines and handling |
| --- | ---: | --- |
| `cx=w/2`, `cy=h/2`, `r=w.min(h)*.35`, `sw=(w*.02).max(1.5)`; one-decimal stroke width; ASCII lowercase of optional name | 2634–2642 | 32–46, 278–296 |
| Exact `internet`/`globe`: circle, ellipse, horizontal and vertical lines | 2644–2657 | 49–54, 217–245 |
| Substring `internet`/`gateway`/`api-gateway`: circle, ellipse, horizontal line only | 2658–2670 | 55–62, 217–245 |
| Exact `server`: three rectangles alternating with three indicator circles; `bx/by/bw/bh`, `rows`, `row_h`, `ry`, `dot_x/dot_y/dot_r` retain source `f32` order | 2671–2694 | 63, 114–134 |
| Substring `server`/`ec2`/`compute`: identical six ordered items | 2695–2716 | 64–66, 114–134 |
| Exact `database`/`disk`: `bx/bw/ell_ry/body_top/body_bot/body_h`, rectangle, two ellipses, two side lines | 2717–2734 | 67, 136–154 |
| Substring `database`/`aurora`/`rds`/`dynamodb`/`db`/`disk`/`storage`/`s3`/`glacier`: same source measurements and first three items, without side lines | 2735–2756 | 68–79, 136–154 |
| Exact `cloud`: `s=r*.7`; Move, four quadratic curves, Close | 2757–2772 | 80, 156–184 |
| Substring `cloud`: identical closed quadratic path | 2773–2787 | 81–83, 156–184 |
| Substring `lambda`: `fs=w*.72`; one-decimal center, zero-decimal size, middle/central/serif, raw unescaped fill lexeme and direct λ | 2788–2793 | 84–100, 268–276 |
| Fallback: `s=r*.9`, `x/y`, `inner=s*.35`; rounded square, horizontal line, vertical line | 2794–2812 | 101–109, 203–245 |

The match retains source branch order, so exact names precede their substring
matches and the cloud check precedes lambda. `server_icon`, `database_icon`,
and `cloud_icon` only consolidate duplicate source branches; their arithmetic
and item order remain the source's. `decimal` formats and parses a scalar at
each `.1` or lambda `.0` emission site. Cloud operands are individually
quantized into a `NonZero` typed path, with no interpretation of the source
comment that calls the curves cubic.

Outlined shapes use no fill, the supplied resolved paint for their stroke,
one-decimal width, round cap and join, miter limit 4, no dash, unit opacities
and empty metadata. The server dots use the supplied fill with no stroke.
Server and fallback rectangle corner radii are the source's literal 2 and 4;
the database rectangle has zero radii. Nonfinite emitted values and negative
emitted sizes/radii/font size produce structured `SceneError`, with no input
clamping or arbitrary upper bound. The lambda fill remains
`TextValue::AttributeLexeme(fill_lexeme)` for the common lexical resolver;
constant anchor/family use `Literal`.

Limitations: no Architecture caller is wired to this precursor, and lambda
has no resolved text appearances until the shared text stage is complete.
Malformed or unsupported fill lexemes must be diagnosed by that stage. No
blocking implementation question was found. Root owns source observations,
oracles, mutation and WARP tests, exports, build review and acceptance. I did
not run builds or tests, modify other source or manifests, or commit. This
submission does not establish complete scene, SVG, Direct2D or Folia parity.
