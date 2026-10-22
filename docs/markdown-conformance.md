# Markdown syntax conformance

## Current checkpoint (2026-10-22)

This is a Markdown syntax repair, using official CommonMark expectations before
consulting md4c. Keep Folia's block tree, lossless inline CST, source positions
and WYSIWYG editing. The earlier UI high-CPU investigation remains paused;
its existing Math handoff edits are separate.

List separators and quote terminal-line fixes are verified and accompany this
checkpoint. No live native build. The former render test expected an inter-item separator
to belong to a synthetic empty paragraph; it now checks a generated boundary
after the code row, retaining all code identity/content/indent assertions.
Candidates remain under ignored build/markdown-conformance/candidates/.
Luna list-tightness and whitespace-semantics candidates await integration;
no candidate delivery counts as acceptance. Root owns
integration/builds; maximum parallelism is **4**. Accepted batches:
a6c11c20 (character/label utilities), 6c36adc5 (inline syntax), 11b85899
(block recognition/source serialization), plus the followup accompanying this
checkpoint: nested continuations, escapes, code endings and literal Tab source.

| Check | Initial baseline | Current |
|---|---:|---:|
| Strict official semantic matches | 292/652 | **522/652** |
| Inline source/CST losslessness | 652/652 | **652/652** |
| Character-exact document serialization | 571/652 | **628/652** |

All 652 cases ran in **0.609s**, with no skips or abnormal exit. There remain
**130 semantic differences**, **24 exact-save differences**, and **145 combined
failed cases**. No semantic, save or inline-CST regression against d5dfc16b.
Source checks and correct syntax are independent assertions.

## Verification

- Core: **499 tests / 41556 assertions**, exit 0. The 8074-assert structure
  property test covers Enter/merge/undo/redo and zero full-document parsing,
  serialization or transaction-diff work during normal edits.
- Windows editor: **80 tests / 3429 assertions**, exit 0.
- md4c on the 130 remaining official differences: **130 semantic matches**;
  109 exact HTML matches, 21 formatting-only newline differences. Its native
  exact-byte assertions return the normal test-failure code 1.
- Fixture/generator Python tests: 10 passed; fixture inputs are hash-verified.

Reports/logs are in ignored build/markdown-conformance/: results.json,
results.jsonl, results.md, core-tests.log, platform-tests.log,
syntax-build-test.log, md4c-build-test.log and md4c/results.json.
Initial reports remain in baseline/; phase-one/ and phase-two/ are historical.
Generated fixtures and downloaded third-party sources remain ignored.

## Remaining work and exact next action

Of the 130 strict differences, 38 match only after diagnostic list-paragraph
unwrapping; 29 carry HTML policy/projection labels; 20 carry other limits
(some also have actual syntax differences); 43 currently have no such label.
They are not 130 independently established parser bugs. Diagnostic relaxed
comparison never passes a formal test; expectations were not changed or skipped.

Immediate next action: review list-tightness candidate (AST/parser/projection, list shell copies)
and whitespace-semantics candidate (existing inline_document source ranges,
render_builder editable markers, strict projection), merge by saved bases.
The read-only Sol 6.1
prefix_history_review is finished: owner/source prefix anchors also need
generated/blank-line anchors, split/join transfers, fresh-ID remapping and
recorded payload updates after TextOps (never double-translate during replay).
Absent lazy prefixes must be invalidated when an edit would change structure
on reload. Freeze this contract before implementing prefix provenance.

Following that: preserve container prefix provenance and partial-Tab visual columns in the
existing parser/serializer/projection boundaries, then finish lazy continuations
#93/#238/#312 and leading whitespace semantics. Do not rewrite raw source to
improve rendered output. For #93, accepting a lazy === line while saving adds
the missing quote prefix would turn a paragraph into a heading on reload;
that incomplete change was rejected. Ordinary nested continuations #292/#293
now preserve save/reload semantics. #117 was a duplicate test-projection newline,
not a production parser defect; fenced empty/trailing blank lines remain tested.

Structural-prefix normalization remains an explicit exact-save failure; the
list separator batch fixed #273/#274/#311/#313 and related gaps without
rewriting source. Overpadded list markers and continuation extraction
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
