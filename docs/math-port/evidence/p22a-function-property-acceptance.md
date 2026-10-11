# P22a Root local acceptance — callable property identity and sparse slice

Root locally accepts the five native host helpers under
`../p22a-function-property-contract.md`: `functions::reference`, `fromProperty`,
`toProperty`, FunctionValue strict equality and `PropertyArray::slice(size_t)`.
They bind shared data values needed by complete TokenMap; they are not a port
of concrete TokenMap algorithms and increase no complete TS file count.
MathJax Source acceptance remains **53/246**; all complete rendering/Folia
gates and the parallel Mermaid requirement remain open.

Root froze module ownership, constructors, signatures and independent actual
Source fixtures in 2120c0a before Luna implementation. `Missing` is owned once
by object_types; callable_types no longer imports Property. Property imports
the callable leaf and carries the original FunctionRef in its single value
graph. FunctionValue's typed/raw forms are bindings to that canonical owner,
not separate function objects/bodies. Invocation resolves either binding,
preserving saved body, receiver and mutable closure state. PropertyArray slice
snapshots length, creates fresh sparse storage, uses prototype-aware presence
and copies present values to shifted indices. Own undefined, inherited undefined
and holes remain distinguishable. Function/object values keep shared identity.

Root reviewed the two Luna cpp implementations against these responsibilities.
Implementation commit: 695539b. Root native protocol changes stay separately
reviewable from the implementation. No parser/map feature, JS VM, platform API
or source-string special case was introduced by this packet.

`function-property-oracle.cjs --check` independently executes Node 24.13.0,
actual locked TokenMap.js/MacroMap and FunctionList.js plus ordinary JS arrays.
It reproduces **640 slice cases, 16 graph/call flags, 169 strict comparisons**,
Source fixture SHA256
`591a387988dcb3b72ee899792a2afb36ee54462b50ebfc875e93940bef04a216`.
The generated native suite exercises four typed/raw equality crossings, actual
raw-Property FunctionList/TeX calls, receivers, ownership and invalid boundaries.
Its final baseline is **4,040 assertions in five tests**. MacroMap Source is an
independent reference for its data graph; the native test directly binds those
values and does not pretend the still-unimplemented native MacroMap exists.

The first red attempt failed compilation because a test exception name collided
with Boost.UT. Root corrected it in b421934. The corrected frozen stubs then
compiled and all five tests failed. The failed compilation was never accepted.

Root's isolated final mutation matrix detects **22 compiling semantic faults**,
each by unchanged assertions with exit 1, without a crash counting as detection.
Tracked results: `p22a-function-property-mutations.json`. Initial clearing of a
direct function handle exposed a null dereference in the test itself. Root
added handle assertions/access guards, preserving Source expected values, then
reran the entire matrix successfully. Raw/typed erasure/cloning, cross-binding
identity, NaN equality, slice bounds/holes/inheritance/undefined/aliasing,
invocation bindings, null/unbound metadata and closure state are all covered.

Final `cmd /c build_test.bat 2>&1` exits 0: **86 suites, 3,927 tests, 161,045
assertions**, including the final 4,040-assertion suite. Ignored evidence is
`build/p22a-function-property-final-core.log` (raw SHA256
`931e46d3432757b859106f69d19d26e2d2f64b263f6191a91b1977651db42ef0`).
Existing receiver mutations all detect (12; unchanged 79-assertion baseline).
Numeric and primitive dependency harnesses also compile, pass their original
baselines, and detect all six/four unchanged mutants. Root updates the 25 old
isolated module manifests for the new callable leaf dependency; changed
function-type graphs additionally link function_value.cpp. These are mechanical
build-graph repairs, not changed old fixtures or relaxed expectations. All old
mutation matrices are not claimed rerun; complete core tests cover their current
production modules. No platform/editor test is required for this portable change.

Generic function-object metadata/coercion/getters/proxies, custom array species,
signed JS index conversion and custom iterators are outside this narrow binding.
Metadata access explicitly reports the unbound host. Any required configured
callsite must acquire its typed host contract before whole TeX acceptance.
Concrete TokenMap constructor ordering/registry identity, actual regexp host,
parser/Stack/NodeFactory, Common layout/scene/SVG and Folia replacement remain
required. This local acceptance does not authorize production integration.
