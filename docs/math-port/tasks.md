# Root-owned implementation packets

No packet is complete because an agent says it is. Root checks source fidelity,
independently runs the frozen tests, reviews the diff and records acceptance.
Agents must not modify architecture, scope, root contracts, the oracle, frozen
fixtures, build files, production integration or another packet's files.
The rejected prototype is archived. Revision-1 primitive contracts govern
P1, P2 and P3a; `numeric-contracts.md` separately freezes P4/P5. These are not
acceptance of the dependency closure or permission for broader dispatch.

## Packet template required before dispatch

Every implementation task must contain:

- exact upstream file hashes and methods to port;
- exact owned target files, interfaces and dependencies supplied by root;
- required preserved behavior, including branches and errors;
- explicit host adaptations with reasons, if any;
- frozen expected fixtures and unit/differential test commands;
- architecture constraints, permitted edits and prohibited substitutions;
- an acceptance checklist and known blocking prerequisites;
- completion evidence: method ledger, differences, test logs and remaining gaps.

If a prerequisite or interface is missing, the agent reports that fact. It must
not invent a replacement tree, registry, fallback, formula or model to keep
coding. Root resolves the dependency before work resumes.

Root-owned member discovery uses `tools/math-port/member-ledger.cjs`, with the
same TypeScript 5.9.3 parser declared in the pinned MathJax development manifest.
Install it in ignored output with `npm install --prefix build/math-port-tooling
--no-save --package-lock=false --ignore-scripts typescript@5.9.3`. Pass upstream
`ts/...` paths; `--summary` prints counts, otherwise it emits member hashes and
pending target/review fields. It inventories class members, module functions
and initialized globals. Nested literal callbacks remain inside their enclosing
member hash and need separate manual method correspondence where applicable.
AST discovery never approves behavior, targets, exclusions or acceptance.

## Current dispatch: reference evidence, not renderer implementation

Root has locally accepted ten of 246 inventory entries: Attributes, BBox,
PrioritizedList, lengths, Options and five generic Tree files. The separate
native Number formatting helper is a host adaptation. The other 236 inventory
entries are still unreviewed. Broad parser/layout implementation packets remain undispatched;
agents cannot fill missing prerequisites with a substitute architecture.

The following Luna assignments own **one proposal file each**, never frozen
expectations. All use upstream commit
`fb987178c2d279a99b0db5ea02e751691703955e` and the unchanged production profile.

| Agent | Only writable file under `docs/math-port/evidence/` | Exact package responsibility |
| --- | --- | --- |
| `tex_parser` | `e4-input-corpus-proposals.json` | base, action, ams, amscd, bbox, begingroup, boldsymbol, braket, bussproofs, cancel, cases, centernot, color |
| `math_layout` | `e5-layout-corpus-proposals.json` | colortbl, configmacros, empheq, enclose, extpfeil, fontsizev3, gensymb, html, mathtools, mhchem, newcommand, physics, setoptions |
| `math_fonts` | `e6-font-text-corpus-proposals.json` | bbm, bboldx, dsfont, noundefined, tagformat, texhtml, textcomp, textmacros, unicode, units, upgreek, verb |

Each proposal supplies bounded TeX input, display/em/width options, complete
render outcome classification, concrete source methods/lines/hashes and the
reason that particular branch is exercised. Parser validity and render success
are separate facts. An unconfigured API option or disabled HTML guard is a
policy witness, not positive feature coverage. A rendered warning caused by
missing font data is a dependency defect, not a disabled syntax feature.
State-changing cases must reset state or explicitly describe a session test.
Every proposal records known uncovered behavior.

Permitted inspection is read-only source analysis and small fresh-process
queries through the unchanged root oracle. Agents may not edit native code,
contracts, scope, corpus, golden results, comparator, build or app integration;
may not build in shared output directories or commit; and may not silently
remove packages, suppress errors, add fallbacks or change the production profile.

**Root acceptance:** independently verify cited hashes, execute the complete
ordered corpus, reject incorrect result classifications, inspect intended
semantic witnesses, and freeze requests/results independently. Root owns
`reference-corpus.json`, `evidence/reference-golden.json` and all five
`tools/math-port/reference-*.cjs` files. The tools must prove observer neutrality
against a fresh uninstrumented profile and detect deliberately corrupted
semantic/layout/SVG representations. Detailed acceptance and gaps are in
`evidence/reference-oracle-acceptance.md`. These assignments authorize evidence
collection only; they do not pass a native implementation gate.

