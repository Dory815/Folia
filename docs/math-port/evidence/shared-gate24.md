# Shared parser/output-host checkpoint 24

Root independently ran the shared build and actual compiled observations on
2026-10-13. This is a worktree snapshot checkpoint, not full renderer acceptance.

## Build provenance

One existing `build/core` CMake/Ninja cache compiled the registered math library,
18 diagnostic targets and ordinary `FoliaTests`. Snapshot
`build/math-port/gate-24-inputs-before.json` contains 808 input hashes; Root
compared them after the build: zero changes. Unregistered OutputJax,
FunctionListContextBindings and their observer/tests were excluded and remained
editable. Registered source edits were released immediately after linking,
before executable tests.

The build's terminal exit is 1. The math library, ordinary FoliaTests and 16
diagnostics freshly linked. Two diagnostic test translation units failed:

- Boldsymbol test called nonexistent `FunctionList::size()`.
- ParseUtil test's UTF-16 literal became ambiguous after the raw Property
  `addArgs` overload was added.

Owners repaired those test calls after the snapshot. Those repairs are not
compiled by Gate24. No old ParserTests/BoldsymbolTests executable was counted
as a fresh Gate24 result. New native indexed-view-specific tests still live in
the unregistered OutputJax test target and have not run.

## Actual executions

| Fresh executable/check | Result |
| --- | --- |
| Ordinary FoliaTests | Exit 0; 345,517 assertions, 5,291 tests, 168 suites |
| HostTests | Exit 0; 420 assertions, 190 tests |
| RegexpTests | Exit 0; 18,518 assertions, 41 tests |
| FontTests | Exit 0; 97 assertions, 11 tests |
| StylesTests | Exit 0; 19 assertions, 6 tests |
| Styles whole-record original comparison | All 37 match |
| BaseMethods tree plus parser-state original comparison | 35/37 match |
| AMS/Newcommand result/error/maps/packageData comparison | 23/24 match |
| AMS/Newcommand supplemental comparison | 33/36 match |
| Boldsymbol/ConfigMacros complete emitted fields | 10/16 match; all ConfigMacros match |
| BegingroupTests | Exit 1; 33/34 assertions, 3/4 tests pass |
| FrontendDump | Exit 1 at TextMacros inner NodeFactory probe |
| Array.concat structured comparison | 19 matched, six explicit host boundaries, zero supported mismatches; partial |

Core statistics were parsed after removing ANSI escape codes. The actual tool
session completed with exit 0; the separate terminal core-run state records
that result. Aggregate diagnostic build failure does not mean the freshly
linked ordinary core test failed.

Several dump programs take an output-path argument rather than writing JSON to
stdout. Root initially redirected stdout and obtained empty private outputs;
those comparator attempts were invalid. Root corrected the CLI invocation,
reran the same immutable executables with explicit private output paths, and
then obtained the comparison results above. No empty-output attempt is counted
as a behavior mismatch or accepted comparison.

## Resolved and remaining Source differences

Base AMS matrix/cases spacing now matches: the two default UTF-16 literal
callers formerly constructed bool Properties and now construct actual strings.
Alignedat's omitted optional alignment no longer injects `[undefined]`;
`ams-alignedat-two-pairs` matches all observed fields.

Both remaining Base rows (`array-row-controls`, `new-column-type`) now have
Source-identical parser state. Their tree difference is an extra
`mtable.attributes.align:'undefined'`. The owner subsequently fixed the caller
to preserve Source's `align || ''` behavior; that later change is uncompiled.
Do not describe current Gate24 parser state as underconsumed.

AMS main's only remaining row is `ams-tag-reference`. The supplemental failures
are `ams-arrow-empty-labels`, `ams-tag-duplicate` and `ams-tag-starred`. Tag
reference/starred throw `Native NodeFactory scalar slot is not a Property`;
duplicate differs in MML result. Empty arrows throw when inheritance encounters
an undefined child. These require actual caller/registration/graph tracing;
relaxing MML arity or suppressing the errors is not accepted.

Frontend's actual stderr reports `textmacros-prefilter count=0 jax-factory=set`
and then a null inner NodeFactory MML factory. The outer factory is explicitly
set by the fixture; that does not prove the inner preprocess callback ran.
The fresh original CJS witness observes one real configured prefilter before
execution. New unregistered FunctionListContextBindings cannot cause the
Gate24 mismatch. The parser owner is tracing actual configuration membership
and filter insertion into the existing canonical FunctionList.

Root repeated the pinned Boldsymbol/ConfigMacros CJS oracle independently:
complete original output equals the existing corpus, SHA-256
`7a1683ebd2385cda44d273082658e54aaa437a6a78a187579aac60be120e969c`.
The original uses full `TeX.compile`. All six fresh native Boldsymbol rows
retain `fixBold` and miss the actual original rewrite. The persisted comparator
checks every emitted field (`name/result/error/maps/packageData`), case order,
coverage and schema; `input/packages/options` are explicitly Source input
metadata. There is no accepted Boldsymbol filter result yet.

## Scope

The new Context indexed-view/FunctionList production edits compiled and did
not regress the ordinary or existing focused tests. Their new reflection,
retained-view and callback-mutation tests remain uncompiled; no full host-view
acceptance follows from the generic tests. OutputJax is still unregistered.
The 35 locked original LiteAdaptor rows and file/ownership contract are a
baseline for a future port, not its implementation. CommonOutputJax, wrappers,
shared scene, SVG and Folia replacement remain outstanding. Mermaid remains
paused and untouched.
