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

## Accepted prerequisites and initial evidence packets

Root has locally accepted ten of 246 inventory entries: Attributes, BBox,
PrioritizedList, lengths, Options and five generic Tree files. The separate
native Number formatting helper is a host adaptation. The other 236 inventory
entries are still unreviewed. Broad parser/layout implementation packets remain undispatched;
agents cannot fill missing prerequisites with a substitute architecture.

The initial Luna assignments owned **one proposal file each**, never frozen
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
uses actual Node 24.13.0 operations, with 3,811 frozen assertions, bit-exact
binary64 comparisons and deterministic generated inputs. This is a host
prerequisite; MmlNode.ts and every concrete mathematical node remain unaccepted.

| Agent | Sole writable files | Responsibility |
| --- | --- | --- |
| math_layout | util/js_numeric_input.ixx; evidence/p8a-numeric-input.md | Number(string), scoped parseInt(string, radix), grammar/whitespace/prefix/error/rounding |
| tex_parser | util/utf16.ixx; evidence/p8a-utf16.md | UTF-16 indexing/iteration; strict UTF-8 decoding and diagnostic byte-offset map; UTF-8 encoding boundary |
| math_fonts | evidence/e11-mml-host-usage.md; evidence/p8a-host-review.md (separate sequential assignments) | Read-only actual CJS MML state/caller audit and independent host-contract review; no implementation authority |

Root owns interfaces, expected results, CMake, mutation tests, independent
source review, builds, commits and acceptance. No agent may edit these files,
introduce object coercion, an interpreter, a parser, substitute defaults, or
wire production. A source annotation is not sufficient evidence for runtime
types or state: root rejected E11's missing-empty-child-array claim and checked
actual Text/XML construction. Extra radices are not in the frozen parseInt
API; they need caller evidence and separately frozen behavior before expansion.

### P8b: MmlNode semantic hierarchy

Root froze `mml-contracts.md`, public semantic declarations, class-default and
tuple bindings, Property null/object support, and actual pinned-CJS expectations
in 0b315bc before dispatch. The source has 111 inventoried members, 85 executable
members; root independently expanded the oracle to 1,084 assertions in 43 cases
(1,077 actual-runtime assertions plus seven explicit native resource assertions).

| Agent | Sole implementation/evidence files | Responsibility |
| --- | --- | --- |
| math_layout | core/mml_tree/mml_node.cpp; evidence/p8b-normal.md | Ordinary semantic node lifecycle, TeX classes, inheritance, verification/errors and policy tables |
| tex_parser | core/mml_tree/mml_branches.cpp; evidence/p8b-branches.md | Token class-identity text/visitors, inferred-layout delegation, base/script class propagation |
| math_fonts | core/mml_tree/mml_empty.cpp; evidence/p8b-empty.md | Empty read-only/no-op branch, lossless TextNode, opaque XML service/copy |

After completing Empty, math_fonts has a separate read-only review assignment
owning only `evidence/p8b-review.md`. Root owns all interfaces/bindings/host
adapters, fixtures/oracle, source ledger, mutations, CMake, builds, scope,
commits and acceptance. No agent builds, edits shared files, invents concrete
node classes/parser/layout rules or integrates production. Test-only registered
subclasses validate the actual base methods through injected factories; they
cannot substitute for production MML-kind implementations. No mathematical
node gate is passed merely by compiling this hierarchy.

### P9: concrete row/root/token/style nodes

Root froze `leaf-contracts.md`, ten per-file module interfaces, native class
metadata/default bindings, primitives and 610 actual-CJS assertions in commit
5f50819 before dispatch. The source inventory has 62 members, including 39
executable members. Exact writable packets were:

| Agent | Source files under core/mml_tree/mml_nodes | Evidence |
| --- | --- | --- |
| math_layout | mrow.cpp; math.cpp | p9-row-math.md |
| tex_parser | mi.cpp; mn.cpp; mtext.cpp; ms.cpp; mspace.cpp | p9-tokens.md |
| math_fonts | mstyle.cpp; merror.cpp; mphantom.cpp | p9-style.md |

