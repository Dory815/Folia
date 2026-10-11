# Root three-observer compile acceptance

Accepted only simple/mfenced/maction native observer binding repairs. Original
JavaScript computations, inputs and expected values remain unchanged. Root
regenerated the three concrete fixtures, replayed all --check oracles, verified
unchanged original prefix/value tables, and independently compiled all fixtures
over the actual 90-module graph. No production algorithm or semantic runtime
test was accepted by this compile-only packet.

Profiles remain simple 106 cases/7,667 assertions/1,605 raw rows/296 records;
mfenced 225 cases/984 assertions/509 records; maction 934 cases/935 assertions
including its real-tree test/2,318 records. Preserved sourceValues table hashes:

| Family | SHA-256 |
|---|---|
| simple | 08b5ba33439a0fd6c0639dedac19e31614b226105a4adbdacbdf281834064020 |
| mfenced | 226b5161981fba0c567ede972e923dd8f3e9bf21b100072ae18d793a6c1025c4 |
| maction | 05ce7985818966da1b6c593cf5be069f5b417c707bd9bf21a372ed54a812e88f |

Probes now use existing tagged virtual signatures and ChildArray. Native map
observations capture the receiver/initial length, use Has/Get and preserve
holes in PropertyArray. Known factory products remain raw nodes; result/parent
observations use tagged services according to the exact original helper.
Simple CPP result declarations changed to NodeValue; original result identity
comparisons, matrices, assertions and expected records remain intact.

Root corrections mattered: simple's old native emitter lagged the fixture's
AttributeList hook, although its Source expected suffix reproduced exactly.
Luna's partial mfenced replacement accidentally modified the original JS
callback to use a Native-only nodeOrNull method. Root rejected it and restored
the exact original prefix before replay; no changed Source expectation entered
acceptance. Root also rejected a proposed TraceFactory for-of adapter because
the original uses map. Luna's subsequent writes failed on mapped-file
truncation; Root completed native-only changes through exact checked in-place
writes without truncating/moving the files or terminating processes. See the
implementation reports for ownership and partial delivery, not pass claims.

node tools/math-port/container-observer-compile.cjs compiles all three fixtures
against 90 actual interfaces. Ignored compile-result SHA-256:
08509d260c885cf213293a75ae1748e2ddd2b7c8fda1da6dd81f122fdf702d45;
build log SHA-256:
9c5fbde7b5a9e29c560e683ebf237c681c8d82fa301fb9497be1d27db40b2503.
The result explicitly reports semantic_tests_executed=false and all working
implementation/fixture files unchanged during the compile.

Root's new keep-going repository inventory confirms six remaining stale
fixtures, down from nine: table, mo_order, atom_semantics, multiscript,
arithmetic and arithmetic_order. Ignored inventory log SHA-256:
ed8729aecf328b69921cacb332372e3f8d2e4cbeaa22756718bbc3252b194ad9.
Full core tests still cannot run. No parser/output/backend/Folia gate or
historical 54/246 whole-file tally advances from these observer repairs.
