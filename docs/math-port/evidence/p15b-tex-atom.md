# P15b TeXAtom method evidence

The pinned MathJax 4.1.3 source is `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/TeXAtom.ts`, SHA-256 `bad114224392b94852456a4636b85ad90a5a2c96b63e6c7911d155db299f7028`. The production-resolved CJS file is `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlNodes/TeXAtom.js`, SHA-256 `bf72fd0716f88d144a8ba0befef383de4a0a4fd4d690d37fec9d55b2f15a02e4`. The native implementation is [tex_atom.cpp](../../../src/math/core/mml_tree/mml_nodes/tex_atom.cpp). Both source-derived files retain MathJax's Apache-2.0 attribution; see `src/math/LICENSE-MathJax.txt`.

## Method mapping

| Effective source behavior | Source location | Native mapping |
| --- | ---: | --- |
| `TeXAtom.setTeXclass(prev)` | TypeScript 87–90; generated CJS `TeXAtom.prototype.setTeXclass` | `MmlTeXAtom::setTeXclass` enters one `TreeCallGuard`, checks/gets required child zero with `host::arithmeticChild`, calls that child's virtual `setTeXclass(nullptr)`, discards its result, then calls the receiver's virtual `adjustTeXclass(previous)` and returns that result. It does not call base `setTeXclass` or `getPrevClass`. |
| Class-body `TeXAtom.adjustTeXclass(prev)` stub | TypeScript 97–99; generated CJS method before the final assignment | The stub itself returns `previous`; it is not the effective method after module evaluation. It is documented here and not separately implemented as runtime behavior. |
| Final `TeXAtom.prototype.adjustTeXclass = MmlMo.prototype.adjustTeXclass` binding | TypeScript 101–104; final CJS assignment immediately before source map comment | `MmlTeXAtom::adjustTeXclass` delegates directly to the root-shared `adjustOperatorTeXclass(*this, previous)`. It has no second `TreeCallGuard` and contains no copied algorithm. |

## Dispatch and adaptation notes

The order in `setTeXclass` preserves the source's child-first behavior: failure to read child zero prevents adjustment, the child receives a null previous node, and its return value is ignored. The original `previous` pointer is forwarded unchanged to the virtual adjustment method on the TeXAtom receiver. `arithmeticChild` validates the accepted dense native child-array representation at the required read and raises the typed host `TypeError` instead of dereferencing a null/out-of-range pointer.

The final prototype assignment makes the effective JS method function object the very same function object as `MmlMo.prototype.adjustTeXclass`. Native C++ does not expose prototype function identity in this interface; the root-frozen adaptation preserves the single shared operation's calls, effects, virtual receiver state, and return value without claiming identical JS function-object reflection. The shared body and its `TreeCallGuard` are owned in `operator_class.cpp`; this wrapper contributes no second guard.

The actual-CJS oracle was frozen independently by root before dispatch at 6,828 assertions across 63 cases, including 924 shared-operation raw rows. No oracle, generated expectations, interfaces, registration, tests, or build files were modified, and no build or test was run for this packet. This evidence records the implementation mapping only; root owns differential review, mutation/build checks, and acceptance.
