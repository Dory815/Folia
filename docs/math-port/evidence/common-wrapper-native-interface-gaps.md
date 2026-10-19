# Common wrapper / font interface gap audit

Status: read-only source-to-native audit during gate 18. This records current
interfaces and implementation gaps only; it does not authorize wrapper code or
claim parity. The existing scene-boundary ledger in
`e3-common-scene-boundary.md` remains the method inventory.

Reference is MathJax-src 4.1.3, commit
`fb987178c2d279a99b0db5ea02e751691703955e`.

| Source | SHA256 |
| --- | --- |
| `ts/output/common/Wrapper.ts` | `d1db6112db1cb3546658eb8868b5997a4f69e588632ecbe4e08a71949544a828` |
| `ts/output/common.ts` | `3686a8e803e26ae59d03c096ac2e6e5b1ab1332a5c4bcaa083cb9a02c2c61096` |
| `ts/output/common/FontData.ts` | `7d8eeca536fa1b7c14517590b9c5a372ad10b34667d196d09b030f88cda780b2` |
| `ts/output/svg/FontData.ts` | `403f59e6cb71fa8baf291402e085daaa25d1d11e8fc48640e370498da6a83da9` |
| `ts/output/common/Wrappers/mtext.ts` | `855b7d60c3736cbb7d3368ae3399c71a704754cecdbc78dd490508a2dd3cc4e9` |
| `ts/core/OutputJax.ts` | `e4932ba7a5d8f782022fa0ab3c8a8bc05a3072704cd8802213fdadb2722789a3` |
| `ts/core/DOMAdaptor.ts` | `fc4bc991ab2e9a35fd2693a38ff6915286e549d884591922a5d7a716630e8af6` |
| `ts/core/MathItem.ts` | `c1c7308320ae15514fd1114c3f94642a4e8a8268bfe6f68192e8a557829626ce` |

## Source call paths and required values

`CommonWrapper` captures `factory.jax.font` in its constructor and immediately
uses font state while initializing wrapper style, scale and variant
(`Wrapper.ts:538–555`). `getVariant` checks the selected name through
`font.getVariant` (`925–968`). `getChar` reads one resolved glyph tuple and
falls back to `[0,0,0,{unknown:true}]` when absent (`1469–1470`); callers use
the first three tuple slots as height, depth and width and read the options
slot for `ic`, `oc`, `sk`, `dx`, `smp`, `hd`, `unknown` and SVG's `p`/`c`.
Other direct common-wrapper consumers include `font.params.rule_thickness`
(`1371`), `getVariant(name)` (`1392`), delimiter/variant selection in
`common/Wrappers/mo.ts:361–538,662–675`, and `getCssFont` in
`common/Wrappers/mtext.ts:234–262`.

`CommonOutputJax` selects either an existing `fontData` instance and its
constructor or the configured/default font class (`common.ts:292–317`), calls
`setOptions`, and loads configured extensions through `font.addExtension`
(`350–352`). Its measurement/style paths read `font.params.x_height`
(`437,836`), `font.styles` (`881`), and `font.getFamily` (`1047,1062`). Thus
the common layer requires one shared font instance with a live variant graph,
glyph data and option records, delimiter data, CSS font tuples, parameter
values, and styles. SVG path/cache metadata must remain on that same resolved
record; common wrappers must not depend on the SVG font class.

`AbstractOutputJax` owns options, pre/post filters and a `DOMAdaptor`; the
interface includes adaptor installation, initialize/reset, typeset/escaped,
metrics, stylesheet and page-elements operations (`OutputJax.ts:28–111,
129–193`). The common implementation also calls adaptor node, text,
attribute/style, append/remove, clone, parent/child, measurement and font
queries; representative calls are `common.ts:404,443,665–739,797–836,
922–994`. The adaptor is an actual host boundary, not equivalent to serializing
an SVG string. `MathItem` provides the source math, metrics and lifecycle
relationship (`MathItem.ts:346 onward`).