## Wave 0 — reference and contracts (root)

**Owned files:** this directory, `tools/math-port/*`, the future public API and
all shared semantic/layout/scene contract interfaces.

**Inputs:** pinned source, existing Folia package/options profile, complete
font package inventory and the three read-only evidence audits.

**Outputs:** reviewed `scope.json`, per-method port ledger, interfaces with
ownership/value/lifetime/position semantics, full reference snapshots and a
mutation-tested comparator. Inspect actual MathJax objects for MML attrs,
classes, BBoxes and wrapper placements rather than recovering everything from
rounded SVG. The comparator must also inspect complete serialized fragments.

**Acceptance:** Full G0 and oracle mutation tests in architecture.md block Wave 2
and later. A primitive packet can be implemented and locally checked after root
reviews its own dependency slice and freezes interfaces and fixtures. No source item
may be unreviewed. No undeclared profile reduction. All snapshots record the
version/config/source hashes. Source discovery is not scope acceptance.

## Wave 1 — small independently verifiable primitives

### P1: Attributes (Luna implementation, root acceptance)

**Owned files:** `src/math/core/mml_tree/attributes.ixx`, its implementation
unit if needed, and non-frozen local tests for this class only.

**Source:** `ts/core/MmlTree/Attributes.ts`, all methods. **Dependencies:**
root-defined Property/Missing types and property-map storage contract.
**Output:** upstream Attributes API and lookup semantics. Explicit values,
inherited values, defaults, globals, `INHERIT`, clone/getList and presence
queries must be distinguishable. No flattened resolved-attributes substitute.

**Acceptance:** per-method upstream snapshots covering missing/false/0/empty,
all four lookup levels, mutation/deletion/copy, default/global interactions,
`getExplicit`/`getInherited`/`getDefault` distinctions and malformed queries.
Every retained method listed and mapped. G1 local class gate.

### P2: BBox (Luna implementation, root acceptance)

**Owned files:** `src/math/util/bbox.ixx`, its implementation unit if needed,
and non-frozen local tests for this class only.

**Source:** `ts/util/BBox.ts`, all fields and methods. **Dependencies:**
root-approved sentinel/constants and numeric behavior.
**Output:** BBox math with scale/rscale, L/R, ic/oc/sk/dx, pwidth, side handling,
empty/clean/rescale/combine/append/update behavior preserved. No reduced box.

**Acceptance:** upstream method snapshots and seeded operation sequences with
negative dimensions, empty sentinels, non-unit relative scale and asymmetric
spacing. Exact discrete fields; frozen full-precision numerical comparison.
No SVG involved in this packet. G4 primitive gate.

### P3: PrioritizedList and Options (Luna implementation, root acceptance)

Split into P3a PrioritizedList and P3b Options. P3a is accepted under
primitive-contracts.md. P3b is independently accepted under the root-frozen
options-contracts.md and evidence/options-acceptance.md, only as a configuration
prerequisite for the specified native API.

**Owned files:** `src/math/util/prioritized_list.ixx`, `options.ixx`, their
implementation units if needed, non-frozen local tests for these utilities.

**Source:** `ts/util/PrioritizedList.ts`, `Options.ts`, only root-listed retained
methods with an explicit method ledger; all package configuration requirements
must be covered. **Dependencies:** root's typed option-value contract.
**Output:** ordering/tie behavior and upstream merge/expandable/value rules.
Do not replace priority semantics with an unordered map or reinterpret merges.

**Acceptance:** tie/priority insertion, removal/reordering and default/user/
expandable option snapshots used by the exact enabled profile. Root verifies
source method coverage before accepting G2's configuration prerequisite.

Wave 1 must pass before broader parser/layout tasks are dispatched. These tasks
are intentionally limited to primitives whose interfaces and complete behavior
can be independently verified; no agent is told simply to “implement TeX”.

### P4/P5: exact numeric formatting and lengths

