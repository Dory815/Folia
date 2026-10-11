# Root acceptance: MML inheritance closure and token text

Root reviewed all eight delivered bodies against the pinned original sources and
the established sparse/tagged graph. Only four base/token bodies, two fraction/
script hooks and the two frozen arithmetic helpers changed. There is no dense
fallback, second parent/child store, new declaration or source-text special case.
Both original numeric fraction increment and string-capable script addition are
retained; fraction rereads current children while script traversal retains its
captured array. The generic and token hooks preserve their distinct dispatch.

## Independent rejection and original comparisons

Root rejected two initial builds: arithmeticChild returned a pointer for a
reference signature; the two script local references had the same mismatch.
The implementation agent fixed only those dereferences. Neither failed build
counts as acceptance. The complete 90-module/all-unit graph then compiled and
passed the frozen **753 comparisons** (699 new Source observations and 54 old
Source299 consumers). The pre-delegation red was 699 failed comparisons plus
52/54 failed old consumers, with two old copy/NodeUtil consumers already green.

Root subsequently rejected surviving scriptlevel-truthiness and linebreak-
fallback mutants. Another denominator-alignment-key mutation survived because
the child discarded that unsupported key; the argument itself was unobserved.
Root added independent original observations, leaving the original gold intact:
128 callback/alignment cases, 16 explicit null/zero level cases, 16 falsey indent
cases, eight TextNode isolation cases and eight fraction call-argument cases.
The isolation observer calls the qualified original TextNode method if reached;
the fraction observer calls both qualified original child methods before tracing
their arguments. Neither replaces the algorithms with expected-value stubs.

Final comparison: **929 assertions / 5 tests**, all passing, zero unimplemented
calls in the selected path. All six Source profiles and all five native generators
reproduce unchanged with --check. Source699 SHA remains
`4f48ef03c00e70c549029b25c23a92810ca7b25385d102c300686a55c040a459`.
Supplement profiles and SHA:

| Original observations | Count | SHA-256 |
|---|---:|---|
| callback/alignment | 128 | db7478e5fc0006003df4dd4fd00b4ad0d7d27cc8cec4a0eecd4ee9d5f808fedf |
| explicit level | 16 | e6bc4c64db4ebeedd6581b77eecf14003f01e2f7e99af7bb96bc35a26b13d9d8 |
| falsey indent | 16 | 892d75dd398dc0c74f1b669dd96570a72a33e2f9e74c22a7fd3a055a2b304297 |
| token isolation | 8 | de9a07b51dec7bc6a0dc98e5240fb5e070591b93a3e25472960673e987326dfc |
| fraction arguments | 8 | 1cf778caa6baaa7b34c568b72443a27a74a3d71e7034f87a6e7ab2c2da2c4598 |

These supplements were generated after delivery and are not claimed as part of
the pre-delegation red. Earlier rejected runs remain diagnostic evidence.

## Mutation and regression gates

All **47** final method-scoped semantic mutants compile and fail assertions,
covering defaults/own membership, forbidden/stopped attributes, alias safety,
explicit null/zero, raw prime, arity repair/parents, indentation, live/captured
arrays and prototype Get, token membership/callback/errors, fraction and script
arithmetic/arguments, and both helpers. Crashes and compile failures do not count.
The isolated runner byte guards all working production/fixtures.
Result: build/math-port-mml-inheritance-mutations/results.json, SHA
`8bc1cf3396bc383eeb62a68ca82c4064da6536667fd03268ef9300e3033419bd`.
Baseline log SHA:
`a984980d18f3f13a97bf34be41bdd9cc2f2646a518f1146b4c4bc0e908f63657`.

The independent regression matrix also passed **1,765 assertions / 17 tests**:
inheritance, returns, predicates, copy, construction, append and NodeUtil children,
plus a compiling prime-type witness mutation. This run preceded the final eight
fraction argument observations, which are verified by the final mutation matrix.
Regression baseline log SHA:
`782ff9b5938bb897cb41a541e20b85b11e0ea695f3927307656fed5546959a84`.

The remaining 54 immutable Source299 consumers now pass, completing the prior
245 plus this 54-case closure. Whole-file acceptance stays **54/246**. Arbitrary
JS proxies/foreign nodes, non-string textContent returns and unbounded custom
arities are outside this typed API's certification. Full-core stale observer
repair, remaining MML/TeX/Common/fonts/SVG and Folia remain required and open.
