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
- Isolated compilation may read the completed Gate101 IFC/lib artifacts and
  write only to `build/math-port/isolated/<task-name>`. Never update shared
  artifacts. Capture exact source/library hashes, executable hash, PID, UTC
  interval and real exit code. Gate101 lacks current API changes: do not add
  compatibility code for that snapshot or call isolated results a current
  shared-build pass. Ask Root to compile against the new graph when needed.
- Work autonomously through the full assigned packet. Report coherent compile
  readiness, concrete findings and real observations. A Source comparator
  self-check is not native parity. Do not repeatedly rerun unchanged checks.
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
| font_packages | bbm, bboldx, dsfont observations; CommonOutputJax getBBox and Array.unshift witnesses; four-record XML native observer, coordinated with texhtml |
| mhchem_acceptance | Luna owner of Mhchem package, actual pinned parser dependency, and complete Source/native observations; continues the recorded mhchem_native implementation |
| architecture_review | Sol audit and scene_layering implementation role described above; independent recheck of live Scene fields/append behavior |
| Root | Public math facade, build integration, baseline tracking, original acceptance rules, remaining output fixes, final verification and Folia integration |

New packages remain outside production session registration until Root verifies
their complete original comparison and integration. Mermaid stays paused.

The scene_layering owner takes Scene base module declarations/exports,
`scene/menclose.{ixx,cpp}`, and typed Scene schema additions. scene_context
keeps the concrete SVG surfaces and their live Context record/array/geometry
behavior; coordinate interface edits rather than overwriting each other's work.

## Current evidence and pending changes

Gate110 is the current passing compile: 38 targets and 1,836 unchanged inputs.
Core, Host, Regexp and LinkedList pass; Parser and three typed Scene tests do
not. Complete renderer comparison is 47/75, public interface 8/9, registration
23/38. Official Mhchem dependency comparison is 7,960/7,960. Its package capture
is 219/220 despite clean diagnostic repeats. See
`evidence/shared-gate107-110.md` for the complete limits.

Registered sources are editable after that capture. Root has authorized
MapHandler changes and HTML document constructor wiring. The next shared gate
must include their complete updated dependencies. Bussproofs, Mathtools and
TagFormat/SetOptions are currently excluded as whole CMake work packets;
re-enroll them only with their complete coherent interfaces/implementations.
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
