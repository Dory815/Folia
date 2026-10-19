# Shared gate 262: null indexing, Source parity, and Lite array lifetime

The 13-target build exited 0 with at most four compile jobs. All 2,211
registered inputs remained unchanged throughout evaluation.

## MML null indexing

The gate 261 Core exception dump recorded a null read at module RVA
`0x421f781`. A separate map-only relink had identical `.text` bytes and
layout to that executable. Static symbols, the exception CONTEXT, PE unwind
metadata, and the captured stack identify `native_mml_receiver`'s indexed
child access during `math_port_replace_child_source`. The active `Children`
variant contained an empty shared pointer. The nearest public symbol alone
was too distant to establish this attribution.

Native MML property/index access now throws the Source TypeError before
dereferencing a null child array. A pinned Source witness confirmed the exact
`Cannot read properties of null (reading '0')` message for `setChildren`,
`appendChild`, and `replaceChild` on `mstyle`. The corresponding new regression
passed six assertions. The original `replace_child` corpus passed all 1,504
assertions. Constructor setter-dispatch tests passed 42 assertions in 11 tests.

## Finite evaluation results

- Host: 471 tests, 3,393 assertions passed.
- Output Foundation: 116 tests, 1,334 assertions passed.
- Begingroup: four tests, 43 assertions passed after canonical factory setup.
- Factory kind/argument order: three tests, seven assertions passed.
- Common layout: 75/75 Source comparisons passed.
- Full SVG: 150/150 exact byte comparisons passed, including continuous and
  fresh-session execution of the 75-case profile.
- Paired Scene geometry: 150/150 passed; Scene-only materialization and XML
  serialization counters were both zero.
- Listener positions: four Source/native pairs passed.
- Font tables, three extension rows, and the font provider's 40 modules,
  80 requests, and 24,646 glyphs passed their existing comparisons.

The profile acceptance artifact is in ignored
`build/math-port/g262-current-acceptance/acceptance.json` and reports accepted.
Source remains MathJax 4.1.3 commit
`fb987178c2d279a99b0db5ea02e751691703955e`.

## Lifetime measurement and remaining work

Lite cleanup now retires the exact initial array projections owned by
request-created nodes. It preserves old nodes first projected during the
request, Source replacement arrays, and retained immutable Scene data. Five
tests cover those boundaries, cleanup progress, and request-mark validity.

In the unchanged 192-render workload, post-setup to post-loop owner-bearing
growth decreased from 30,976 to 28,416: exactly 2,560 fewer Lite array views.
The total Context allocation-slot increase remains 136,064, and MML projection
counts are unchanged. This is a measured family improvement, not bounded
session-lifetime acceptance.

Full Core completed normally with exit 1, rather than a native exception. It
still reports 443 failed assertions across three tests: raw TextNode
`toString` (one), live ACCENTS slots (two), and token-walk virtual dispatch
(440). The separate Boldsymbol/Configmacros fixture still reports six compile
exceptions and needs canonical MathItem identity setup. These are not passing
scopes. Folia integration and replacement of its math QuickJS path remain open.