Root froze the exports, native host adaptations and source obligations in
`numeric-contracts.md` and generated `tests/math/reference_numeric.cpp` before
dispatch. `math_layout` owns only `src/math/util/js_number.ixx` and P4 evidence;
`math_fonts` owns only `src/math/util/lengths.ixx` and P5 evidence. The helper
adapts Number formatting, while lengths preserves the four source functions,
all tables, prefix regex behavior and Unicode whitespace. It imports the
accepted helper rather than implementing a second formatter. Agents cannot
modify fixtures, oracle, contracts, build or production files, and cannot build
in shared output or commit. Acceptance evidence is `evidence/numeric-acceptance.md`.

`tex_parser` separately owns only `evidence/e7-options-identity-cases.json`:
read-only Options source/probe proposals covering every export, recursive
values, identities, descriptors, symbols and merge behavior. This does not
authorize Options implementation or freeze its native value contract. Root
resolved the graph contract and generated the initial JS fixtures before P3b
dispatch. E7 remains proposal evidence, never native implementation expectations.

### P3b: recursive Options graph and composition

Root owns `option_types.ixx`, private `option_storage.hpp`, source-derived oracle,
fixtures and CMake registration. The three Luna packets had these sole files:

| Agent | Implementation | Responsibility |
| --- | --- | --- |
| math_layout | util/option_object.cpp | Context-owned identity graph, prototypes, ordering, descriptors, accessors and array special property rules |
| math_fonts | util/option_array.cpp | Ordinary array creation, live spread consumption, strict indexOf and mutation-aware filter |
| tex_parser | util/options.ixx | All twelve Options.ts functions/globals through the frozen Context interface; no separate graph or parser |

Each agent also owns its corresponding `evidence/p3b-*.md` ledger and cannot
alter contracts, expected results, build files, production or other packets.
Root rejected source evaluation-order changes and the Windows stack-overflow
implementation. Exact getter reads were restored and internal copy/insert
recursion became explicit traversal frames. Revision 2 records that host
adaptation and the single-definition exported diagnostics ABI. Root accepted
593 assertions in 21 tests, eight compiling/failing isolated mutations and a
full core build/test; this does not accept TeX, font resolution or SVG.
The separate E8 packet authorizes only the read-only profile-usage evidence,
not code edits or a scope change. G0/G1–G7 remain required for later waves.

## Wave 2 — semantic tree and configuration infrastructure

### P6: generic factories only

Root freezes `factory-contracts.md` revision 2 and owns `factory_types.ixx`,
the actual-JS `factory-oracle.cjs`, generated `reference_factory.cpp`, mutation
runner, CMake, scope and acceptance. Exact packets:

| Agent | Sole files | Responsibility |
| --- | --- | --- |
| math_layout | core/tree/factory.ixx; evidence/p6-factory.md | Generic Factory members; independent class Map and stable captured creator object |
| tex_parser | core/tree/node_factory.ixx; core/tree/wrapper_factory.ixx; evidence/p6-factory-bridges.md | Direct node dispatch/default arguments; wrapper pointer/argument forwarding |
| math_fonts | evidence/e9-factory-profile-usage.md | Read-only source callers, concrete registries, constructor metadata and live-view requirements |

Agents cannot implement Node/Wrapper/MML bodies, modify frozen tests, build,
commit or wire production. Root rejected merged class/creator storage, extra
constructor/predicate validity gates, and a returned-pointer ownership gate.
Later review retained Function.prototype inherited errors after __proto__
registration and made owning factories immobile to preserve captured addresses.
Acceptance is independently recorded in `evidence/factory-acceptance.md`;
generic prerequisites do not accept any mathematical node or layout algorithm.

### P7: generic tree and visitor view

Root froze `tree-contracts.md` and node/wrapper interface storage before dispatch.
The normal and empty MML branches will implement one typed interface using the
same parent, factory and replaceable child-array storage; no alternate AST.

| Agent | Sole files | Responsibility |
| --- | --- | --- |
| math_layout | core/tree/node.ixx; evidence/p7-node.md | Every AbstractNode/AbstractEmptyNode member and executable constructor phase |
| tex_parser | core/tree/wrapper.ixx; evidence/p7-wrapper.md | Generic visitor, absent/present child arrays and base wrap argument policy |
| math_fonts | evidence/e10-tree-host-usage.md | Read-only concrete MML/common/profile usage and interface review |

Root owns node_types/wrapper_types, the added WrapperServices bridge, independent
JS fixtures, mutations, builds, scope and acceptance. Revision 2 corrects TS
spread's emitted Object.assign behavior and Array.join capture/evaluation order;
revision 3 adds Array.join's recursive-array suppression. These contracts do
not authorize MML stubs, Common algorithms or production wiring. Agents cannot
edit contracts/tests, build or commit. `evidence/tree-acceptance.md` records root
review and the 150-assertion/15-mutation local prerequisite acceptance.

