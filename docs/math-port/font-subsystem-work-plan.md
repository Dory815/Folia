# Font subsystem ownership and acceptance

This packet advances the existing `architecture.md` common-layout and scene
boundaries. It does not change the Folia package/font profile in `scope.json`.
Root owns public integration, shared contracts, CMake, builds and acceptance.
The assigned Luna owns the font algorithms, their data generation and complete
source/native witnesses; implementation details within these boundaries are
the owner's responsibility.

Source is MathJax-src 4.1.3, commit
`fb987178c2d279a99b0db5ea02e751691703955e`:

| Source | SHA-256 | Native module |
| --- | --- | --- |
| `ts/output/common/Direction.ts` | `cfa6ccfacb73e6991d373a3677a097356de2092405c742f29811337fd9e511c2` | `folia.math.output.common.direction` |
| `ts/output/common/FontData.ts` | `7d8eeca536fa1b7c14517590b9c5a372ad10b34667d196d09b030f88cda780b2` | `folia.math.output.common.font_data` |
| `ts/output/svg/FontData.ts` | `403f59e6cb71fa8baf291402e085daaa25d1d11e8fc48640e370498da6a83da9` | `folia.math.output.svg.font_data` |

Owned production files are the matching snake_case `.ixx`/`.cpp` files under
`src/math/output/common` and `src/math/output/svg`, plus source-derived font
tables under `src/math/fonts`. Owned tools, tests and evidence use the
`font-` prefix. Existing parser files and shared utility APIs remain with their
current owners. No source file may be represented by a single generic renderer
switch. New dependencies must be identified by exact required behavior.

The API retains FontData responsibilities: variant inheritance and linked
records, SMP remapping, character options and presence, delimiter data,
parameters, size/stretch variants, extensions, dynamic file state and retry
semantics. Static/class defaults and per-font instance state are distinct;
preserve the Source mutation, insertion and alias rules. Reuse the existing
native Options, Property, UTF-16, number, string and Promise services where
their verified contracts apply. Native class fields must remain their actual
value authority; Context copies are prohibited.

SVG glyph outlines and content overrides remain source-derived font data.
The common layout may read metrics/options without using an SVG backend.
Future scene construction reads these same resolved glyphs and outlines;
SVG and Direct2D do not get separate font measurements or glyph selection.
Parsing glyph path commands into geometry is distinct from parsing SVG XML.
Font loading uses native bundled data/provider and the existing native queue;
JS/CJS may run only in build-time generators or original-result oracles.

Retain every member in these Source files with a source/native location and
hash ledger. Pin each generated NewCM/bbm/bboldx/dsfont input to the existing
4.1.3 lock. Validate data counts and contents, exact metrics, optional fields,
variant/prototype/link aliases, SMP Greek/Latin mappings, extension conflicts,
delimiter assembly metadata and repeated dynamic load/retry state. Unknown
glyph behavior and malformed inputs must match the actual original profile;
estimated metrics, replacement fonts and omitted extension shards do not pass.

The owner supplies a deterministic pinned CJS oracle, actual C++ fixtures and
a strict differential comparator. Root compiles/runs the actual native module,
reviews behavior and meaningful semantic faults, and records acceptance in
small reviewable commits. A declaration, generated table or green primitive
test alone does not accept full fonts, common layout, SVG or Folia integration.
