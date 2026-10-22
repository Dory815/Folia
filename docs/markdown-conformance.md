# Markdown syntax conformance

## Current checkpoint (2026-10-22)

This is a Markdown syntax repair, using official CommonMark expectations before
consulting md4c. Keep Folia's block tree, lossless inline CST, source positions
and WYSIWYG editing. The earlier UI high-CPU investigation remains paused;
its existing Math handoff edits are separate.

List separators and quote terminal-line fixes are committed in 72fe9247.
List tightness is committed in e36af31e (core and all official cases verified).
Whitespace semantics is committed in 7a804d73. The accompanying commit fixes
#60 thematic-break precedence in the list sibling loop (c5d8da4c).
Blank-line source and quote opening-marker fixes are committed in e94f5904.
Shared markdown_html scanner + inline adapters accompany this checkpoint;
core and all official cases passed regression comparison. No live build.
The app project registers the new module. Windows tests passed before the final
raw-tag delimiter-collection fix; rerun after the block/projection batch.
Focused #536 showed complete unknown tags were scanned correctly but their
attribute brackets were still parsed as links in the main inline loop. Root
now emits existing Raw CST nodes for complete tags without derived presentation;
their text/render policy is retained, their internals stay opaque Markdown syntax.
Next integrate html-type-seven/candidate-v2 and
html-semantic-projection (test-only projection of already parsed inline HTML).
Both candidates are delivered, no live agents.
Root reviews/merges/builds; no agent builds/Git.
The initial html-type-seven candidate is rejected: duplicate tag grammar,
all tags made unsupported, and duplicate raw body storage. Only v2 may be used.

Frozen HTML responsibilities: parsing/markdown_html.ixx scans complete open/close
tag syntax to source ranges, without source copies/decoding/presentation policy.
Block/inline parsers consume that single scanner. html_cst retains DOM/editing
structure duties; complete supported HTML keeps existing semantic mapping.
Type-7 starts only at a block boundary, never interrupts a paragraph, and ends
at a blank line/EOF. Tests retain all strict failures from rendering policy.
Blank-line source stays in InlineDocument; existing semantic whitespace ranges
hide only literal spaces/tabs, retaining editable source spans.
Quote opening_marker preserves only the first physical prefix, not a general
multiline provenance implementation. Full prefix/history and Tab contracts
remain pending before lazy continuations #93/#238/#312 can be completed.
Scalar diagnosis found #21/#344 are HTML boundaries too; notes are under
candidates/remaining-scalars/.
Emphasis #475–477 are inline HTML behavior/projection differences, not delimiter
resolution errors; six-line notes are under candidates/remaining-emphasis/.
Candidates remain under ignored build/markdown-conformance/candidates/.
No candidate delivery counts as acceptance. Root owns
integration/builds; maximum parallelism is **4**. Earlier verified syntax
batches are recorded in Git through d5dfc16b.

| Check | Initial baseline | Current |
|---|---:|---:|
| Strict official semantic matches | 292/652 | **573/652** |
| Inline source/CST losslessness | 652/652 | **652/652** |
| Character-exact document serialization | 571/652 | **631/652** |

All 652 cases ran in **0.612s**, with no skips or abnormal exit. There remain
**79 semantic differences**, **21 exact-save differences**, and **97 combined
failed cases**. No semantic, save or inline-CST regression against d5dfc16b.
Source checks and correct syntax are independent assertions.

## Verification

- Core: **527 tests / 41935 assertions**, exit 0. The 8074-assert structure
  property test covers Enter/merge/undo/redo and zero full-document parsing,
  serialization or transaction-diff work during normal edits.
- Windows editor: **80 tests / 3429 assertions**, exit 0.
- md4c on the remaining 79 official semantic differences: **79 semantic matches**;
  74 exact HTML matches, 5 formatting-only newline differences. Its native
  exact-byte assertions return the normal test-failure code 1.
- Fixture/generator Python tests: 10 passed; fixture inputs are hash-verified.

Reports/logs are in ignored build/markdown-conformance/: results.{json,jsonl,md},
blank-prefix-{core,official,platform}.log and md4c/results.json (79 unchanged
semantic failures; reference run from the preceding batch).
Initial reports remain in baseline/; phase-three-start/ is the d5dfc16b baseline.
Earlier phase-one/, phase-two/ and intermediate batch logs are historical.
Generated fixtures and downloaded third-party sources remain ignored.

