# Gate193: stable TagInfo owners and remaining constructor/Label work

All 60 targets compile. All 2,119 captured inputs remain unchanged through
runtime evaluation. Input manifest SHA-256:
`9311938981fb0abcbca2c9a1f3dfe649ab4cf0bb6c657c57deccfdafe6d472da`.

| Check | Actual result |
| --- | --- |
| Complete public SVG/errors | 150/150 match |
| API / Common layout | 9/9 and 75/75 match |
| Paired Scene routes | 146 equal successful Scenes and four equal error pairs; Scene-only materialize/XML calls zero |
| Captures / checkpoint comparisons | 25/25 complete; 18/20 pass; next-six and font-package comparisons still fail |
| Host / Foundation / Regexp / LinkedList | Pass |
| Core | Three failing tests: repeated constructor descriptor handling and two incorrect fixture module lookups |
| Parser | One failing repeated NullItem constructor test |
| Earlier owner-count / request-lifetime drivers | Both exit 0; lifetime allowance restored to 180 seconds |
| Strong mixed owner driver | Exits 1 on aggregate parser and Lite identity assertions |

The shared Factory argument sequence restores Host raw-argument evaluation
while preserving Foundation's deferred native-child conversion. Private
Renderer TagInfo retirement now traces actual currentTag/history/stack raw
roots without invoking getters. Four focused tests (25 assertions) pass.
The 24-family diagnostic shows every family stable through 66 subsequent
plain renders; TagInfo remains four, rather than growing once per render.

The remaining constructor error is a missing guard around dormant `_env`
native-data declaration when a closed instance is initialized again. The
lifecycle flag was captured before resetting Source access, but the final
conditional was omitted from the integrated increment. Two separate Core
fixtures fetch NullItem/PrimeItem from StackItem; pinned CJS exports both
from BaseItems. These are fixture corrections, not new production exports.
The guard and fixture corrections are applied after Gate193 and are unbuilt.

The exact mixed sequence records parser counts 116 warm, 119 after label A,
118 through 64 array/fraction/Scene iterations, and 121 after label B with a
reference to A. Labels are two after A and four after B. Source HandleLabel
creates a placeholder which makeTag replaces; an unresolved HandleRef also
creates a Label absent from persistent dictionaries. Therefore Gate192's
description of both additions per label as durable was too broad. A separate
private Renderer Label-root retirement candidate is under independent review.
Normalizing with the same first loop formula is required before comparing
TagInfo histories; the active tag itself can legitimately change those roots.

Lite records remain 11 throughout the mixed sequence. Canonical identities
are one until malformed input, then remain two on repeated errors/plain x.
The readonly identity diagnostic reports document `8:3` plus element `10:0`.
Source construction order identifies the latter as the existing session body,
not a new request record. A named raw-record diagnostic is added for the next
build to verify this directly without hard-coding allocation order in tests.

Logs and executable/input provenance are under `build/math-port/`:
`parser-registry-breakdown-g193.log`, `renderer-owner-full-sequence-g193.log`,
`lite-error-identity-g193.log`, and the individual test/driver logs. Public SVG
comparison SHA-256 is
`71051207c6d0c4b12844049eff4933cc8a5f62549304eff1527f2708154deadc`;
API comparison SHA-256 is
`22ce4ade41daf0b24b65c4454b6c34a149e0cfb19abcfc328a2d71fa7b75d6ad`.

Context slots remain cumulative; stable owner registries do not establish a
whole-renderer memory bound. The shared MML algorithm packet, String.split
semantics, and six Stack methods remain unaccepted candidates. The MML
architecture is fixed by `mml-live-receiver-contract.md`; native and Context
entry points must consume the same algorithms. Profile parity does not prove
all upstream features. Folia still uses math QuickJS; Mermaid remains paused.