Root issues separate packets for generic Tree factories, MmlNode semantics,
each concrete MML node, operator dictionary, Token/TokenMap/MapHandler,
Configuration/ParseOptions and Stack/StackItem/StackItemFactory. Each has exact
file ownership. Validate semantic nodes independently of TeX first, then maps
and stack transitions. G1/G2 infrastructure must pass; no layout may consume a
custom AST as a shortcut.

### P8a: explicit MML/input primitive host services

Root freezes `input-host-contracts.md` revision 1, `utf16_types.ixx` and
`input-host-oracle.cjs`/`reference_input_host.cpp` before dispatch. The oracle
uses actual Node 24.13.0 operations, with 3,159 frozen assertions, bit-exact
binary64 comparisons and deterministic generated inputs. This is a host
prerequisite; MmlNode.ts and every concrete mathematical node remain unaccepted.

| Agent | Sole writable files | Responsibility |
| --- | --- | --- |
| math_layout | util/js_numeric_input.ixx; evidence/p8a-numeric-input.md | Number(string), scoped parseInt(string, radix), grammar/whitespace/prefix/error/rounding |
| tex_parser | util/utf16.ixx; evidence/p8a-utf16.md | UTF-16 indexing/iteration; strict UTF-8 decoding and diagnostic byte-offset map; UTF-8 encoding boundary |
| math_fonts | evidence/e11-mml-host-usage.md | Read-only actual CJS MML state/caller audit; no implementation authority |

Root owns interfaces, expected results, CMake, mutation tests, independent
source review, builds, commits and acceptance. No agent may edit these files,
introduce object coercion, an interpreter, a parser, substitute defaults, or
wire production. A source annotation is not sufficient evidence for runtime
types or state: root rejected E11's missing-empty-child-array claim and checked
actual Text/XML construction. Extra radices are not in the frozen parseInt
API; they need caller evidence and separately frozen behavior before expansion.

## Wave 3 — base/AMS input and complete font services

TeX packets follow upstream BaseConfiguration/BaseMappings/BaseMethods/
BaseItems and AMS counterparts, with PostFilter ordering tested separately.
Other agents can implement font data generation and FontData resolver methods
only against accepted semantic/contracts. Direct-record data and effective
variant/extension/delimiter behavior have distinct verification gates.
Acceptance is G1/G2 for parsed trees and G3 for fonts. No production wiring.

## Wave 4 — common wrappers and shared scene

Start with Wrapper/Factory lifecycle and the base token/row wrappers. Dispatch
fractions, scriptbase/scripts, radicals, mo stretching, under/over/accent,
tables and other node families as separate packets, not a generic engine task.
Every packet includes actual upstream common and SVG-wrapper methods because
shared measurement and shared placement are different responsibilities.
Scene adaptations require root's mapped extraction design and per-method
evidence. G4 and per-feature G5 must pass before the next dependent family.

## Wave 5 — remaining enabled extensions, SVG and line breaking

Dispatch extensions through the accepted registry and stack mechanism. Enumerate
their dependencies and exact handlers/environments/options; all remain required
until root explicitly records a scope change. Complete linebreak traversal,
fragmentation, SVG semantics, clipping/viewport composition, paints and errors.
Pass G5 for the complete profile and composition corpus, then G6 native SVG
normalization/Direct2D pixel verification. Nonempty SVG smoke checks are not
acceptance evidence.

## Wave 6 — replace the Folia backend

**Owner:** root. **Prerequisites:** no unreviewed manifest entries, all G0–G6
accepted, complete feature/method coverage ledger and no prototype algorithms.

Replace the existing service implementation with the accepted native session
while preserving worker/cache/dependency behavior. Transfer fragment metadata
directly; remove the math JS runtime, linkage, runtime assets and SVG metric
reparse. Never add an old/new feature flag or invoke MathJax JS as fallback.
Check settings lifecycle, cancellation, editing previews, tables, PDF export,
core/editor suites and WinUI build. Commit small reviewed subsystem batches.

**Release gate:** G7. A native library build, passing a few formulas or a font
table dump is not authorization to mark the port complete or switch Folia.