Root supplies all static defaults and bindings, class_binding, leaf_host and
all interfaces, fixtures, mutation tools, CMake and acceptance. An independent
review is evidence only. Agents cannot register a partial MML factory or wire
production. Root's source review and differential tests rejected bypassed
virtual class setters, collapsed attributes getter reads, incorrect C++ enum
qualification and Missing scalar arguments bypassing source entry defaults.
Final fixtures currently have 669 assertions in 27 cases, including six native
resource assertions and five entire-UTF-16-unit-domain bitset comparisons.
Root accepted all 29 compiling/failing isolated mutations, the updated base
suite and its twenty mutations, and the full core suite. Detailed acceptance
is evidence/leaf-acceptance.md; no complete renderer gate is implied.

### P10: script nodes and UTF-16 string helpers

Root froze script-contracts.md, source hashes, module declarations, copied
family statics, captured actual-constructor ACCENTS bindings and 1,270 initial
actual-CJS assertions in commit 5240599 before dispatch.

| Agent | Sole implementation | Sole ledger |
| --- | --- | --- |
| math_layout | mml_nodes/msubsup.cpp | evidence/p10-subsup.md |
| math_fonts | mml_nodes/munderover.cpp | evidence/p10-underover.md |
| tex_parser | util/string.cpp | evidence/p10-string.md |

Agents cannot modify interfaces, shared host operations, data bindings, oracle,
fixtures, CMake, scope or production; no builds/commits. Root reviewed every
one of 43 AST-inventoried members (35 executable), corrected eager over getter
evaluation and native numeric error formatting, and independently validated
the final 1,689 assertions/18 cases (1,683 actual-CJS and six native resource
assertions), all 28 compiling/failing mutations, and the complete core suite.
Root hardened malformed-trace indexing in the test harness, and configured
16 MiB Windows test-executor stacks after real 512-deep Debug hooks exhausted
the default reservation; G7 must configure/verify native math worker stacks.
evidence/script-acceptance.md records local acceptance and explicit boundaries.

The separate operator-dependencies.md audit records live definition aliases,
ordered range lookup and LinebreakVisitor module-load mutation. It authorizes
no dictionary/visitor implementation before root freezes its own interfaces
and fixtures. mo, complete factory registration and all renderer gates remain
unfinished; 24/246 local source acceptances do not establish a complete port.

### P12a: root-owned class-state prerequisite, before Mo dispatch

Frozen authority: class-state-contracts.md and actual-CJS fixture in 8260318.
Root owns all implementation/interface/oracle/build changes in this packet.
The existing numeric optional class boundary was rejected after a real profile
`\\mmlToken{mo}{toString}` witness; no Mo implementation is authorized yet.

| Owner | Exact files | Deliverable and limit |
| --- | --- | --- |
| Root | mml_node.ixx/.cpp, mml_empty.cpp, mml_branches.cpp, mrow.cpp, property.ixx, mml_node_host.ixx, leaf_host.ixx | One raw class field, source getter order/nullish selection/spacing, narrowly bound number boxing; remove unnecessary getter-helper const_cast; no parser, scene or production changes |
| Root | class-state-oracle.cjs, reference_class_state.cpp, mutation tools | Locked actual-CJS expectations, state/read traces, previous and current script-level branches; unchanged expectations for existing packets |
| math_layout | evidence/p12-class-state-review.md only | Read-only per-method comparison; report failures to root, never repair shared source or certify acceptance |
| math_fonts | evidence/p12-class-fixture-review.md only | Read-only coverage/lock audit; report missing branches, never change oracle values |
| tex_parser | evidence/p12-number-boxing-review.md only | Read-only audit of bound numeric prototype services against actual Node runtime; list observed discrepancies and scope limits |

