# Gates188–189: request-owner retirement exposes duplicate classification

The private Renderer now explicitly marks and retires request-owned MML
products, parser/stack/item identities, Context records, Lite records and
projections, and Scene handles/source identities/source views after copying
the result. Persistent definitions, tags, fonts and module state remain
outside this cleanup. Layout-job retirement still precedes dependent owner
retirement. Context slots remain tombstones and are not claimed bounded.

The increment includes three ownership primitive UT files, a Scene/frozen
result ownership UT, and a real Renderer diagnostic with 64 mixed array,
fraction and Scene iterations, macro/tag/reference continuity, retained
results, and cancellation. Cancellation input is prepared as UTF-16 before
the stop thread. Named per-owner assertions are retained. The Source76
dispatch fixture is separately corrected after an exact emitted-CJS receiver
and operation witness; all 77 direct Stack observations pass in Gate189.

Gate188 fails compilation only at the new Context record UT's unqualified
NativeDataField name. Root qualifies options::NativeDataField and repeats
the build. Gate188 closes with 2,108 unchanged inputs and no runtime claim.

Gate189 compiles all 60 targets and completes evaluation with 2,108 inputs
unchanged. Manifest SHA-256:
`7af30cedd2bd8b2659e4f7f099e387e03bd1be17ef967e269917846e837eaa06`.

| Check | Actual result |
| --- | --- |
| Complete public SVG/error comparison | 2/150 match; fails |
| API comparison | Fails; comparer produces no accepted report |
| Common layout observations | 75/75 match |
| NativeRenderSession route audit | 146 equal successful Scenes plus four equal error pairs |
| Package captures / checkpoint comparisons | 25/25 complete and 17/20 pass |
| Parser / Regexp / LinkedList | All pass |
| Core / Host / Foundation | All fail, including Renderer-based tests |
| Three Renderer lifetime diagnostics | All exit 1 with retired Context record errors |

The ownership primitive tests and focused Scene test pass. Existing Renderer
tests fail during their first conversions. A phase diagnostic compiled
against the stable Gate189 library reports:
`primary/warm-render: TypeError: Object handle refers to a retired Context record`.
This is not a successful cleanup or bounded-owner result. The passing Common
layout and paired NativeRenderSession routes bypass private Renderer cleanup,
so they do not refute the public regression.

Source inspection identifies duplicate ownership classification:
ParseOptions::getList stores a bindChildren canonical array in nodeLists;
TeX::releaseRequestReferences manually retires every nodeLists ObjectRef;
outer Renderer cleanup then retires captured nodeArrayIdentities, including
the same already-retired record. The candidate correction asks the actual
typed registry whether it owns the handle and leaves that record's retirement
to its registry. Record retirement remains non-idempotent, preserving stale
handle and ownership checks. Its primitive test now checks canonical handle
membership before and after retirement.

The classification correction and phase-reporting diagnostic are applied
after Gate189 closes, for Gate190. Gate190 is building at this record's
creation and has no accepted runtime result. The artifacts under
`build/math-port/` preserve all three stderr/provenance files, the complete
SVG report, the failed comparison process records, and
`retirement-phase-g189-run.log`. No Source capture or expected rendering was
modified. Folia integration remains deferred; Mermaid remains paused.
