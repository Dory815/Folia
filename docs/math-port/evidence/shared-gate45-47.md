# Output compilation: Gates45 through 47

These are compiler-feedback checkpoints for the actual Common/Lite/Scene/SVG
production modules. They do not establish native TeX-to-SVG parity.

Gate45 adds the Source script wrappers, the canonical wrapper-class binding,
and concrete SVG factory/Jax bindings. All 1,025 input hashes remain unchanged.
Compilation exits 1 on Lite Styles visibility, remaining nodeProperty calls,
Common polymorphic getter declarations and the FunctionArgs namespace.

Gate46 advances into Common per-kind algorithms, output bindings and Scene
painting. The actual build session 40394 exits 1. One of the 1,025 captured
inputs, `common/wrappers/munderover.cpp`, changes during compilation; this gate
is not a clean frozen snapshot. No completed target is accepted from it.
Owners repair the compiler-reported array, bbox, option-value and interface
errors after terminal release.

Gate47 explicitly adds Source merror placement and a foundation test target
using real Lite/Context objects and completed Scene/Serializer objects. All
1,029 input hashes remain unchanged; actual session 68909 exits 1. The Scene
ownership test translation unit compiles. It has not linked or executed.
The remaining diagnostics occur in six production files and the previously
uncompiled Lite projection test:

- LiteParser callback types and LiteModule Context calls/RegExp service member.
- Common wrapper parent traversal, script temporary bbox and reference access.
- CommonOutputJax's Source method changes containerWidth through a const method.
- The Lite test needs explicit constructor arguments and actual Context/DOM
  handle types; diagnostic correction must retain its identity assertions.

Production wrapper construction also has an independent architectural defect:
CommonWrapper's C++ base constructor invokes virtual Source initialization
methods before the concrete derived class is active. MathJax invokes derived
methods at this phase. Repair requires ordered Source initialization after
allocation/adoption of the real derived product, with one canonical wrapper
identity and one execution of each constructor phase. It is not resolved by
successful compilation.

The SVG constructor bootstrap must preserve actual constructor/prototype
identity, distinct mixin applications, descriptors and executable methods.
Named empty constructors and a guessed inheritance graph do not satisfy this
boundary. Full native FontData and production session service lifetime remain
required alongside the unfinished per-kind/Common and SVG placement methods.

Logs and before/after hash manifests are in ignored `build/math-port` and
`build/math-font-package-gate-{45,46,47}.log`. Fresh executed diagnostics still
come from Gate41; the latest fully passing core remains Gate36. Folia still
uses its existing math QuickJS path. Mermaid remains paused.
