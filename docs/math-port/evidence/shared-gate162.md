# Gates161-162: shared Scene integration compile checkpoints

The current native path now separates Common/BBox/placement Scene production
from its SVG consumer. Renderer::renderScene captures the same completed Scene
without the SVG consumer; NativeRenderSession uses one leased Lite text probe
for unknown-text measurement. The paired 75-case/two-runtime-group probe
compares complete frozen fields and resources, counts every serializer entry,
and records independent route errors. It is registered as its own target.

The changes also integrate reviewed native owner semantics for factory fields,
Input/Output filter records, RegExp lastIndex and Promise queues, plus
cooperative cancellation. These are implementations awaiting acceptance.

Gate161 builds 57 targets against 2,073 captured inputs and fails compilation.
Diagnostics find dependent template base names, ambiguous arithmetic assignment
to the raw lastIndex cell, and standard header includes after Boost.UT's module
import. Those issues are corrected before Gate162. Gate162 also fails, exposing
remaining factory binding imports and test conversions to the new owner types.
Both gates report zero changed inputs. No current runtime test, complete SVG
comparison or paired Scene comparison is claimed.

The Gate160 access violation was reproduced on a single plain API conversion.
Its stack and freed Debug CRT pointer identify handleInlineBreaks: copying Node
shares the ChildList owner, and clearing the copied list invalidates the source
line reference. The correction creates independent children/attributes and
uses GeometryOwner::cloneFor to bind deferred geometry to the new node; it does
not eagerly stringify raw attributes or discard live geometry.

Two observation/test boundary defects are tracked separately. The XML oracle
uses a custom SVG helper adding own xmlns, unlike real SVG.svg, which passes a
namespace argument to html. Only the native observer's custom helper is changed;
production captureXmlNode is not changed to add xmlns. A helper-return test
also constructs an isolated XML node that reaches a Source-equivalent readonly
prevLevel error; its fixture requires a valid MathML parent structure. Generated
Scene parent inference and foreignObject typed transform units remain under
independent review. These are not covered by native-to-native Scene equality.

Pinned CJS witnesses run successfully for RegExp raw/readonly lastIndex,
filter record descriptors, live factory fields, Source measurement cache order,
children owner aliases and SVG namespace observation boundaries. Their logs
are under build/math-port with g160-root-source names. These Source runs do not
prove that the new C++ implementation compiles or runs correctly.

Authoritative build artifacts: build/math-font-package-gate-161.log,
build/math-font-package-gate-162.log, and each gate's inputs-before/input-changes
JSON files. The last complete passing public SVG comparison remains Gate158.
Folia still uses its math QuickJS path and Mermaid remains paused.

The temporary standalone exception diagnostic binary, PDB and linker outputs
were removed after recording their hashes, releasing 929,275,636 bytes. Their
source, command scripts and stack log remain in
build/math-port/scene-helper-debug; shared build/core outputs are reused.
