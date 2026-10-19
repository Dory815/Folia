# Gate213 full evaluation

All 61 targets compiled with at most four jobs in `build/core`. The complete
evaluation ended with failure; all 2,179 registered input entries remained
unchanged through its final audit. These are working-tree results, not an
isolated build of either intervening Tree commit.

| Observation | Actual result |
| --- | --- |
| Core | Exit `0xC0000005` after about 827 seconds; crash site unproven |
| Host / Foundation | Exit 1; remaining assertions require correction |
| Parser / Regexp / LinkedList | Exit 0 |
| Public API | 9/9 match |
| BBoldx text SVG | All six SVGs and glyph paths match; Scene snapshots present |
| Package captures | 24/25 completed; mhchem exceeded 60 seconds |
| Checkpoint comparisons | 15/20 pass; whole75, colortbl, mhchem, next-six and font packages fail |
| Common layout | Exceeded 120 seconds; no complete capture to compare |
| Full SVG / paired Scene | Each exceeded 300 seconds; no complete parity result |
| Request lifetime / owner retirement | Each exceeded 60 seconds |

Timeouts are incomplete observations, not proof of semantic differences. The
Gate212 SVG parity result does not establish Gate213 parity. Slot counts include
retired Context records; cumulative slot growth alone does not prove retained
native payloads.

Artifacts are under `build/math-port`: `serial-evaluation-g213.json`,
`eval-finish-g213.log`, `gate-213-post-evaluation-changes.json`, the five diagnostic
logs, capture/comparison manifests, output audit records and binary provenance.
The final shared state reports `evaluationExitCode: 1` and unlocks inputs only
after all evaluation children terminated and the unchanged-input audit passed.

Root's focused Tree and factory checks pass 543 assertions; the Context Tree
checks pass 53 assertions. The separately isolated mfenced group completes
without reproducing the Core crash and retains six failing assertions; see
[the isolation evidence](mfenced-focused-g213.md).

After this evaluation Root applied two Source-confirmed fixture corrections,
complete fallback property-copy key snapshots, assertion labels and mhchem
phase timing. These later edits are not Gate213 results. Maction's first passive
host candidate was rejected during review because actual receivers are Scene
handles, while its implementation accepted only Lite elements. The replacement
must use one owned callback store for both receiver types. Folia integration
remains pending.