Acceptance belongs to root: source review, reproducible oracle output, complete
core regression, and compiling/assertion-failing isolated mutations for raw
state, rereads, strict equality, transfers and script-level ordering. Existing
affected packet mutation anchors must remain effective. Review reports alone
never accept a packet. No agent may edit contracts, declarations, CMake,
fixtures, registry or runtime wiring; no agent builds or commits. Contract
changes return to root before dependent implementation resumes. Root locally
accepted P12a in evidence/p12-class-state-acceptance.md: 13,441 source-derived
assertions, all 20 new mutants, 77 existing mutants and the final complete core
suite. Read-only helper constness correction was verified independently. This
does not authorize Mo dispatch until its own declarations and fixtures freeze.

### P12b: exact Mo pattern prerequisites

Root freezes mo-pattern-contracts.md, mo_patterns.ixx, actual source metadata
and 971 initial assertions/16 cases before dispatch. The reference locks the
entire 424-file compiled source package, the exact Mo TS/CJS and Node version.
The fixture makes 2,342,329 actual source queries, including 30 entire UTF-16
domain masks, mixed class products, malformed text and callback lifetimes.
Root owns the data, oracle, tests, CMake, mutations and acceptance.

| Agent | Sole implementation/review file | Sole evidence |
| --- | --- | --- |
| tex_parser | core/mml_tree/mo_patterns.cpp: matchMoPattern, makeMoPattern only | evidence/p12b-pattern-implementation.md |
| math_layout | Read-only source/data/interface review | evidence/p12b-pattern-source-review.md |
| math_fonts | Read-only oracle/coverage/lock review | evidence/p12b-pattern-fixture-review.md |

Agents cannot change shared interfaces, generated data, tests/expectations,
contracts, CMake, source lock, registry or production; no builds/commits.
Needed changes return to root. This packet ports the five known no-flags UTF-16
predicates through compiled callbacks, not a generic JS regex VM. Default source
patterns, bounds and data stay exact; later Mo live static/constructor services
remain a separate root-frozen packet. No partial Mo registration or broader
gate acceptance follows from this prerequisite. Root locally accepted the
service in evidence/p12b-pattern-acceptance.md: final 1,136 assertions/16 cases,
2,342,369 source queries, all 22 compiling/assertion-failing mutants and the
complete core suite. Root added range metadata and multiletter whitespace
coverage after read-only review. Five of 32 Mo members are mapped prerequisites;
the remaining 27 are unaccepted. Complete Mo still requires root-owned live
static/constructor bindings and node-method fixtures before dispatch.

### P12c: complete Mo node, disjoint source-method packets

Root freezes mo-contracts.md, mo.ixx, mo_types/mo_host, generic captured
staticServices metadata, exact generated mo_bindings.cpp and actual-CJS
mo-oracle.cjs fixtures before dispatch. All 32 source members require individual
mapping; the five patterns already passed P12b. Root owns all shared interfaces,
source locks, expected values, build, mutations, scope and final acceptance.

| Agent | Only source under core/mml_tree/mml_nodes | Only evidence |
| --- | --- | --- |
| tex_parser | mo_lookup.cpp (six named methods in mo-contracts.md) | p12c-mo-lookup.md |
| math_layout | mo_classes.cpp (five named methods) | p12c-mo-classes.md |
| math_fonts | mo_scripts.cpp (six named methods) | p12c-mo-scripts.md |

No agent edits declarations, shared hosts, data, tests/oracles, CMake, source
locks, scope, another packet or production; no builds/commits. Report interface
gaps before proceeding. Root independently reviews every method, extends source
fixtures to close gaps and requires compiling/assertion-failing mutants. Neither
agent completion nor initial fixtures alone establishes Mo or renderer parity.
No partial production MmlFactory registration or backend switch is authorized.

Root accepted P12c after source-by-source review and corrections, final
actual-CJS fixtures (23,577/14, 538/50 and 74/14 assertions/cases), complete
core regression and 33 compiling/assertion-failing isolated mutants. All 32
source members have individual reviewed mappings in evidence/p12-mo-members.json.
Required function-intrinsic metadata is an explicit host binding. Evidence:
evidence/p12c-mo-acceptance.md. Local source acceptance is 26/246; full G0–G7,
the full MML factory and all production integration remain unfinished.

