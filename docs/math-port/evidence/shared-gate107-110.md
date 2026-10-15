# Shared native math checkpoints 107–110

Gate107 failed one ParseOptions implementation unit. Gate108 introduced the
complete Mhchem/Bussproofs compilation packets and additional observers, then
failed package API and observer type checks. Gate109 compiled the Mathtools
and TagFormat/SetOptions packets for the first time and exposed their remaining
API mismatches. All three terminal input audits reported zero changed inputs.
None supplies a new runtime acceptance result.

Gate110 builds and links 38 targets with 1,836 captured inputs unchanged.
Bussproofs, Mathtools and TagFormat/SetOptions are excluded as whole unfinished
packets while their owners continue corrections; production registration is
still 23/38. This is a passing compile checkpoint, not full profile acceptance.

Runtime results use copied executables under `build/math-port/gate110-runners`:

- Core passes 345,659 assertions in 5,325 tests across 171 suites, exit 0.
  Its recorded process runs from 07:15:02 to 07:18:02 UTC on October15.
- Host, Regexp and LinkedList pass. Parser still fails. Foundation still fails
  three typed Scene tests; its seven SceneContext tests pass 56 assertions.
- The full renderer emits all 75 records, with 64 successful layouts and
  11 errors; 47 complete records match Source. Actual registration is 23/38
  with 15 warnings. The earlier missing Array.unshift parse failure is gone.
- Public SVG/error comparison matches 8/9 records. The aligned case fails
  with an unbound native identity. The document probe exits 0 and confirms
  Context/graph/host release and detached Scene survival.
- Physics matches 49/63 cases and 8/11 maps. Colortbl emits all 29 records and
  its map matches; complete records fail, initially on observer key order.
  Next-six matches 15/20; font packages match 13/16.
- The official Mhchem dependency executable matches all 7,960 records:
  1,679 parse observations, 6,279 patterns, and full tables/regex descriptors.
- The captured Mhchem package output matches 219/220 cases, all 63
  replacements, six rules, two maps and 14 mutations. One deep inherited
  Unicode field differs. Eight diagnostic reruns of the identical executable
  and an instrumented isolated rerun are clean; the recorded original capture
  remains unaccepted and the differing observation must be rechecked.
- The Common getBBox lifetime witness passes. Its initial JSON observer exits
  0 but truncates numeric output and lacks the complete Source schema; that
  capture is not an exact Source pass. Revised isolated observers for getBBox
  and Array.unshift subsequently match their complete Source records against
  the G110 library, with additional native checks in a separate object.
- The original unshift/text-tip standalone observations do not finish and are
  stopped after approximately 106 seconds. Their recorded exit codes are
  4294967295. Revised observers catch exceptions; unshift's differing test
  argument is corrected, while full-profile text-tip reports the shared
  unbound native identity failure. Those revisions await shared recapture.

Detailed reports retain `g110` in their names under `build/math-port`.
Every copied observation records executable/input hashes, PID, UTC interval
and actual exit status. No new package is registered and Folia continues using
its existing math worker until the full native path passes acceptance.

The architectural review identified two per-session ownership gaps:
Bussproofs module-private state was process-global, and MapHandler's registry
was shared by independent runtimes. Luna owners are correcting both. Root's
registry design and required tests are in `../map-handler-session-plan.md`.
The ordinary Context callable identity bridge also needs a checked native
execution boundary; the focused Sol review recommends an identity-only handle
and explicit NodeFactory converters, while Luna owns implementation.
