# Gates222–223 package observation order

Both incremental builds completed all 61 requested targets using at most four
compile jobs. Their 2,187 registered input entries remained unchanged through
the selected package captures and comparisons; executable hashes matched
before and after execution. These gates did not rerun Core, SVG or lifetime
acceptance.

Gate222 explicitly separates compile, tree serialization, packageData and map
observation into full expressions in the next-six and font-package observers.
The next-six comparison passes all 20 records against the unchanged pinned
Source baseline. The state trace now reaches packageData after compile with
the same canonical jax/ParseOptions/mathItem/root/texParser identities.

The font comparison initially remains 13/16, now differing in a repeated Map
alias marker. Source plain() records its root Map in its WeakSet before
visiting entries; the native observer's manual root traversal did not seed
that canonical identity. Gate223 adds that seed, matching the existing
next-six observer. Its full font-package comparison passes all 16 records.
No production mathematical fields, expected records or comparator exceptions
were added to obtain either result.

A small C++23 experiment reusing the existing std module and compiler flags
logs the old stream expression's actual call order as maps, packageData,
compile, node. A separate header-based control gives the same result.
Enabling `/w14866` emits C4866 for the stream calls. Microsoft's C4866
documentation describes the limitation when operands contain objects passed
by value and recommends avoiding order-dependent operands. The explicit
full-expression observer phases avoid that limitation. An attempted
`/Zc:evalOrder` option is unrecognized by this compiler and is not adopted.
See [Microsoft's C4866 documentation](https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/c4866).

Artifacts under build/math-port include the per-gate capture/provenance files,
next-six-focused-comparison-g222.json,
font-packages-focused-comparison-g222.json,
font-packages-focused-comparison-g223.json, both input audits, stream-order.cpp,
the std/header control outputs and stream-order-warning.log.

Tooltip composition still fails the Gate221 Scene/SVG checks. Context owner
refactoring remains subject to its own compilation and lifecycle tests. These
bounded package results do not establish full renderer acceptance; Folia
continues to use QuickJS for math.