## Existing native surfaces and concrete mismatches

The font implementation is present at `src/math/output/common/font_data.ixx`
and `font_data.cpp`, with SVG-specific data at
`src/math/output/svg/font_data.ixx/.cpp`. Native `CharDataArray` is now an
alias for the identity-bearing sparse `PropertyArray`, and the canonical tuple
can preserve holes, explicit Undefined/Null, extra slots and the shared
options record. Native `FontData::getChar` exposes `optional<Property>`
(`font_data.ixx:243`); the contained Property can hold the same PropertyArray
storage, while an empty optional represents no result. That is a workable
tagged host representation of Source's tuple-or-null result, not evidence for
a second glyph struct. A wrapper consumer must read the contained tuple and
option slot without flattening or copying it. `FontDataClass::charOptions`
returns a `Property` (`font_data.ixx:184`) and preserves the slot's raw value;
the caller must perform property reads only at the corresponding Source
operation.

Native `FontParameters` contains the known numeric fields and an `extra` map
(`font_data.ixx:73–80`); wrapper algorithms directly consume the full set,
including `delimiterfactor`, `delimitershortfall`, `rule_factor`,
`min_rule_thickness`, `separation_factor`, `extra_ic` and `extender_factor`.
The field names exist, but plain `double` storage cannot represent Source
Missing/Null or unusual raw values if those are observable through the enabled
profile. Source wrapper integration must establish whether each property is
always normalized before use rather than silently assuming every field is a
number.

`FontVariantData` retains a shared character map, linked maps and optional
cache ID (`font_data.ixx:91–96`). Native delimiter values retain the principal
optional arrays/metrics (`100–113`), CSS font tuples are PropertyArrays
(`80–81`), and the common API exposes variant/delimiter/CSS lookup
(`239–252`). These are useful data primitives, not a common wrapper or a
wrapper-facing resolved-glyph contract. `getCssFont` returns a PropertyArray
handle by value (`font_data.cpp:496`); PropertyArray copies share the same
PropertyStorage, so this does not itself break object identity. Its fallback
tuple is a fresh array, consistent with a fresh Source fallback value.

Native `BBox`, `Styles` and `StyleJsonSheet` exist (`util/bbox.ixx`,
`util/styles.ixx`, `util/style_json.ixx`). Generic tree wrapper interfaces also
exist (`core/tree/wrapper.ixx`, `wrapper_factory.ixx`). At this audit point,
there is no native `core/OutputJax`, `core/DOMAdaptor`,
`output/common/Wrapper`, `CommonOutputJax` or common wrapper factory module in
`src/math`. The existing `mml::HtmlAdaptor` in
`core/mml_tree/html_adaptor.ixx` serves MML `HtmlNode` and only exposes XML/HTML
operations; it does not supply the output adaptor's measurement, style,
document, or general node lifecycle surface. Existing `core/math_item.ixx`
has output-jax service callbacks returning `NativeObjectRef`, but does not
define the OutputJax/DOM adaptor contract.

## Implementation boundary indicated by the evidence

The missing interface is the real common wrapper/output-jax boundary wired to
the existing MML tree, BBox/Styles, and the same `FontData` instance. A
wrapper-facing glyph read must preserve the canonical PropertyArray and its
option-slot presence; common measurement consumes metrics/variant/delimiter
data, while SVG-only path/cache keys remain available to the later SVG scene
consumer. The core output host must expose the actual DOM-like operations
reached by the enabled SVG route or a narrowly documented native SVG host
adaptation. Neither a fabricated font tuple, a second measurement model, nor
the MML HtmlAdaptor alone closes these dependencies.

This audit does not settle the final C++ signatures or decide which DOMAdaptor
operations are unreachable under the locked Folia profile. Those require the
Root-owned common wrapper/scene contract and a call-closure review against the
enabled entry profile.
