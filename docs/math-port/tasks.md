# Root-owned implementation packets

No packet is complete because an agent says it is. Root checks source fidelity,
independently runs the frozen tests, reviews the diff and records acceptance.
Agents must not modify architecture, scope, root contracts, the oracle, frozen
fixtures, build files, production integration or another packet's files.
The rejected prototype is archived. Revision-1 primitive contracts and frozen
fixtures permit only P1, P2 and P3a implementation submissions. These are not
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

Split into P3a PrioritizedList and P3b Options. Only P3a is dispatched under
primitive-contracts.md. P3b remains pending recursive option-value contracts.

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

## Wave 2 — semantic tree and configuration infrastructure

Root issues separate packets for generic Tree factories, MmlNode semantics,
each concrete MML node, operator dictionary, Token/TokenMap/MapHandler,
Configuration/ParseOptions and Stack/StackItem/StackItemFactory. Each has exact
file ownership. Validate semantic nodes independently of TeX first, then maps
and stack transitions. G1/G2 infrastructure must pass; no layout may consume a
custom AST as a shortcut.

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