### P13: fraction and radical semantic nodes

Root froze arithmetic-contracts.md, three class interfaces, arithmetic_host,
generated binding/default files and arithmetic-oracle.cjs before dispatch.
AST inventory has 21 total members in mfrac/msqrt/mroot.ts (16 executable).
The six agent-owned methods are split by source node; constant getters,
defaults and radical field phases remain root-owned. Initial actual-CJS
fixture has 13,501 assertions/7 cases and 1,440 raw inheritance rows, plus
array/attribute mutation, alias, malformed-state and iteration evidence.
Root extends coverage gaps, reviews every method, runs core regression and
compiling/assertion-failing mutants. No geometry, parsing or production wiring.

| Agent | Only CPP | Only evidence |
| --- | --- | --- |
| tex_parser | mml_nodes/mfrac.cpp | evidence/p13-mfrac.md |
| math_layout | mml_nodes/msqrt.cpp | evidence/p13-msqrt.md |
| math_fonts | mml_nodes/mroot.cpp | evidence/p13-mroot.md |

No builds/commits/shared edits or changed oracle expectations by agents.
Root accepted P13 after complete member review, full core regression, final
actual-CJS fixtures (13,501/7 and 56/22 assertions/cases) and 34 compiling,
assertion-failing isolated mutants. The virtual-helper witness rejected root's
initial own-spread assumption: the pinned CJS uses __assign/Object.assign.
Revision 2 binds that operation; existing addInheritedAttributes required no
change. All 21 member mappings are reviewed in evidence/p13-arithmetic-members.json;
evidence/p13-arithmetic-acceptance.md records the corrections and rejected runs.
Local source acceptance is now 29/246. Geometry, parser, full MML factory and
SVG/Folia gates remain unaccepted.

### P14: table semantic family — root-frozen packet

Root read mtable/mtr/mtd.ts and reviewed the agents' read-only reports in
evidence/p14-{mtable,mtr,mtd}-audit.md. The 26-member family design and ownership
are fixed in table-contracts.md. Root corrected minimum-arity and receiver-order
errors in the reports. Root now freezes the interfaces, 29 default additions,
12 constant getters, ordered table properties/class fields, primitive split/
index/factory bindings and independent actual-CJS oracle (3,783 assertions /
116 cases / 240 raw rows) before assigning the nine node methods. Each agent
receives only one listed CPP and one method report; root owns all builds,
mutations and acceptance.
Tables retain MML inheritance/verification/tree repair here; all table geometry
and backend work remain in later Common/scene packets.

Root accepted P14 after reviewing all 26 members and correcting null-child
dispatch timing in the table/row output loops. Final actual-CJS fixture passes
4,700 assertions/135 cases; complete core regression and 39 compiling,
assertion-failing isolated mutants pass. Working sources and fixture remain
byte-equal after mutations. Evidence: evidence/p14-table-acceptance.md and
the individual mappings in evidence/p14-table-members.json. Local source
acceptance is 32/246; full G0–G7 remains closed.

### P15 prerequisite audits — no implementation authorization yet

Agents have produced read-only audits of mmultiscripts, TeXAtom, semantics and
HtmlNode in evidence/p15-*-audit.md. Root must freeze source-ordered class
construction, live multiscript array edits, nullable verification and the HTML
adaptor contract before implementation dispatch. TeXAtom's final method is the
actual Mo adjustTeXclass function installed by prototype assignment; the
class-body identity method is not the effective implementation. Root must
design and revalidate a shared operation instead of accepting a duplicated
algorithm or a test-only Atom descriptor. These audits are not acceptances.

### P15a: multiscript semantic packet

Root freezes multiscript-contracts.md, the three-class interface, primitive
array/error host operations, three generated defaults copies and six accessors,
13-member AST inventory and independent actual-CJS oracle before dispatch.
The initial oracle has 1,286 assertions/222 cases, 480 raw inheritance rows
and 1,161 source-value records. Source callbacks distinguish in-place insertion
from virtual append, live property traversal, raw prime OR/level addition,
captured fix, shared attributes and null/factory partial states. Real error
trees and direct-parent/null-options cases complement returning-error probes.
Two read-only reviews informed explicit direct-parent and partial-mError rules.

