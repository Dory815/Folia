# Active native math work ownership

Root coordinates the full native MathJax goal. This document assigns complete
work packets for the resumed 2026-10-15 run; it does not accept their behavior.
Read `architecture.md`, `status.md`, and the repository AGENTS.md first.

## Common constraints

- Use ordinary local C++ software development and testing language in messages,
  comments, test names and documentation. Explain exceptions, property changes,
  lifetimes and regression tests plainly.
- Preserve the full Folia 38-package profile and three font extensions. Do not
  remove required features, weaken a comparator or change the original output
  to match the native implementation. No JavaScript engine in the native math
  path, no precomputed formula outputs, no input-specific fixes.
- Reference: `.reference/MathJax-src`, MathJax 4.1.3, commit
  `fb987178c2d279a99b0db5ea02e751691703955e`. Scope JSON SHA-256:
  `9705d389538b2fc8bf782feea4372ace639b3727304f2971937bbcfb0042a0d1`.
  Inspect actual source and existing C++ APIs before implementing or importing.
- Use C++23 modules and `import std;`. Tests include the existing shared
  Boost.UT module header and do not import std themselves. Core/math code stays
  platform independent. Source file responsibilities remain individually
  traceable. Keep functions readable, with normal indentation and line breaks.
- Preserve actual owners and canonical identities. Live Context fields and
  arrays project their existing native owner; no copied maps, shadow arrays,
  empty placeholder objects, duplicate layout graphs or legacy alternatives.
- Root owns CMake, production package registration, shared builds, final
  acceptance and Folia integration. Do not run a shared build or change CMake.
  Do not commit another owner's work. Coordinate shared API needs with Root
  and the named owner; do not edit their files unilaterally.
- Unfinished tests stay in `tools/math-port/drafts`, outside `tests/math`,
  which CMake collects automatically. Use unique package-prefixed tool names.
- Isolated compilation may read the latest completed shared IFC/lib artifacts and
  write only to `build/math-port/isolated/<task-name>`. Never update shared
  artifacts. Prefer reading shared artifacts over making large duplicate
  library/IFC snapshots. Capture exact source/library hashes, executable hash,
  PID, UTC interval and real exit code. Older gates may lack current API
  changes: do not add compatibility code for a snapshot or call isolated results a current
  shared-build pass. Ask Root to compile against the new graph when needed.
- Work autonomously through the full assigned packet. Report coherent compile
  readiness, concrete findings and real observations. A Source comparator
  self-check is not native parity. Do not repeatedly rerun unchanged checks.
- Do not configure or build a complete separate CMake/FoliaTests graph. Compile
  only the necessary translation units. Changed native layouts require a new
  coherent shared build for runtime acceptance; do not mix new IFCs with an
  older library to claim the current graph passes.
- Prefer Luna for subsequent implementation and package observations. Root
  owns architecture, review and acceptance; use Sol only for a specific need
  that warrants it. The existing Sol review and Mhchem work remain recorded.

## Ownership

| Agent | Exclusive implementation scope |
| --- | --- |
| parser_projection | TeX/ParseOptions canonical projections, ParserRuntime Map/FunctionList owners, NodeFactory projection, original ChildArray indexed views, directly affected parser callers and focused draft tests |
| tex_nextsix | AMSCD, Cases, Centernot, Empheq, HTML, Textcomp, Units package files and their complete existing next-six observers |
| physics_colortbl | Physics and Colortbl packages, Source locks, complete observers and draft tests |
| scene_context | Shared SceneContextNodes, Maction/TimerHost, Menclose shared projection integration, geometry attribute adapters, required Lite scoped target APIs and render-session wiring |
| scene_layering | Scene foundational type/builder module ownership and imports, backend-neutral Menclose service boundary, typed completed-Scene text/paint/viewport data; coordinate Surface adapter signatures with scene_context |
| mathtools | Complete six-file upstream Mathtools package port and package-specific Source/native observations |
| bussproofs | Complete five-file upstream Bussproofs package port and package-specific Source/native observations |
| texhtml | TexHtml package, actual HTMLDomStrings owner, Common/SVG/Scene XML node path and package-specific Source/native observations |
| tag_setoptions | TagFormat/SetOptions packages and complete observations; explicit per-runtime MapHandler ownership and coordinated registry callsites/tests |
| font_packages | bbm, bboldx, dsfont observations; accepted Common getBBox/Array.unshift witnesses; new Handler/PrioritizedList canonical binding packet |
| map_storage_design | Generic ordered map storage and live Context Map projection; TokenMap/SubHandlers storage owners, coordinated with binding packets |
| token_identity_design | Canonical emitted TokenMap/Token/Macro class, field and callback bindings; independent of ParserRuntime integration |
| mml_attributes | Canonical Attributes class/layer/method bindings and focused observations; parser_projection owns node integration |
| mhchem_acceptance | Luna owner of Mhchem package, actual pinned parser dependency, and complete Source/native observations; continues the recorded mhchem_native implementation |
| architecture_review | Sol audit and scene_layering implementation role described above; independent recheck of live Scene fields/append behavior |
| Root | Public math facade, build integration, baseline tracking, original acceptance rules, remaining output fixes, final verification and Folia integration |

