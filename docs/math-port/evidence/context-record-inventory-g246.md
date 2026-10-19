# Context record inventory: gates 246–247

The diagnostic inventory reads stored Context record members without invoking
getters, native views, or owner-trace callbacks. It distinguishes allocated
slots, live records, retired records, and live records carrying native owner
fields. These counts do not measure heap bytes or prove reachability.

Gate 247 built all nine requested targets with at most four compile jobs.
The complete Host suite exited 0: 455 tests and 3,185 assertions across 41
suites, including inventory creation/retirement and zero-getter assertions.
All 2,204 registered inputs remained unchanged through that run.

Gate 246's owner-retirement probe exited 0 after about 92 seconds. Its existing
MML, parser, wrapper, Scene and Lite checks passed, including persistent macro
and label/reference behavior. The new inventory nevertheless exposes retained
Context records:

| Stage | Slots | Live | Retired | Native owner-bearing |
| --- | ---: | ---: | ---: | ---: |
| Warm renderer | 4,358 | 4,303 | 55 | 2,062 |
| Persistent-state setup | 6,862 | 6,272 | 590 | 2,745 |
| After 64 iterations, three requests each | 142,926 | 111,808 | 31,118 | 39,801 |
| After later persistent-state checks | 145,996 | 114,241 | 31,755 | 40,654 |

This is not just retained slot numbering after retirement. General Renderer
lifetime acceptance remains open despite the probe's successful exit. Do not
switch Folia based on its narrower owner counters. Canonical Attributes, Map
storage and property-projection ownership require coordinated investigation;
blindly retiring a Context allocation suffix would leave native registries or
durable Source state pointing at retired records.

Gate 246's full Host run also reported eight assertions in two new formatter
tests. Their helper omitted Source's inferred row under `merror`; correcting
the helper to the independently checked Source tree resolved those assertions
at gate 247. It did not change the formatter's expected text or identities.

Execution artifacts are under `build/math-port/host-g247.txt.*` and
`build/math-port/owner-retirement-g246.txt.*`. Host executable SHA-256:
`e361abb986f647ecc892ca69d1c6e6f86badac18a03398e785c6ff7671326132`.
Owner probe executable SHA-256:
`87277f435807bab077772b3cc9a6694de75f58c4538aa2d3f216c55cd7ae778c`.
The owner observation's input and binary hashes were unchanged before/after.