tex_parser owns ONLY mml_nodes/mmultiscripts.cpp and
evidence/p15a-mmultiscripts.md for four methods. Root owns declarations,
bindings, tests, builds, mutations and acceptance. Reviewers may inspect but
cannot edit shared code or expected results. TeXAtom, semantics and HtmlNode
remain separate future packets; this assignment includes no geometry or wiring.

Root's subsequent review added kind-order traces, in-loop script/verification
array callbacks, and error-options identity/mutation through the actual argument.
The expanded fixture has 1,304 assertions/226 cases and passes its focused
native build. Root corrected generated test naming/API errors without changing
source expectations. Complete core regression and all 34 compiling,
assertion-failing isolated mutants pass. Root accepted all 13 member mappings;
evidence: evidence/p15a-multiscript-acceptance.md. Local source acceptance is
33/246; Common/scene/SVG and full G0–G7 remain closed.

### P15b/P15c: shared TeXAtom class operation and semantics

Root owns operator_class extraction from the accepted Mo method. Mo and
TeXAtom delegate to one MML operation, with one algorithm guard; the typed
native API's lack of JS function-object reflection is explicit in
atom-semantics-contracts.md. Root also freezes the two interfaces, generated
defaults/accessors/field-constructor phases and actual-CJS oracle (6,828
assertions / 63 cases / 924 raw rows) before dispatch. The AST inventory has
18 class members plus the separately recorded final prototype assignment.

tex_parser may implement only tex_atom.cpp's two methods and its P15b report;
math_fonts may implement only semantics.cpp's source-empty annotation hook
and its P15c report. Root owns every declaration, expected observation, build,
mutation and acceptance. The existing Mo semantic fixture will register a
real TeXAtom; narrow dummy-kind witnesses stay isolated and explicitly scoped.
The previous 33 Mo mutants and new packet-specific mutants remain required.

## Wave 3 — base/AMS input and complete font services

P11 is locally accepted under operator-contracts.md revision 2. Root froze the
pristine graph/interfaces and initial source expectations in f027581, then
constrained one agent to OPDEF/getRange and two to read-only reviews. All six
OperatorDictionary exports are mapped in p11-members.json. The final fixture
has 13,578 assertions/nine cases, including every initial graph edge/operator
key/range boundary and the full Unicode range domain with empty form tables.
All 21 compiling mutants fail unchanged assertions; full core regressions pass.
Root corrected array has/prototype services and owns the shared-array MML tuple
bridge. LinebreakVisitor startup mutation remains separate. 25/246 local source
acceptances do not pass renderer gates. Next mo audit is read-only; no
implementation dispatch is allowed before root freezes its own raw-class,
constructor static, regex and getter-side-effect interfaces and independent tests.
The read-only p12-mo-dependencies.md audit and root mo-dependency-probe.cjs show
a real profile witness: \mmlToken{mo}{toString}/constructor retain undefined
classes, distinct from null. The initial nullable numeric MML class binding was
therefore insufficient for Mo. P12a now corrects that shared boundary to one raw
Property state and revalidates the MML/leaf/script packets; see its acceptance
evidence. Mo's live constructor/static/regex and getter interfaces still require
root-owned declarations and fixtures before implementation dispatch. The
audit is a prerequisite report, not Mo acceptance or permission to omit quirks.

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

### P15b/P15c root acceptance

The source packet is locally accepted: all 18 class members and the final
prototype assignment are mapped. Root corrected source-ordered base field writes
and expanded actual-CJS witnesses to 6,888 assertions/74 cases. Complete core
regression passes; 26 packet mutants, 33 Mo mutants and 20 class-state mutants
compile and fail unchanged assertions. Evidence: evidence/p15bc-atom-semantics-acceptance.md.
Local source acceptance is 35/246; full G0–G7 remain closed.

