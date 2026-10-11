# P23a Root acceptance — typed sparse array prerequisite

Accepted only within `p23a-child-array-contract.md`'s numeric ordinary-array
binding. This is not acceptance of the dense-to-sparse node migration, a whole
MathJax file, TeX parsing, Common layout, SVG, Direct2D or Folia integration.
FactoryChildren and its 31 production consumers still require atomic migration.
The complete Source-file tally remains 54/246, with 192 not_verified.

Root froze declarations, sole storage, product ownership, actual Source corpus,
native observer and real red failures before dispatching Luna math_layout. Luna
edited only ChildArray bodies/helpers and its implementation report. Root ran
all builds, Source replays, semantic mutations and the final review.

## Independent evidence

* `child-array-oracle.cjs --check`: 8,256 actual Node 24.13.0 Array cases;
  `9533e7b450232aa81dcf06b119093bc67a062a18d5def5b081b4a0d7ca7378c0`.
* `freeze-child-array.cjs --check`: unchanged generation from those independent
  Source observations; three native tests, 16,543 assertions, exit 0.
* `child-array-mutations.cjs`: 32 altered implementations all compiled and each
  exited 1 through failed assertions. No compiler failure, crash or timeout
  counts as detection. Original implementation and fixture bytes were unchanged.
* Complete `cmd /c build_test.bat 2>&1`: exit 0, 88 suites / 3,943 tests /
  178,458 assertions. This portable storage change required no Windows editor
  interaction or app restore. Previous core total plus these three tests equals
  this total; no existing expected value was changed.

Raw logs are ignored build artifacts. SHA-256:

| Artifact | SHA-256 |
|---|---|
| `build/p23-child-array-real-red.log` | `4e7b30683c766ef1c77c89c5842c17490b929140f50e54fede3129db6054cc8e` |
| `build/p23a-child-array-native.log` | `0e5b3ab6d616dcfdc6a642f895abcd6f534b78e1a3edd8f535389c7e7e65a0f2` |
| `build/p23a-child-array-full-core.log` | `25b895bd3bfada7f0be04ed229fdeb277aab2d8673efba56c58e8e199446ee27` |
| `build/math-port-child-array-mutations/results.json` | `a83a97b1522a4eb4ab7bff6e397fc3b419dcf8151d7d1bce53231850fb3460e0` |
| `src/math/core/tree/child_array.ixx` (working bytes at acceptance) | `ce451b3b945270173487e4dc8f653adda91451957f08e612dc854546acdc67f1` |

The first mutation attempt exceeded spawnSync's default 1 MiB output buffer
because it generated many genuine failing assertions. Its interrupted result
was rejected, not accepted as detection. Root raised the runner's output bound
to 64 MiB, retaining the 30-second timeout, exact baseline count, successful
compile, exit-1 and failed-assertion requirements, then reran all 32 mutations.
The source golden corpus and assertion conditions did not change.

## Root semantic and ownership review

The only slot store is vector<optional<ChildValue>>. Absence, present Undefined,
present Null and borrowed node identity stay distinct. Get/Has consult a bound
numeric prototype even beyond own length; HasOwn does not. Set/resize grow
holes, erase preserves length, pop Gets before removing a slot, and slice/splice
preserve Has/Get/movement/Delete semantics. Splice uses forward movement when
shrinking and reverse movement when growing; removed arrays remain sparse.
indexOf skips absent properties and compares exact tags/identity. forEach reads
live presence/value over captured length and supplies the original receiver.

Array handles own slots/prototypes only, never Products. Factory/session
unique_ptr ownership and stable borrowed Product addresses remain the node
identity boundary. Prototype assignment checks cycles before mutation, keeping
the shared array ownership graph acyclic. forEach retains a shared self handle
across callbacks; positive lifetime/rebind/expiration assertions and code review
cover this requirement. A removal of the lifetime hold could invoke C++ use
after free, so a crash from such a mutation would not count as semantic evidence;
no lifetime-loss mutation is claimed in the 32 valid detections.

Signed index normalization avoids INT64_MIN overflow; growth/addition guard the
native vector allocation domain. These are typed numeric child operations,
not a claim about JS species, exotic descriptors, accessor numeric prototypes,
generic named properties, custom iterators or ECMAScript's universal array
length limit. Required Source call-domain closure still belongs to the next
root-frozen node/MML/TeX migration gate.

ChildValue.requireNode is a **read** dereference boundary with precise Null vs
Undefined messages. A parent assignment in the next migration needs a distinct
write dereference binding; reusing its reading message for Source's setting
error would be wrong. No implicit pointer cast or blanket null projection may
be added to make old consumers compile.

Root also replayed existing sparse Node/NodeUtil Source111 evidence unchanged.
The new actual MML Source299 evidence belongs to the next prerequisite and has
no passing native counterpart yet. In particular Source MML copy deliberately
turns falsey children into explicit Null, while generic copy skips them and
NodeUtil.copyChildren writes explicit Undefined for a hole. Preserve the
individual Source algorithm rather than enforcing one global conversion.
