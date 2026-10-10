# P6 root acceptance: generic factories

Status: the three generic factory sources are accepted as a local dependency
prerequisite under revision 2 of `../factory-contracts.md`. Node, Wrapper,
MML, parser, font resolver, Common layout, positioned scene, SVG and production
replacement are not accepted by this packet.

Root froze the native service declarations and upstream-generated expectations,
then assigned disjoint files. The factory agent owned only Factory and its
ledger; the parser agent owned only the two bridges and their ledger; the font
agent owned only the read-only E9 usage audit. Root owned contracts, fixtures,
builds, mutations, scope and acceptance. Source hashes and clone/package source
equality are checked by `tools/math-port/factory-oracle.cjs` against pinned
MathJax 4.1.3, commit `fb987178c2d279a99b0db5ea02e751691703955e`.

## Source review and rejected implementations

Root compared every field and executable member of Factory.ts, NodeFactory.ts
and WrapperFactory.ts with the per-member ledgers. Interfaces become typed
native services; callbacks carry synchronous argument spans and identities.
The generic factory, direct node factory and wrapper forwarding remain separate
modules. No semantic node model, parser or renderer was added to these files.

The first implementation merged the class Map and creator object. Root rejected
it: upstream closures capture the registration's class, and MmlFactory.MML and
CommonWrapperFactory.Wrappers return the same live creator object. The accepted
implementation has independent protected `nodeMap_` and stable `node_`, with
fresh captured creators per registration. Direct changes to either object do
not synchronize the other; saved creators retain their class after deletion or
replacement. Map order and initial Object.keys order are independently tested.

Root also rejected constructor availability and nativeType as prerequisites
for instance tests, instance predicates as prerequisites for construction, and
an extra returned-pointer ownership validation. These are separate native
bindings. Real class predicates preserve aliases, inheritance and rebinding;
reported node kind is never used as class identity.

A read-only second-agent review identified inherited Function.prototype names
after __proto__ registration. Root added real-JS witnesses and froze their
TypeError behavior before authorizing the correction. Generic/direct fallback,
retained prototype creators and own-undefined masking are tested separately.
Factories cannot copy or move because creators borrow their service address.

## Independent verification

- `factory-oracle.cjs --check`: nine actual pinned-JS cases, 110 behavioral
  assertions; none of their expectations originate in native code.
- Native tests: **114 assertions in ten tests**. The four additional assertions
  verify the native ownership adaptation: an adopted partially initialized
  product survives abrupt completion, and session destruction releases all
  products. Compile-time assertions prohibit copying and moving factories.
- `factory-mutation-test.cjs`: an isolated baseline passes, then eight changes
  compile and fail unchanged assertions: missing generic fallback, direct
  dispatch gaining fallback, re-reading a captured class from the Map, reversed
  Map order, deleted retained prototype, missing Function.prototype errors,
  lost property identity and duplicate kind reads. Working sources never mutate;
  logs/results remain in ignored `build/math-port-factory-mutations`.
- `cmd /c build_test.bat 2>&1`: complete core build and suite pass. The four
  modules are explicitly registered in the test-only folia_math library.
- The complete reference oracle still passes all 64 frozen snapshots and
  fresh-process observer-neutrality checks, and all eleven reference mutations.
  These establish oracle integrity, not native SVG equivalence.

Root corrected two mutation-runner errors before accepting final evidence.
An unqualified virtual call in the direct-fallback mutation recursively entered
NodeFactory; qualifying the base call made it the intended compiling behavioral
mutation. An overlapping rerun invalidated shared Ninja output; both runs were
discarded, a per-run exclusive lock was added, and the final complete run passed.
No expected behavioral values were changed to accept either error.

## Native adaptations and remaining gates

Revision 2 explicitly documents typed constructor records, injected derived
static defaults, per-product-family static registries, stable native service
addresses, synchronous spans and factory-owned products. E9 checks the actual
MML, wrapper and StackItemFactory callers; the TeX-local NodeFactory dispatcher
is a separate source and remains unported. Profile sources do not override
generic setNodeClass during construction, so the injected C++ initialization
does not claim arbitrary JS subclass constructor dispatch.

Inherited builtins can return non-node values in upstream JS. The native typed
service maps those invalid outcomes to TypeError; it does not claim universal
JS Factory API compatibility. Only the documented ordinary-object/arrow-prototype
bridge is modeled; arbitrary JS prototype graphs, descriptors, Symbol.hasInstance
overrides and erased cross-family registries are outside that API. VM wording
is not promised; explicit exception kinds are compared. Future real callers
requiring excluded behavior must extend a root-frozen service and its witnesses.

The live view's vector stores own-entry insertion order; it is not an Object.keys
enumeration API. Any later caller needing ordinary-object enumeration must use
the source's integer-index ordering rather than iterate that vector directly.
Node initialization, parenting and Wrapper traversal are future packets; their
interfaces cannot be filled with stubs or custom AST substitutes.

Eight of 246 inventoried sources now have local acceptance. The remaining 238
and external imports keep full G0 closed; complete G1–G7 remain closed. Folia
still uses its existing single QuickJS backend, and this packet does not change
the application's math behavior.