P15d is only a read-only HTML/XML host prerequisite audit. Root must freeze
its interfaces, exact source methods, host obligations and independent fixtures
before any implementation dispatch. Disabled texhtml defaults do not authorize
omitting the configurable enabled path.

### P15d root acceptance

Root froze the HTML semantic node and its typed host calls in f7d8ce7 before
Luna implementation. All seven source members are mapped; final actual-CJS oracle
has 434 assertions/75 cases. Full core regression and 26 compiling/assertion-failing
mutants pass. Evidence: evidence/p15d-html-node-acceptance.md.
Local source acceptance is 36/246; full G0–G7 remain closed.
Concrete DOM/texhtml parsing, HTML measurement and SVG foreignObject are still
required separately. P16-remaining-mml-audit.md is a read-only prerequisite,
not implementation authorization or acceptance of the remaining eight sources.

### P20a root acceptance

Complete MapHandler.ts is locally accepted under map-handler-contract.md:
21 inventoried members plus both MapHandler object methods, 35 actual-CJS
probes/153 observations, 198 native assertions/22 tests, 29 compiling and
assertion-failing mutants, and final full core exit 0. Evidence:
evidence/p20a-map-handler-acceptance.md. Local Source acceptance is 52/246.
TokenMap concrete algorithms, regexp engine, complete TeX/Common/SVG and all
Folia release gates remain open. Mermaid remains separately required before
goal completion; its radial Source audits freeze no implementation/API.

### P16a root acceptance

The five menclose/mpadded/maligngroup/malignmark/mglyph sources are locally
accepted after root's interface freeze and source review. All 22 source members
are mapped. Final actual-source oracle has 7,667 assertions / 106 cases;
complete core regression passes; 42 compiling/assertion-failing mutants pass.
Evidence: evidence/p16a-simple-mml-acceptance.md. Local acceptance is 41/246,
full G0–G7 remain closed. The remaining 205 sources are not accepted by this
packet. Mermaid's parallel native-rendering goal is recorded separately in
docs/mermaid-native/architecture.md and remains required before goal completion.

### P20c root acceptance

Complete util/numeric.ts is locally accepted under numeric-reductions-contract.md:
two inventoried functions, 145 actual-CJS cases/290 exact numeric observations,
290 native assertions, seven compiling assertion-failing mutants and final
full core exit 0. Evidence: evidence/p20c-numeric-reductions-acceptance.md.
Local Source acceptance is 53/246; Common mtable, complete TeX/Common/SVG,
Mermaid backends and Folia release gates remain open.

### P21 root progress — prerequisites, no new MathJax file accepted

P21b isolates generic Unicode escape grammar in an ignored candidate copy;
P21c uses the original generator with Unicode 16 data. Root-owned five corpora
retain all failures and report no regressions. The original 119,218 cases still
have 376 mismatches, legacy corpus 18, and new 286,500 property cases 274.
Unchanged upstream suites remain failed; no engine/host/TokenMap acceptance.
See evidence/p21b-unicode-escape-root-review.md and
evidence/p21c-unicode16-root-review.md. Local acceptance stays 53/246.

P21a NodeUtil and child-array audits identify a real sparse-array prerequisite:
hole, present undefined, explicit null and node must remain distinguishable,
with one live aliased storage. Dense nullable vector cannot provide that.
No NodeUtil implementation or storage/API freeze is accepted by the audits.
Root must freeze the single authoritative replacement and independent Source
cases before any migration/delegation. Math remains required in parallel with
Mermaid; M3f locally accepts shared radial stage ordering after M3e geometry,
but complete backends and Folia gates remain open.

### P21d root progress and M3g local acceptance

