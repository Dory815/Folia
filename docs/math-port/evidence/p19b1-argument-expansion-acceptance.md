# P19b1 root local acceptance — native raw argument expansion

Root accepts the bounded native Property-domain host operation frozen in
`../argument-expansion-contract.md`, not TokenMap, MapHandler or a TeX parser.
Root froze the interface and responsibilities before the Luna implementation,
read the complete implementation and pinned CJS lowering, and owns the oracle,
tests, mutations and acceptance. The implementation imports only portable
generic Property/numeric/UTF-16 services; no MML or JavaScript engine is used.

`argument-expansion-oracle.cjs` invokes the unchanged pinned MathJax 4.1.3
MacroMap.parse. It assigns each raw test value to the actual Macro `_args`
storage, then observes the callback's dense rest arguments or actual error
category. The Source Macro getter remains unchanged. Source TS/CJS hashes are
checked, with only line-ending normalization used to compare npm and checkout
TS content. The native helper is never an expected-value generator.

The **455 Source cases / 456 assertions** cover raw scalar/nullish inputs;
strings with NUL, supplementary characters, combining marks, surrogate
boundaries and lone units; dense/sparse arrays with own/inherited slots;
array-like records with own/inherited values and explicit undefined; numeric,
string and native object length coercion; positive-infinity/oversize ArrayCreate
errors; and 26 pristine intrinsic metadata identities. A native ownership
witness adds **9 assertions** for shared nested identities, holes, fresh result
storage, and independent slot vectors after input mutation. Successful baseline:
**465 assertions in 2 tests**. Mutation-safe size/optional guards ensure a faulty
result is reported by assertions rather than indexing invalid storage.

All **18** isolated mutants compile and are detected by unchanged expectations:
nullish behavior/error category, UTF-16 versus code-point splitting, order,
surrogate replacement/loss, array truncation/prototype/hole/order/identity,
generic length/coercion/rounding/clamping/error, inherited slots and order.
Every result is test exit 1 with failed assertions; timeout, crash and compile
failures are not accepted as detection. Runner checks the baseline's actual
465-assertion count and byte-guards working implementations and fixtures.
Results are `p19b1-argument-expansion-mutations.json`; logs/builds remain ignored.

Root ran the focused positional filter and the required full
`cmd /c build_test.bat 2>&1` after final test generation. Both pass. Ignored final
log is `build/p19b1-final-core.log`; the fixture is reproduced by oracle --check.

The separately reviewed `p19b1-profile-argument-domain-audit.md` traces literal
and TeX-defined Macro.args, `\\let` aliasing, config timing and `\\setOptions`.
Root verified its 27 file hashes and read the option-filter/config call sites.
This supports the fixed profile's produced argument data; it does not accept
the later parser or general embedding extensions. No arbitrary JS accessors,
Proxy/symbol/custom iterator/boxed-string overrides are bound by Property.
Invocation/getter order, partial iterator reads, receiver identity and general
regex behavior require their own packets. Engine-specific JavaScript apply
stack limits are not an argument-expansion implementation claim.

This is a host adaptation and increases no TS source-file count. Local source
acceptance remains **51/246**; full G0–G7 and native TeX/Common/SVG remain closed.
The Folia math production route and its QuickJS dependency are unchanged until
the complete replacement passes acceptance.
