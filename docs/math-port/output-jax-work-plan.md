# Core OutputJax and native output-host contract

Root-owned contract; implementation and native acceptance are outstanding.
Reference: MathJax-src 4.1.3, commit
`fb987178c2d279a99b0db5ea02e751691703955e`.

## Source responsibilities and target files

| Source | Native module | Responsibility |
| --- | --- | --- |
| `ts/core/OutputJax.ts` | `core/output_jax.ixx/.cpp` | Base options, live class name, adaptor assignment, base lifecycle and pre/post filter execution |
| `ts/core/DOMAdaptor.ts` | `core/dom_adaptor.ixx/.cpp` | Output-host interface and the actual abstract node/attribute/property/class/replace helpers |
| `ts/adaptors/liteAdaptor.ts`, `NodeMixin.ts`, `lite/*` | Corresponding `adaptors/*` modules | The reference profile's output DOM ownership, style, serialization and measurement behavior |
| `ts/output/common.ts` | `output/common.ixx/.cpp` | Common output lifecycle, metrics, font service, wrapper map, line breaking and stylesheet services |

These are separate packets. Porting OutputJax does not accept its concrete
adaptor, CommonOutputJax, wrappers, scene construction or SVG. Source base
methods that return null or do nothing retain that behavior; derived required
methods cannot be replaced with those base defaults.

The OutputJax SHA-256 is
`e4932ba7a5d8f782022fa0ab3c8a8bc05a3072704cd8802213fdadb2722789a3`.
DOMAdaptor has
`fc4bc991ab2e9a35fd2693a38ff6915286e549d884591922a5d7a716630e8af6`.
Root's `tools/math-port/output-jax-original-oracle.cjs` records 14 actual
observations from the pinned CJS class, with source/package hash checks. Its
constructor, identity, filter, error and prototype observations are a minimum
acceptance set, not generated C++ expectations.

## Native OutputJax interface

Reuse `core::MathOutputJaxService`, `MathItem`, `MathDocumentReference` and
`MathOutput` from `core/math_item.ixx`. `OutputJax` implements that service and
extends `NativeObject`. It owns one options Context object, canonical
FunctionList instances and one adaptor reference. No second math item, output
job, metrics store or alternate parser graph is permitted.

Follow the existing `AbstractInputJax` class-binding lifecycle: allocate the
actual derived instance/class identity before the Source base initializer
reads `this.constructor.OPTIONS`. The `name` accessor reads the current class
NAME at access time. Do not freeze NAME per instance or initialize through the
C++ base class's static defaults. Options use existing `defaultOptions` and
`userOptions`; no private substitute merge or filter parser.

Provide `setAdaptor`, `initialize`, `reset`, `getMetrics`, `styleSheet`,
`pageElements` and protected `executeFilters`, with pure virtual `typeset` and
`escaped`. Base stylesheet/page methods return the actual null output marker.
Distinct instances own distinct pre/post FunctionLists. Adaptor references
preserve object identity and explicit Missing/Null; no no-op deleter disguises
borrowed storage as ownership.

`OutputJaxFilterData` has the same math/document/data responsibilities as the
existing input filter record. One live record is passed to every callback;
changes to its `data` are returned, including Missing/Null and replacements.
Filter order, equal-priority insertion, false early termination and thrown
exceptions come from the canonical FunctionList. Every constructor-export,
class metadata, prototype or descriptor operation in the frozen observer must
either be implemented and compared or remain an explicit incomplete gate.

## Concrete host boundary

Folia's original entry uses `liteAdaptor({fontSize:16})`, not a browser
measurement API. Native measurement must follow that exact LiteAdaptor and
NodeMixin behavior. The existing Source CJK range/counting and unknown-character
width/height defaults are reference behavior to port, not permission to choose
new metric heuristics. Do not replace them with DirectWrite measurements during
SVG parity work. Future platform typography must be an explicit render profile.

The output host owns actual element/text/document objects, ordered attributes,
parent/children, style state and node identity. It is a host DOM graph, distinct
from the MML input graph and the completed geometric scene. MML's existing XML
adaptor is not implicitly a complete output DOM implementation.

The host must preserve Source mutations (append, insert, remove, replace,
clone, split), property/attribute distinction, namespace handling, escaping,
class behavior and lazy Styles authority. Define ownership/lifetimes before
assigning method bodies. A generic output reference may wrap that actual host
object; it may not wrap a serialized SVG string to simulate a DOM.

## Common and backend boundaries

CommonOutputJax captures one canonical font instance and constructs wrappers
over the actual MML nodes. Wrapper/BBox caches and the Source nodeMap use actual
identity. Common measurement reads live sparse glyph tuples and options;
backends do not normalize or copy the font graph into another glyph model.

Source SVG placement/assembly methods are extracted into the documented scene
builder, preserving their per-kind responsibilities. One completed scene feeds
SVG and future Direct2D. The host DOM can express wrapper/lifecycle output, but
must not be parsed to recover scene geometry. No Direct2D types enter `src/math`
or platform-independent core modules.

## Acceptance

1. Native observer emits actual values/identity/descriptor traces for all 14
   original rows; strict whole-record comparison passes.
2. Focused tests cover construction through a real derived class, options
   mutation, instance isolation and filter state replacement/short circuit.
3. Root reviews every Source method and observes faults independently;
   deliberate ordering, shallow-copy and replacement mistakes must be caught.
4. Fresh shared build links the native observer/tests and the ordinary core
   build remains valid. Native Source-facing errors retain names/messages.
5. The DOM/LiteAdaptor and CommonOutputJax packets are independently frozen and
   accepted before wrappers depend on their production implementation.

Do not call the full renderer clean or complete based on this packet's success.
