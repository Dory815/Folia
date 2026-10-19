# Six TeX package port status

This packet covers the pinned MathJax 4.1.3 `action`, `bbox`, `braket`,
`cancel`, `enclose`, and `color` package paths. The C++ implementation currently
lives under the corresponding `src/math/input/tex/<package>/` folders. The
configuration translation units register the Source handler/options/config
records. Package files follow their Source ownership: Braket has separate
`items`, `methods`, and `mappings` modules; Color has separate `constants`,
`util` (the `ColorModel` class and processor/Map ownership), `methods`, and
`configuration` modules. Action, BBox, Cancel, and Enclose keep their Source
methods with their configuration owner because those upstream files do. These
are candidate sources only until Root registers and builds them.

`tools/math-port/tex-six-packages-original-oracle.cjs` validates the full 246
file Source lock before loading CJS, then records the 24 package observations
and trees in ignored `build/math-port/tex-six-packages-original.json`.
`tools/math-port/tex-six-packages-native-candidate.cpp` runs the same inputs
through the real native TeX graph and emits MML, exact native errors,
`packageData`, and the selected Source-ordered TokenMap entries. It has not
been freshly compiled or compared against the current Braket fixes, so it
remains diagnostic input rather than acceptance. Row `braket-nested-set`
reproduces the G77 parser crash input and verifies the Source push-then-
environment assignment order through a comparable successful result.

The supplementary `tex-color-model-boundary-original-oracle.cjs` records five
additional pinned cases: ECMAScript whitespace in RGB/gray parsing, overflow
error identity, a user-defined named color, and the `ColorModel` own Map and
prototype method metadata. Native ColorModel now constructs an actual Context
instance with a real `userColors` Map; the Source `ColorModelProcessors` and
`COLORS` lookups also use Context Map identities and live method dispatch.
Cancel and Enclose share the same canonical `ENCLOSE_OPTIONS` PropertyStorage.

Remaining acceptance work:

- Compile all package interfaces and implementations in one registered graph.
- Run the native candidate and strictly compare all 24 Source rows, including
  packageData and exact token-map entries; extend the native observer rather
  than omitting those fields.
- Compare the supplementary ColorModel observations against the actual native
  Context constructor/prototype/Map identities and values.
- BraketItem's internal `barNodes` remains a native vector rather than a live
  Context-visible Source array property. Constructor reflection and mutation
  through that field remain unaccepted.
- Context methods currently validate typed ColorModel method arguments and
  return string colors. Arbitrary monkey-patched methods or non-string values
  in `userColors` remain outside the observed TeX corpus and require exact
  Context call/return semantics before claiming unrestricted Source parity.