## Remaining work and exact next action

Of the 79 strict differences, none is list-tightness-only; 29 carry HTML
policy/projection labels; 9 carry other limits; 41 have no such label.
Several unlabeled cases also concern HTML boundaries (diagnoses above).
They are not 79 independently established parser bugs. Diagnostic relaxed
comparison never passes a formal test; expectations were not changed or skipped.

Before implementing container-prefix provenance, finish its edit/history
contract. Read-only Sol 6.1 prefix_history_review established that owner/source
prefix anchors also need
generated/blank-line anchors, split/join transfers, fresh-ID remapping and
recorded payload updates after TextOps (never double-translate during replay).
Absent lazy prefixes must be invalidated when an edit would change structure
on reload. Freeze this contract before implementing prefix provenance.

Keep provenance and partial-Tab visual columns in existing parser/serializer/
projection boundaries; do not rewrite raw source to
improve rendered output. For #93, accepting a lazy === line while saving adds
the missing quote prefix would turn a paragraph into a heading on reload;
that incomplete change was rejected. Ordinary nested continuations #292/#293
now preserve save/reload semantics. #117 was a duplicate test-projection newline,
not a production parser defect; fenced empty/trailing blank lines remain tested.

The 21 exact-save failures are #4/#5/#9/#93/#229/#230/#232/#233/#240/
#247/#250/#251/#259/#260/#278/#279/#290/#291/#292/#293/#320.
They concern container prefixes/indentation and blank-line spelling.
Overpadded list markers and continuation extraction
no longer invent spaces in child source from Tabs; unresolved column semantics
remain reported. Candidate originals stay under ignored
build/markdown-conformance/candidates/. Never reapply entire old candidates:
emphasis-editor, generic code-ending removal and Tab-to-space source rewriting
were rejected. Candidate delivery alone was never acceptance.

## Fixed responsibilities

- parsing/parser.ixx: block/container recognition, physical source ranges, definitions.
- parsing/inline_parser.ixx: one block's lossless CST, matching and incomplete states.
- model/inline_document.ixx: exact source and CST queries, retaining source coordinates.
- parsing/block_source.ixx: lossless code source and semantic content/source mapping.
- parsing/serializer.ixx: existing block tree to Markdown, inline source verbatim.
- support/utf.ixx, support/html_entities.ixx, parsing/link_label.ixx: immutable
  Unicode/reference utilities and raw-label identity.
- tests/parsing/test_commonmark.cpp and tests/support/commonmark_projection.hpp:
  named official tests and explicit AST/CST semantic projection.
- tools/markdown-conformance/: verified fixtures, reference runs and reports.

No production md4c parser dependency, second document model, compatibility path,
HTML-based editor, replacement selection coordinate or normal-edit global reparse.
Reference labels fold raw spellings (#545); href/title decode once. Markdown
ImageBlock retains its parsed InlineDocument and serializes that source while
keeping atomic interaction. Code-span normalization retains original ranges,
including CRLF and hidden padding. Existing terminal-line-break editing layout
stays unchanged. Mixed bullet glyphs correctly create separate lists; the
existing rendering test retains every original glyph/display assertion.
List tightness belongs to BlockListSpecial; derived shells retain it. Whitespace
display ranges belong to inline_document; render_builder keeps those characters
as editable source-spanned markers. No source/CST normalization was added.

## Reproduction and provenance

Run build_commonmark_test.bat, inspect official failures, then
build_md4c_reference_test.bat. Run build_test.bat and build_platform_test.bat
after relevant changes. Compare semantic and source regression sets, not only
pass totals. Stop broad native evaluation after abnormal exit and diagnose a
bounded focused run.

[CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/) (CC BY-SA 4.0);
[md4c](https://github.com/mity/md4c), pinned to
f9e5eb232b95b737373ae706648cb75bd259ed1a (MIT). Unicode 15.1 tables and named
references include licenses and generator provenance. Hashes and reproduction
details are in tools/markdown-conformance/sources.lock.json and its README.