P21d's ignored two-file point-set repair is reproducible from tracked patches.
The six actual-Node corpora have zero regressions in the correct adapter;
original remaining differences are 293, legacy 18, property aliases 144 and
the new corpus's ordinary-ui controls 28. The v/vi point-algebra observations
agree; the 101,232-case mutation baseline agrees and all 11 compiling semantic
mutants are detected. Unchanged upstream remains 495 pass/56 fail/1 ignored.
Evidence: evidence/p21d-v-casefold-root-review.md. No engine/host/TokenMap is
accepted; MathJax Source acceptance stays 53/246.

P22 parser and TokenMap audits inventory constructor/identity, MacroRef, stack,
factory and registry dependencies. They freeze no implementation or API.
Root must resolve those boundaries before authorizing implementation, including
base-constructor immediate registration and one live shared map identity.

Mermaid M3g locally accepts six shared stop/color preparation methods, following
M3e geometry and M3f stages. Root actual Source: 3,242 cases/382,002 assertions,
22 detected compiling semantic mutants, final 40 tests including 11 WARP with
the pinned font. Evidence: docs/mermaid-native/evidence/m3g-gradient-stops-
acceptance.md. Complete wrapper/sampling/resources/backends and Folia remain
required; Mermaid rendering-function acceptance stays 34/70.

### P22a Root local acceptance and M3h freeze

P22a locally accepts canonical callable Property bindings and prototype-aware
sparse slice needed by map configuration. The independent Node/CJS corpus has
640 slices, 169 comparisons and 16 graph/call flags. Final native suite:
4,040 assertions in five tests; all 22 compiling semantic mutants detected.
Complete core: 3,927 tests/161,045 assertions pass. Existing callable receiver
matrix retains all 12 detections. Evidence: evidence/p22a-function-property-
acceptance.md. This is native host work; Source count stays 53/246.
Whole concrete TokenMap/regexp/parser/Common/SVG and Folia remain required.

Mermaid M3h freezes one portable gradient program from accepted M3g stops and
M3f radial plans, including complete wrapper/color expansion. Root created
actual Source observations and compiled three failing native stubs before Sol
implementation. Contract: docs/mermaid-native/m3h-gradient-program-contract.md.
Acceptance remains Root-owned; complete backends and Folia gates remain open.

M3h now has Root local acceptance of the complete shared gradient wrapper and
color context: 13,988 programs/1,296 colors/8,010 powers, 2,130,397 assertions,
34 effective compiling mutants and final 43 tests including 11 actual WARP.
Evidence: docs/mermaid-native/evidence/m3h-gradient-program-acceptance.md.
Source warning omission was repaired by Root, and initial equivalent-rounding
mutation survival remains recorded separately. No Source expectations were
weakened. Sampling/precision/resources and complete SVG/Direct2D/Folia remain
required in parallel with the complete native Math path.

### P22b and Mermaid M3i1 Root local acceptance

P22b accepts the complete TokenMap class family: all 22 executable members and
the initialized Map field retain original per-source responsibilities and
inheritance. Root froze ownership/registration/Map/callable/argument contracts
and independent actual Source tests before Luna implementation. Final native
870 assertions/13 tests and 41 effective compiling semantic mutants pass;
complete core passes 3,940 tests/161,915 assertions. The original MapHandler
29-mutant and callable 12-mutant regressions also pass. Root corrected an unsafe
mutation observer without changing any Source expectations. Evidence:
evidence/p22b-token-map-acceptance.md. Local Source acceptance is now 54/246;
RegExpContext remains a controlled dependency, with engine/configured parser,
sparse NodeFactory/Common/SVG and Folia gates still required.

Mermaid M3i1 accepts shared whole-pipeline compiler control as a bounded portable
projection: all 79 stage identities/lowp table, complete-list precision/width,
full/tail order/repetitions/Return, context default/movement and capacity result.
Root actual Source compiler pointers: 12,872 cases/64,552 assertions; all 27
compiling semantic mutants detected. The fifteen-target matrix passes 45 tests,
including 11 WARP. Evidence: docs/mermaid-native/evidence/m3i1-pipeline-control-
acceptance.md. Executable stage sampling and complete context/blitter/GPU paint,
shared scenes and full SVG/D2D/Folia remain open; render-function count is 34/70.
