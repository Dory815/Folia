# Markdown syntax conformance

## Current checkpoint (2026-10-22)

The user authorized syntax corrections using md4c as a reference **after**
running the official CommonMark examples. This is a syntax implementation
task, not an editor architecture migration. The previous Folia UI high-CPU
investigation is paused at the user's request.

Initial official run: all **652** CommonMark 0.31.2 examples executed;
**292** strict semantic-tree matches, **360** differences. Inline source/CST
losslessness: **652/652**. Exact save serialization: **571/652**, with 81
differences (an additional editor invariant, not a CommonMark requirement).
With all three assertions, 403 test cases fail, 249 pass. Native exit is the
normal test-failure code 1; no abnormal process exit. No cases are skipped.
Full immutable pre-fix results are in ignored
`build/markdown-conformance/baseline/`; current results overwrite the parent
directory. Generated fixtures and third-party sources stay under ignored build.

The 360 semantic differences currently comprise 30 that match only after
diagnostic list-paragraph unwrapping, 28 carrying HTML policy/projection labels,
45 carrying other model/projection limits, and 257 without a known projection
limit. These are not 360 independently established parser defects. The strict
tests remain failing for every difference; relaxed comparison never passes a
formal test. The test projection translates empty editing paragraphs, code
content's separately stored line endings, and standalone image blocks to their
standard semantic shapes. It never reparses Markdown or consults expected HTML
to manufacture results.

The pinned md4c reference is
`f9e5eb232b95b737373ae706648cb75bd259ed1a`; the independent reference tests run
only the failed official subset and compare md4c output with **official** HTML.
Source hashes and licenses: `tools/markdown-conformance/sources.lock.json`.

The reference target executed all 360 failed cases: 331 exact HTML matches,
29 formatting-only newline differences; shared canonical comparison gives
**360/360 semantic matches**. Its exact-byte native assertions intentionally
still return failure code 1; do not describe those 29 as Markdown syntax bugs.
Logs: `build/markdown-conformance/md4c-build-test.log`, `md4c/results.json`.

Exact stop point: no live native build. Root has added (not yet compiled)
immutable Unicode 15.1 P/S + Zs + full-casefold helpers to existing `utf.ixx`,
and shared reference-label identity in `parsing/link_label.ixx`. These are
leaf syntax utilities; global editing `is_cp_white` remains unchanged.
Root owns native builds/Git; max build parallelism 4. Next: integrate scalar
candidate + emphasis + links + containers, then block leaves as delivered;
apply rendering normalization with explicit original source spans, run official
and existing core tests. Candidate directories below hold base and candidate
full files; merge against the base, never overwrite another family's changes.
Update this checkpoint as candidates/results change; do not rely on old agent
completion messages after context recovery.

| Luna task | Candidate / stopping point |
|---|---|
| `commonmark_projection` | Emphasis delivered: `build/markdown-conformance/candidates/emphasis/{base,candidate}/inline_parser.ixx`; depends on root UTF helpers; not compiled |
| `commonmark_official_tests` | Links delivered: `.../candidates/links/{base,candidate}/inline_parser.ixx`; depends on shared label/entities; entity_end overlaps scalar, keep one implementation |
| `commonmark_fixture_tests` | Containers delivered: `.../candidates/containers/{base,candidate}/parser.ixx`; only list/quote methods; root/block-leaf agent owns dispatch |
| `commonmark_block_leaves` | Active: `.../candidates/block-leaves/{base,candidate}/parser.ixx`; leaf recognition, main dispatch and definitions; preserve raw characters, no trim-to-pass |
| `commonmark_inline_scalars` | Delivered `.../candidates/scalars/`: base/candidate inline_parser + inline_document, new `support/html_entities.ixx` + 2125-entry data + generator/license. Root must adapt generator paths and embed attribution before committing |
| `commonmark_runner` | Active render-scalar integration candidate `.../candidates/render-scalars/`; code span normalization must retain original TextSpan pieces/marker padding, not whole-span approximate mapping |

Candidate code has not been accepted merely because a Luna delivered it. Root
must compile, inspect actual source/range/incomplete behavior, run official and
old tests, then give narrow failure feedback to the owning agent. No production
parser candidates have been applied yet. Test harness is ready to commit;
previous Math handoff modifications predate this task and are separate.

## Fixed architecture and ownership

| Owner | Responsibility |
|---|---|
| `src/core/parsing/parser.ixx` | Block/container recognition, source-range accounting and link definitions |
| `src/core/parsing/inline_parser.ixx` | One block's lossless inline CST, delimiter/bracket matching and incomplete states |
| `src/core/model/inline_document.ixx` | Exact source and queries/projections over its CST; retain source coordinates |
| `src/core/parsing/serializer.ixx` | Serialize the existing block tree; inline source stays verbatim |
| `tests/parsing/test_commonmark.cpp` | Every official example is a named Boost.UT test with official expectations |
| `tests/support/commonmark_projection.hpp` | Explicit test-only AST/CST to semantic-tree projection; no Markdown recognizer |
| `tools/markdown-conformance/` | Verify/generate official fixtures, run formal tests, preserve failures and reports |

No production md4c dependency, second parser model, old/new path, HTML-based
editor, whole-document reparse on normal edits, replacement selection coordinate,
or reconstructed inline source. Preserve block-local source editing, NodeId
reconciliation, incomplete CST, existing WYSIWYG behavior, and undo/redo.
New helpers must own an independent invariant, not arbitrary pieces of a parser
state machine. A candidate must specify affected functions and actual cases;
do not change expectations, add skips, or special-case example source strings.

## Acceptance

- Build and run `cmd /c build_commonmark_test.bat`; reports retain every official
  ID, original Markdown/HTML, actual tree, first difference and source checks.
- After official failures, run `cmd /c build_md4c_reference_test.bat`. md4c is a
  reference implementation, never the definition of expected output.
- Syntax-family fixes must improve corresponding official cases without moving
  failures into a test projection/unsupported category or changing the tests to
  erase syntax distinctions. Deliberate HTML presentation policy differences
  remain explicitly separate.
- Run relevant existing parsing/editing cases, then the complete core suite
  with `cmd /c build_test.bat`. Maintain source/CST equality, exact selection and
  history, and zero full-document parse/serialization on ordinary edits.
- Stop a broad native evaluation after abnormal exit and diagnose a bounded
  focused run; an ordinary Boost.UT failure is expected during this repair.

## Confirmed examples and implementation references

- Official 352: `a*"foo"*` must stay literal; Folia creates emphasis. md4c
  `md_collect_marks`/`md_analyze_emph` separate flanking classification from
  delimiter-run resolution; Folia currently uses a greedy closer search.
- Official 518: `[foo [bar](/uri)](/uri)` must contain just the inner link;
  Folia constructs nested links. md4c `md_resolve_brackets` resolves inner
  links before invalidating enclosing ordinary-link candidates.
- Official 283: an ordered marker immediately followed by EOL (`2.`) is an
  empty list item. Folia requires marker-following space/tab and merges it into
  previous text; md4c `md_is_container_mark` accepts EOL.
- Official 270: the syntax tree matches, but saving adds two spaces to a blank
  list continuation line. This is a separate exact-serialization failure.

Official fixtures: <https://spec.commonmark.org/0.31.2/spec.json> (CC BY-SA 4.0).
md4c: <https://github.com/mity/md4c> (MIT). Reproduction commands and projection
limits are in `tools/markdown-conformance/README.md`.