New packages remain outside production session registration until Root verifies
their complete original comparison and integration. Mermaid stays paused.

Luna is the default for implementation, diagnosis and tests. Use Sol sparingly
for a specific, bounded question when needed; the latest callable-boundary Sol
review is complete and produced no edits. Root retains architecture and final
acceptance ownership.

The scene_layering owner takes Scene base module declarations/exports,
`scene/menclose.{ixx,cpp}`, and typed Scene schema additions. scene_context
keeps the concrete SVG surfaces and their live Context record/array/geometry
behavior; coordinate interface edits rather than overwriting each other's work.

Current shared-file exceptions are explicit: mathtools owns BaseMethods Array
and AMS AmsEqnArray argument/default corrections; parser_projection owns
BaseItems/NodeFactory fixes. Texhtml owns the narrow Lite append binding
property-access correction, coordinated with scene_context. Root owns session
package enrollment. See `canonical-handler-graph-plan.md` for the new binding
packet APIs and acceptance requirements.

## Current evidence and pending changes

Gate113 is the latest passing shared compile: 46 targets and 1,904 unchanged
inputs. Core (345,659 assertions), Host, Regexp and LinkedList pass. Parser and
three new TexHtml/DOM/XML test groups still require corrections. Complete
renderer comparison is 53/75, public interface 8/9, registration 23/38. Mhchem
dependency comparison is 7,960/7,960 and its first package capture is 220/220.
A repeat's single difference came from UTF-8 decoding across output chunks;
the tracked capture helper now concatenates bytes before decoding. See
`evidence/shared-gate111-next.md` for evidence and limitations.

Registered sources are editable after that capture. Root has authorized
MapHandler changes and HTML document constructor wiring. The next shared gate
must include their complete updated dependencies. Bussproofs, Mathtools,
TagFormat/SetOptions, TexHtml and HTMLDomStrings are enrolled in Gate113.
Subsequent corrections and the canonical Map, PrioritizedList, Handler,
TokenMap and Attributes bindings await the next shared build. Isolated
Bussproofs 31/31 plus 2/2 maps, TagFormat/SetOptions 23/23, and next-six 18/20
observations remain local evidence, not a current shared pass.
No new package is production-registered from isolated compile results.

Parser projection owns the approved ContextFunctionIdentity handle and explicit
NodeFactory call conversion/lifetime boundary; coordinate parser_runtime.ixx
with MapHandler's owner. Texhtml owns six parser and two HTMLDomStrings native
records; font_packages owns the four XML records in a separate new observer.
Temporary diagnostics may compile instrumented copies of another owner's
source under the task's ignored isolated directory without changing that
production file. Such diagnostics do not count as acceptance.

Historical observations below are retained for context.

The latest successful shared compile is Gate106: 31 targets and 1,804 captured
inputs unchanged. Its runtime acceptance fails: all 75 complete renderer
records encounter the missing generic Array.unshift, Parser and Foundation
tests fail, and Core has two SourceStringMap failures. Host, Regexp and
LinkedList pass. Gate101 remains the earlier complete passing Core checkpoint
(345,659 assertions / 5,325 tests); it does not certify subsequent edits.
Production registration remains 23/38 with 15 warnings. Gate107 terminated with
one ParseOptions compilation failure and zero changed captured inputs. The
responsible owner corrected that source; Gate108 is the next shared build.
Inspect shared-build-state.json before any build.

Root's isolated public facade plus table separator fix gives 9/9 exact public
SVG/error records and traverses nine detached Scenes (213 nodes). Isolated
Foundation with the generic Lite dispatch correction passes 390 assertions /
59 tests. The table correction over Gate101 also gives 55/75 exact complete
records and 13/14 table records; the remaining table case is Colortbl. These
are separate isolated observations, not a new complete shared checkpoint.

Parser projections, shared Scene node fields and geometry mutations are
unfinished source candidates. Physics's initial compile issues were corrected
in several passes; use actual current NodeFactory owner types and audit all
calls before declaring readiness. Physics has a pinned 63-case / 11-map Source
oracle; Colortbl has a pinned 29-case / 1-map Source oracle. Their native
comparisons remain unproven. Folia still uses the existing math worker.
