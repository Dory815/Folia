# Gates191–192: property/argument boundaries and exact owner families

Gate191 adds live Stack environment reads/writes and a readonly snapshot of
the 24 actual ParserRuntime identity families. It fails compilation on a
missing propertyValue forward declaration. Root adds the declaration;
Gate191 closes with 2,110 unchanged inputs and no runtime claim.

Gate192 additionally integrates one authoritative string/Symbol-key
StackProperty storage, weak Context read/write delegates, and live
Object.assign descriptor/Get/Set ordering. The direct Factory callability
increment prevents eager children conversion for invalid kinds. All 60
targets compile, with 2,114 unchanged inputs through complete evaluation.
Manifest SHA-256:
`0c53f8381d83efea11a87e9d18765dd675c9185888748682576ce2ed31136ce8`.

| Check | Actual result |
| --- | --- |
| Complete public SVG/errors | 150/150 match |
| API / Common layout | 9/9 and 75/75 match |
| Paired Scene routes | 146 equal successful Scenes and four equal error pairs; Scene-only materialize/XML calls zero |
| Captures / checkpoint comparisons | 25/25 complete and 18/20 pass |
| Foundation / Regexp / LinkedList | Pass |
| Core | Four failing tests across three suites; two are raw Stack-field tests |
| Host | Argument builder no longer evaluated for non-callable selection |
| Parser | Repeated closed-item constructor declares the dormant environment field twice |
| Owner-count driver | Exits 0 but still exposes growing registry |
| Strong mixed owner driver | Fails registry and Lite identity assertions |
| Earlier request-lifetime driver | Harness terminates it at 60 seconds; no accepted result |

The earlier request-lifetime driver previously used 180 seconds and took
about 100 seconds in Gate190. Root accidentally omitted that explicit
timeout for Gate192. The resulting actual null exitCode/SIGTERM is preserved
as failure. The closure helper now recognizes an ended signal termination
as terminal without converting it into success. There is no automatic retry.
The next changed build must restore the existing 180-second allowance.

The new live-environment test passes its 16 assertions. The Symbol test
passes 54 assertions. The new MML bad-kind tests pass seven assertions.
Independent Source witnesses remain hash-locked. The raw Stack fixture also
contains an incorrect new call to an environment accessor's returned object;
it needs a direct value comparison, not a production behavior change.
The generic Factory builder and MML conversion adapter need separate raw
argument and native conversion phases within one invocation algorithm.

Readonly diagnostics compiled against Gate192's actual IFCs/library identify
the owner family, without copied libraries or alternate render sessions:

- TagInfo identities: 4 constructed, 5 warm, 6/7 after two more plain calls,
  71 after 66 more. All other 23 families stay unchanged.
- Array/fraction/Scene mixed operations keep Lite records 11 and identities
  one through all 192 calls. The registry grows once per conversion.
- The exact setup/error/reference sequence first grows Lite identities to
  two on malformed `\\frac{`, with records still 11.
- Label identities grow from zero to two on label A, then four on label B.
  Those durable definitions must survive; aggregate equality across the
  second label insertion would misclassify required state as a request leak.

Logs and executable/input provenance remain under `build/math-port/`:
`parser-registry-breakdown-g192.log`,
`renderer-owner-step-breakdown-g192.log`, and
`renderer-owner-full-sequence-g192.log`. The next increment must retire only
superseded TagInfo identities while retaining current/history/stack roots,
and identify the exact cached Lite node before changing ownership rules.
Context slots remain cumulative; no whole-memory boundedness is claimed.

The full Tree/MML packet remains candidate-only. Independent review has
confirmed live descriptor/method dispatch, ordinary-receiver construction,
foreign children/factory/attributes, and arbitrary copy-result differences.
Public profile parity does not accept that packet or all upstream features.
Folia still uses its math QuickJS worker; Mermaid remains paused.
