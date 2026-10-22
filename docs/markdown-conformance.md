# Markdown syntax conformance

## Current checkpoint (2026-10-22)

User priority 2/3 is verified: the known Tab/lazy-continuation failures and all
21 exact-save failures are fixed. Commits e489a0b0 (column context/layout) and
69f7a062 (container syntax and edit history) contain the implementation.
The formal regression tests accompanying this checkpoint complete the batch.
No live agents or native builds. Build parallelism remains **4**.

| Check | Initial baseline | Current |
|---|---:|---:|
| Strict CommonMark 0.31.2 semantics | 292/652 | **586/652** |
| Inline source/CST losslessness | 652/652 | **652/652** |
| Character-exact document serialization | 571/652 | **652/652** |

All 652 official cases ran, with no omissions, skips or abnormal exit. Against
phase-five-start/ (9a4ff577), new semantic passes are #5/#6/#7/#9/#93/#238/#312;
all 21 former save differences pass. No previously passing semantic, save or
inline-CST assertion regressed. Expected official trees were not weakened.

Next: resume priority 1, the remaining **66 semantic differences** involving HTML
syntax, block boundaries, supported presentation and test projection. Eighteen
carry HTML policy/projection labels; nine carry other extension/projection limits;
39 have no such label. These are not 66 separately established parser bugs.
Inspect each official failure first, then consult pinned md4c implementation.
The earlier UI high-CPU investigation is paused; the pre-existing Math handoff
edits are separate and were not included in these commits.

## Accepted responsibilities

- `model/container_source.ixx`: prefix-only syntax records, attached optionally
  to existing quote/list-item payloads. Anchors identify an existing owner/source
  offset, a generated block Start/End, or a character offset in the existing
  separator field. They are not selection coordinates or cached body text.
- `parsing/parser.ixx`: capture physical prefixes while nested input maps exist;
  bind them to durable anchors. Per-character physical maps are parse-time only.
  Preserve lazy line context so #93's unmarked `===` stays literal paragraph text,
  and indented continuations in #238/#312 remain inside their open paragraph.
- `parsing/serializer.ixx`: resolve source and generated syntax anchors to their
  current output lines. Reuse existing spelling; generate valid prefixes for new
  lines. Prefix lookup is indexed, not a repeated scan of each full paragraph.
  Blank-line separators have explicit ownership, including #320's final quote
  blank and CR/CRLF tight-list sibling gaps.
- `editing/document_container_source.ixx`: local anchor translation, split/join
  transfer and pruning when nodes leave a container. Existing transactions record
  source edits followed by exact payload updates; Undo/Redo never recalculates
  those offsets. Fresh IDs are remapped in paste and local block recognition.
- `BlockItemSpecial.marker` owns the list marker once. An ItemOpening prefix
  record stores only its following syntax suffix, so checkbox/number edits cannot
  replay an obsolete marker. A quote's first prefix now uses the same per-line
  records as later prefixes; the earlier first-only `opening_marker` workaround
  is superseded.
- `parsing/block_source.ixx`: physical tab-stop columns and residual columns are
  distinct from raw source offsets. Partial tabs never become invented source
  characters. Raw code stores line context; generated indentation belongs to its
  derived projection. Source operations snapshot contexts for exact history.
- Render model/builder/layout consume that same derived code indentation with
  empty source spans and explicit affinity. Source keys include generated columns.
  The already-maintained final block index supplies post-transaction queries;
  removed intermediate owners do not cause another full-document index scan.

The established block tree, block-local authoritative inline source/CST, selection
and transaction model remain in place. Ordinary edits do not serialize or parse
the full document or compute a full-tree transaction diff.

## Verification and limits

- Core: **581 tests / 43936 assertions**, exit 0. Includes bounded randomized
  source edit/delete/newline history, mixed prefixes, split/join, list outdent,
  literal lazy-line edits, raw-code column-context history, task/ordered markers,
  CR/CRLF, generated indentation layout/hit testing, and source-key invalidation.
- Windows editor: **80 tests / 3429 assertions**, exit 0.
- Remaining official subset against md4c: **66/66 semantic matches**; 64 exact HTML
  matches and two formatting-only differences (#175/#309). The exact-byte
  reference assertions return normal test-failure code 1, not an abnormal exit.
- Official suite returns normal test-failure code 1 for the remaining 66 HTML
  semantic differences. The application installer was not built in this batch.
- Fixture/generator Python checks previously passed 10 tests; this batch did not
  change those tools. Fixture hashes were checked by the official/reference runs.

Ignored `build/markdown-conformance/` contains current `results.{json,jsonl,md}`,
`prefix-{core,official,platform,md4c}.log`, and `md4c/results.json`.
Immutable comparison reports: `baseline/`, `phase-three-start/` (d5dfc16b),
`phase-four-start/` (c5d8da4c), `phase-five-start/` (9a4ff577).
Candidate deliveries under `candidates/prefix-phase/` are historical inputs:
production includes root fixes that candidates must never overwrite.

## Settled HTML decisions

The shared `parsing/markdown_html.ixx` scanner recognizes complete tag syntax and
source ranges, without body copies, decoding or presentation policy. Block and
inline parsers consume it. Complete unknown/closing tags stay opaque Raw CST
nodes; attribute brackets/delimiters cannot become Markdown links/emphasis.
Type-7 HTML blocks use one existing editable Paragraph in HtmlText mode, ending
at a blank line/EOF and never interrupting an open paragraph. Its source excludes
only the terminal physical EOL, owned by the existing document/separator syntax.
Recognized HTML CST elements/comments project directly in formal tests.

Rejected history: initial type-7 v1 duplicated tag grammar and body text. v2's
extra HTML containers/fragments lost outer anchor/style rendering because the
container renderer walks only children. Accepted v3 keeps existing HtmlText CST
semantics, with real Editor/render regression coverage. Do not restore v1/v2 or
add a new serializer fragment branch. See Git through 9a4ff577 for prior batches.

## Reproduction and provenance

Run `build_commonmark_test.bat`; inspect official failures, then run
`build_md4c_reference_test.bat`. Relevant regression commands are `build_test.bat`
and `build_platform_test.bat`. Compare individual semantic/source/CST pass sets,
not just totals. Stop broad native runs after an abnormal process exit and
use a bounded focused test to diagnose it. Console tests link the repository's
console runtime so CRT reports stay in stderr.

[CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/) (CC BY-SA 4.0);
[md4c](https://github.com/mity/md4c), pinned to
f9e5eb232b95b737373ae706648cb75bd259ed1a (MIT). Fixtures, Unicode tables, licenses
and hashes are recorded in `tools/markdown-conformance/sources.lock.json` and its
README. No production md4c dependency was introduced.
