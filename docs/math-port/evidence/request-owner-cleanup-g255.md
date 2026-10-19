# Gate255: private Renderer Attributes and exact Map ownership

The nine-target build exited 0 with at most four compile jobs. All 2,208
registered inputs were unchanged through the final native observations; all
observed processes were terminal before releasing the gate. The final audit is
`build/math-port/final-evaluation-g255.json`.

The private Renderer captures newly bound Attributes and their three initial
native layers before Source replaces defaults. Cleanup retires canonical
Attributes, then only the exact captured native projections. Global defaults,
arbitrary nested values, Source-origin objects and replacement layers are not
enrolled. Captures retain unfinished work when retirement fails.

The Map registry now exposes exact object/storage retirement with payload
validation. A private MmlFactory tracks Maps created by its native nodeMap
projection and retires only its current projection. Replaced or Source-supplied
Maps remain ordinary live Maps. This path is independently tested, but it does
not explain the Renderer probe's remaining one-Map-per-request growth: that
Map belongs to the Common layout job and needs a separate change.

Full Host passed 462 tests and 3,269 assertions across 41 suites, including seven
new tests for captured layer identities, weak owner release, aliases, shared
defaults, getters, token validation, failed retirement and exact Map ownership.
The Core null-text mfenced selection passed 6 tests/36 assertions; the complete
mfenced selection passed 228 tests/1,008 assertions. These are filtered Core
results, not a whole-Core pass. ANSI escapes must be removed before parsing UT
summaries; earlier wrapper errors did not represent native test failures.

| Request-owner stage | Live Context records | Owner-bearing records | Canonical Attributes | MML projection entries | Live projection storage | Maps |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Warm | 4,297 | 2,056 | 0 | 9 | 7 | 18 |
| Persistent setup | 6,178 | 2,651 | 0 | 127 | 110 | 22 |
| After 192 requests | 105,826 | 33,819 | 0 | 7,871 | 6,830 | 214 |
| Later state checks | 108,153 | 34,566 | 0 | 8,009 | 6,947 | 220 |

The owner probe passed its existing MML/Lite/Scene, persistent macro/label,
retained result and cancellation checks in 120 seconds. Canonical Attributes
are now released. Broader Context retention remains unbounded; this result
does not establish bounded Renderer memory.

Evidence SHA-256:

| Artifact | SHA-256 |
| --- | --- |
| Host executable | `a732e42196bf87404c13e58c7633c71cb028a9e05fb9e9aabf19aa3ce86177b0` |
| Host stdout | `fa3690f58ea78ce3ab19b32932a9f0a95c1856caf659f62a5f632d5fdb6a10db` |
| Core executable | `6747c2c5be524bd0bc592f9d8e6895459c11a13562da5bb6ac4dce62f28fe401` |
| Complete mfenced stdout | `c0810c576ded24ea7c0573a4525c84d86b4e67fc32bc91953c40226ae81f23c2` |
| Owner probe executable | `d4588cad40a760633fb762dd00d1592b02e576754c2c04b805d361510d0c38fe` |
| Owner probe stdout | `f75751dd0c5a89dd3c5eb88e08c3210b78750a46fc3d18edeec82d92a35e01ad` |

No Folia runtime switch, complete current-profile acceptance, platform raster
acceptance or complete Source-member acceptance is claimed.
