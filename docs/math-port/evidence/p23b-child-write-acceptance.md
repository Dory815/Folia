# P23b Root acceptance — assignment receiver binding

Scope: the Root-defined `ChildValue<Product>::requireNodeForWrite(member)`
primitive, prerequisite to the full graph migration in
`p23b-node-binding-decision.md`. FactoryChildren, parent storage and the 31
production child consumers have not migrated. No full Source-file or Folia
gate advances; math remains 54/246 local Source-file acceptances.

Root froze the concrete member declaration, generated an independent observer
from the existing actual Source111 appendChild-U/N cases, compiled the real
Unimplemented stub and observed the new test fail, then implemented the member.
The two original Source errors are `Cannot set properties of undefined/null
(setting 'parent')`; the actual Source array already contains that tagged value
when each error is thrown. Those Source expected values were unchanged.

The member returns the same borrowed Product identity for a Node value.
For Null/Undefined receivers it throws options::Exception with Source TypeError
name and exact assignment message. It does not assign parent, mutate the value
or own the Product. Existing requireNode remains the read-receiver boundary;
neither method rejects a Null value assigned to a valid node's parent.

Validation:

* `freeze-child-write.cjs --check`: two original Source cases, one generated
  native test / seven assertions. Golds are read from Source111, not native code.
* `build_test.bat "*math_port_child*"`: all four host tests pass with 16,550
  assertions (existing 16,543 plus seven). Root changed no Array algorithm.
* `child-write-mutations.cjs`: five variants compile and fail assertions with
  exit 1: lost Node identity, reading-vs-setting message, operation text,
  Null-vs-Undefined tag and exception class. Original source/fixture bytes stay
  unchanged. No crash, timeout or compile error counts as a detection.
* Complete core suite: exit 0, 89 suites / 3,944 tests / 178,465 assertions.

Raw ignored build evidence SHA-256:

| Artifact | SHA-256 |
|---|---|
| `build/p23b-child-write-red.log` | `5d07d3e538a4710390a5a59a2a100971942453bd38dd145710d44a002c96673b` |
| `build/p23b-child-write-native.log` | `449ea20bf7da06af8b3b0d0e2f3dee282f6f87529045bc3d483319c21f2c3d75` |
| `build/p23b-child-write-full-core.log` | `5b309ac0a7baef12d0b869e6a6ce3197a881885da1566bb682b868ded1e0dbaf` |
| `build/math-port-child-write-mutations/results.json` | `a5e102b98ee6dfbc725829359b3851ab3464f6d989955b36e7031ed4edf3f5b4` |
| `src/math/core/tree/child_array.ixx` (working bytes after this addition) | `6095ad8b31672391a1e1210923ef8a818a5307d537357df45c6c64b4f986fe5d` |

This is a narrowly accepted read/write dereference primitive, not native parity
for Node/NodeUtil/MML mutation, parent accessors, parser, layout or either output
backend. Source331 newly frozen MML/return observations still need their real
native counterpart and full-graph mutation acceptance.
