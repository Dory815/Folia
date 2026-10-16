# Shared builds 208 and 209

These results do not accept the native renderer or Folia integration.
All observations use the current shared build; no executable copies are used.

Gate208 froze 2,160 inputs and ended with compiler exit 1, with no changed
inputs. The new String.match tests omitted Descriptor::access's required
setter argument at four sites. The native MML initialization test passed an
untyped empty initializer to overloaded MmlFactory::create. There was no
runtime evaluation of Gate208.

Root supplied explicit nullopt setter arguments and tree::FactoryArgs{}.
Gate209 then compiled all 61 targets, exit 0. All ten evaluation runners and
their children terminated before the frozen inputs were released. The
post-evaluation audit found zero changes among all 2,160 inputs.

| Gate209 observation | Actual result |
| --- | --- |
| Complete SVG/error comparison | 0/150 matched; failed |
| Common layout comparison | 0/75 matched; zero native SVG results |
| Promoted BBoldx SVG/glyph regression | 0/6; failed |
| Package captures | 25 attempted; seven failed |
| Checkpoint comparisons | 4/20 passed |
| Core | 0xC0000005; failed |
| Host, Parser, Foundation | Failed |
| Regexp, LinkedList | Passed |
| Three real Renderer lifetime drivers | Failed during initial rendering |
| Paired Scene routes | 150 equal error pairs, zero successful Scene pairs |

The full SVG capture runner exits zero because it captures errors successfully.
That exit is not SVG parity. The final evaluation explicitly includes the
hash-bound full SVG comparison and records evaluation exit 1. Equal Scene
errors are likewise not successful mathematical layout.

The native MML argument converters remove Gate207's first rendering error,
`Native MML argument has no Context Value representation`. Gate209 instead
reports `MML method is not callable`. Source inspection identifies a concrete
missing inheritance link: MmlFactoryContextMethods creates an
AbstractNodeFactory prototype containing only create and links MmlFactory to
it, but does not link AbstractNodeFactory to the generic AbstractFactory
prototype. initializeMmlNode reads getNodeClass from that factory, gets
Undefined, and invokes it. The generic Factory implementation already owns
that method. Root requires reuse of its actual Source module/class boundary,
not a copied getNodeClass implementation. The contract is recorded separately.

The Core access violation remains unlocated. The last printed test name is
buffered output and is not evidence of the faulting code. No global Context
retention theory has been established from code or runtime evidence.

Authoritative local records are build/math-font-package-gate-209.log,
build/math-port/gate-209-inputs-before.json,
build/math-port/diagnostic-exits-209-root.json,
build/math-port/checkpoint-comparisons-g209.json,
build/math-port/profile75-full-svg-comparison-g209-root.json,
build/math-port/profile75-scene-route-audit-g209.json, and the three
renderer lifetime provenance records for g209. Build products remain ignored.
