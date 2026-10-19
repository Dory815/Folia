# Common layout and font observations: gates 247–251

## Common layout with the fixed passive event host

Gate247's Common observer exited 0 with unchanged binary/input hashes. The
old strict comparator rejected two rows: its Source action records stopped at
`node.addEventListener is not a function`, while native completed them. The
old 73/75 rejection remains valid for that host-less Source capture.

Root independently reran the Common-instrumented MathJax 4.1.3 observer with
the exact pinned G213 register-only event host. No geometry, call trace, error
or field filtering was added. All 75 fresh Source records exactly match the
saved native records, including full BBoxes, wrappers, placement calls, SVG,
MML and warnings. The other 73 Source records deeply equal the old capture.
The configured and actually registered native package lists also match.

Fresh Source capture SHA-256:
`c1152de00a714c78adcfc4d45f141585c06a9eef07609c83a7e604e852342edf`.
Native Common capture SHA-256:
`4de6db9b34bd0be6d593fc66e2f473e462125c6d53d7e3d369773de0120495c2`.
Native executable SHA-256:
`4c54d5b28d8380352c67f8f5888a2bb0843329450811158f51c9d41f2aa0e498`.
The independently reviewed observer SHA-256 is
`733051d6f1a2b45f150ae572f43e874d126911b886a7b7677dffa166a9b9f5c0`.
Artifacts: `build/math-port/common-passive-source-root-g247.json`,
`common-passive-comparison-root-g247.json` and `g247-layout-font/`.

The new observation does not replace the old frozen capture or relax its
comparator. Promoting the new reproducible observer/reference into the formal
Common acceptance runner remains a follow-up.

## Full font tables

The earlier standalone font observer failed because its manually assembled
host lacked the session RegExp service and its direct font instances lacked
canonical Context identities. `dynamicFileName` requires both. The existing
Source load-error path caught the resulting exception, marked files failed,
and left dynamic placeholders, first exposed at `normal` code point 127.
The pinned Source row at that position is a real glyph; it must not be skipped
or serialized as though an unresolved placeholder matched the reference.

The observer now supplies the actual RegExp owner, constructs both font
instances through their registered Context constructor, and initializes their
Source identity. Load warnings and exact variant/codepoint/file/setup state
are reported when a failure occurs. No production font algorithm, generated
glyph data, expected table or comparator was changed.

Gate251's observer exited 0 with no stderr. The existing full-table comparator
accepted all four static sources plus every synchronous dynamic glyph, option
record and delimiter against pinned MathJax 4.1.3. The build and observation
kept all 2,204 registered inputs unchanged, using at most four compile jobs.
Native executable SHA-256:
`36e58c6ee355edce5f23965bc23e7c45975de7c6aa1ef518b981ccd3245464b0`.
Normalized capture SHA-256:
`4953e779320db95544c999e4068c5556f5d17b9457823f3d41a78f6c0c1bfc41`.
Artifacts: `build/math-port/font-full-g251.json.*`.

Font instance-extension/provider checks, complete current SVG/Scene tests,
general Context lifetime, full core/platform tests and Folia integration remain
open. These observations do not establish full renderer acceptance.
