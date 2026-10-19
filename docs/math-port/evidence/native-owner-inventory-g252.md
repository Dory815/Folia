# Gate252: native binding inventory

The eight-target build exited 0 with four compile jobs and 2,204 unchanged
registered inputs. No application switch or complete renderer acceptance is
claimed. The diagnostic accessors read native registry sizes and weak-owner
counts; they do not call Source getters or change retention.

Full Host passed: 455 tests, 3,185 assertions, 41 suites. Executable SHA-256:
`dba86ea5427eb248526aa56b43c71e036ad6731267ff0aa6e320e4a5a7156c08`.

The request-owner probe exited 0 after 156 seconds. Existing strict MML, Lite,
Scene, macro/label persistence and result-lifetime checks passed. Its broader
inventory demonstrates that other owners still accumulate:

| Stage | Live Context records | Owner-bearing records | Attributes | MML projection cache entries | Live projection storage | Parser Maps | Parser functions |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Warm | 4,303 | 2,062 | 3 | 12 | 11 | 18 | 373 |
| Persistent state setup | 6,272 | 2,745 | 47 | 170 | 158 | 22 | 373 |
| After 192 requests | 111,808 | 39,801 | 2,991 | 10,666 | 9,822 | 214 | 373 |
| Later state checks | 114,241 | 40,654 | 3,044 | 10,851 | 9,992 | 220 | 373 |

These are adapter-specific inventories, not a complete root set. A weak owner
being live does not prove reachability or permission to retire its record.
Attributes' three fresh layers and each private factory's projected nodeMap
have separate ownership reviews; general Context sweeping is not justified.

Probe executable SHA-256:
`59493c45496e97d626361e424ea70c21c629f9cd1bc87b945777b25738f56bba`.
Output SHA-256:
`d72e813bc33b6c05b45e48b1aff3c70e215573c6f5616d33d35e291b1095d9a5`.
Artifacts: `build/math-port/owner-retirement-g252.txt` and its provenance.

Font instance/extension comparison passed 3/3. The provider observer exited 1
while constructing its NewCM graph; it still directly constructs unregistered
font classes/instances and lacks the host setup used by the accepted full-font
observer. No provider parity is claimed. Broad evaluation stopped there.

Both Core mfenced filters exited 0 but executed zero matching assertions; their
logs do not even enumerate the expected mfenced test names. The object is linked
and its test names exist in the executable. These are inconclusive discovery
results, not passing regressions. Registration/discovery requires diagnosis
before a full Core run. No full Core or platform pass is claimed.

All actual processes were terminal and all registered inputs were rechecked
before releasing the gate. Combined evaluation exit is 1. Details are in
`build/math-port/focused-evaluation-g252.json`.
