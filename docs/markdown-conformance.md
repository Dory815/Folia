# Markdown syntax conformance

## Current checkpoint (2026-10-22)

The user authorized syntax corrections using md4c as a reference **after**
running the official CommonMark examples. This is a syntax implementation
task, not an editor architecture migration. The previous Folia UI high-CPU
investigation is paused at the user's request.

Formal harness committed as `64afdc25`. Immutable initial results in ignored
`build/markdown-conformance/baseline/`: 292/652 strict semantic matches,
652/652 inline source/CST checks, 571/652 exact serializations. Its 360 failed
semantic cases were checked against pinned md4c: 360 semantic matches, including
29 differences only in raw HTML formatting newlines. No examples were skipped.

First integrated repair batch completed all 652 cases in 0.575 seconds:
506 semantic matches, 652 inline source/CST checks, 558 exact serializations.
Saved in ignored `build/markdown-conformance/phase-one/`. Remaining differences
are explicit failures; diagnostic relaxed comparison never passes a formal test.
The initial #230 quote nonprogress bug was fixed and verified by a bounded
focused run before repeating all examples. The dispatcher and quote consumer
now agree on indentation and consume a nonzero source range.

Current working tree includes the second syntax batch: label/destination fixes,
dialect-gated wiki links and math interruption, reference definitions, code-info
entities, literal terminal backslashes, original thematic markers, and retained
Markdown image InlineDocument on atomic block promotion. New shared utilities
provide immutable Unicode P/S + Zs + full casefold and named character references;
numeric references use Unicode scalars directly (CommonMark 2.5), with U+FFFD for
zero/invalid scalars. Reference labels fold raw spellings; href/title decode once.
WinUI project explicitly includes the two added modules.

Existing core checks exposed two regressions which were corrected: ATX trailing
whitespace stays in InlineDocument.source; rendering keeps the prior terminal
line-break behavior for editing. Code-span normalization retains exact source
spans, including CRLF and trimmed padding. The HTML-shaped test projection alone
omits the paragraph-terminating physical line ending. List continuation tab
expansion was rejected because it rewrote raw inline/code source. The speculative
`emphasis-editor` candidate was rejected: no confirmed failing CST contract.

Verified commits: `a6c11c20` shared character/label utilities and `6c36adc5`
inline syntax + code-span source projection. Block/serializer batch follows.
Final repair-batch check: **519/652 strict semantic**, **652/652 inline lossless**,
**592/652 exact serialization**; 133 strict differences and 60 save differences
remain explicit failures (174 combined failed cases). All 652 ran in 0.603s.
No strict semantic or CST regression against the original baseline. The 13 newly
failing exact saves are #7/#93/#230/#232/#233/#241/#247/#250/#273/#274/#278/#311/#313;
correct block recognition exposes their existing structural prefix normalization.
They are not inline CST data loss and must remain visible until fixed.

Core and deterministic Windows editor suites both pass (exit 0). The structure
property test passes 8074 assertions, including zero full-document parse,
serialization and transaction-diff work on edits. The mixed-bullet rendering test
now expects three lists, per CommonMark, and retains glyph/display assertions.
md4c repeated all 133 remaining official differences: 133 semantic matches;
111 exact HTML matches, 22 formatting-newline differences (normal exit 1).
Logs: `core-tests.log`, `platform-tests.log`, `syntax-build-test.log`,
`md4c-build-test.log` under ignored `build/markdown-conformance/`.

No live build/agent handles. Next narrow syntax work: quote/list lazy continuation
(#93/#238/#292/#293/#312), reference definitions (#194/#201), and raw-preserving
code tab/indent projection (#5/#6/#117). Of 133 strict differences, 39 are only
list-tightness projection, 29 already carry HTML policy/projection labels,
20 carry other limits (some also actual syntax), 45 have no current label.
Do not call all 133 independently established parser bugs or mask tests.

Candidates remain ignored; speculative `emphasis-editor` and source-normalizing
list indentation changes must not be applied. Previous Math handoff changes
are separate and remain uncommitted.

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
