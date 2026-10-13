# First output closure compilation: Gates42 through 44

Root enrolls the present Source-aligned Common, Lite DOM, Scene and SVG modules
through explicit CMake file entries. This is a compiler-feedback checkpoint;
new module bodies have not passed full native rendering acceptance.

Gate42 exits 1 during scanning because Entities' generated include is missing
from the build dependency/include graph. Root registers the pinned TS/CJS hash
checking generator, all 60 Source inputs and the generated include under the
ignored build directory. Separately, two mtr input hashes changed during Gate42:
Root refreshed the enrollment after the owner believed these files remained
unregistered. This is an enrollment communication race; Gate42 is not reported
as a clean frozen build. Subsequent freezes explicitly include ms and mtr.

Gate43 successfully generates Entities but exits 1 at module dependency
collation. Actual MSVC `.ddi` records show both `output_jax.ixx` and
`output_jax.cpp` providing the same `common:output_jax` partition. The `.cpp`
also imports `:bindings`, creating the reported provider cycle. The same
partition implementation declaration error occurs in Scene bodies. Owners
replace those with normal primary-module implementation units that import
their interface partitions. Root's interface graph check then finds no cycles
or external partition imports. All 1,010 frozen Gate43 hashes remain unchanged.

Gate44 reaches actual C++ compilation and exits 1. The first failures are:

- LiteParser lacks the import owning LiteNodeRef.
- LiteNodePropertyBinding uses LiteNodeStore before its same-module declaration.
- A duplicate nodeProperty overload for the LiteNodeRef alias makes ADL ambiguous.
- CommonWrapper passes itself as AbstractWrapper's base template argument,
  producing recursive inheritance instead of the actual generic Wrapper base.
- CommonWrapper lacks the canonical constants import for BIGDIMEN.

Owners save repairs after terminal release. All 1,010 Gate44 input hashes remain
unchanged; its exact execution handle was 39549 and terminal exit code was 1.
No library or renderer acceptance is inferred from successful dependency scans
or individual prerequisite object builds. Gate41 remains the latest freshly
executed parser/host comparison; Gate36 remains the last complete passing core.

Ignored build logs are `build/math-font-package-gate-{42,43,44}.log`; each gate's
input manifest and changed-input record live under `build/math-port/`.
`shared-build-state.json` records Gate44 as terminal/editable. The current
module owners continue Source algorithms, canonical bindings and the actual
SVG pipeline. Mermaid remains paused and Folia still uses its existing math
worker pending native acceptance.
