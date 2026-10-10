# P6 generic factory evidence

This packet implements the pinned MathJax 4.1.3 `ts/core/Tree/Factory.ts`
generic factory in `src/math/core/tree/factory.ixx`. The source SHA256 is
`e61d2f5868287e2f382503cf690cd241698bf212957c1795d0cd35ce3bef049e`.
The source is Apache-2.0 licensed; the module retains MathJax attribution and
refers to `src/math/LICENSE-MathJax.txt`. Root's revision-2
`docs/math-port/factory-contracts.md`, `factory_types.ixx`, oracle, and frozen
reference tests define the native contract.

## Runtime member ledger

| Factory.ts member | Native implementation and behavior |
| --- | --- |
| static `defaultNodes` | Public inline static `FactoryClassMap<Product>` per native product family. The constructor reads the current map when `nodes` is absent and no derived defaults are injected. |
| instance `defaultKind` | Public string initialized to `"unknown"`; generic `create` uses it only after the requested creator is absent. |
| protected `nodeMap_` | Ordered native `FactoryClassMap<Product>` of kind/class-record pairs, independent of the creator object. Replacing a kind keeps its position; deleting and reinserting appends it. Direct edits to this map do not edit creators. |
| protected `node_`, public `creatorView()` | A stable `FactoryCreatorView<Product>` handle allocated once per factory. It represents the live creator object returned by MML/Wrappers getters. Its ordered own creator entries and prototype creator are separate from `nodeMap_`; direct view edits do not edit the class Map. |
| `constructor(nodes = null)` | Native constructor accepts optional map and optional pointer to derived defaults. An engaged empty map remains empty. Otherwise it copies injected defaults or the current base `defaultNodes`; initial own-key order follows JS `Object.keys` integer-index ordering, then string insertion order. Duplicate native entries preserve first position and last value. |
| copy/move construction and assignment | Deleted. Creator callbacks borrow the factory address, and the factory owns the product session; moving or copying would break this lifetime relationship. |
| `create(kind, ...args)` | Looks up the requested creator object property, then uses the current `defaultKind` property only when the requested value is absent or null (falsey). It invokes the captured callable, never consulting `nodeMap_`; a non-null creator with an empty call throws without fallback. Missing and non-callable creators throw `FactoryError("TypeError", ...)`. Inherited Object-prototype names are rejected as invalid typed products rather than silently routed through the default. |
| `setNodeClass(kind, nodeClass)` | Writes the class record to `nodeMap_` and a fresh creator record to `node_`. The creator closes over that class record and borrowed factory, so saved creator refs keep their original class after replacement/deletion. The `__proto__` write uses the prototype setter only when no own creator entry exists; own-entry replacement is ordinary. |
| `getNodeClass(kind)` | Returns the current own Map class record by shared identity, or an empty reference if absent. It does not expose inherited creator-object properties. |
| `deleteNodeClass(kind)` | Removes the Map entry and same-name own creator entry. For `__proto__`, a creator previously assigned through the inherited prototype setter remains retained. |
| `nodeIsKind(node, kind)` | Resolves the current registered class and invokes only its native instance predicate (RTTI/dynamic-cast supplied by that class); it does not require or invoke the constructor callback. Null product pointers return false when the predicate is valid; a missing or invalid right-hand class throws TypeError. |
| `getKinds()` | Returns Map keys in insertion order, including replacement stability and delete/reinsert movement. |
| `adopt(unique_ptr<Product>)` | Native lifetime service used by constructors: stores complete or partially initialized products in the factory/session arena and returns a stable borrowed pointer. Null products throw TypeError. |
| protected `createDirect(kind, args)` | Invokes only the requested creator-object property (including a retained `__proto__` prototype creator), with no `defaultKind` fallback and no Map lookup. Null/missing/non-callable creator values throw TypeError. This is the bridge required by the separately owned NodeFactory implementation. |

## Exact source adaptations

The C++ base constructor cannot read a derived class's static `defaultNodes`
during base construction, so the frozen API injects a pointer to derived
defaults. The value is copied at construction and does not alter existing
factories. The generic C++ static map is per `Product` family; erased JavaScript
class records shared across incompatible product types are outside the raw-TeX
profile.

Creators receive the original argument span synchronously and may not retain
the span. `FactoryClass::construct` must call `adopt` before virtual
initialization; adoption is the creator's lifecycle contract and the factory
does not add a separate returned-pointer ownership check. This preserves
products if later initialization throws and avoids factory/product
shared-ownership cycles. Construction validates only the presence of the
constructor callback; `nativeType` remains metadata. Replacing a class record
affects later `getNodeClass` and `nodeIsKind` lookups, while existing creator
records and product identity remain unchanged. Direct mutation of `nodeMap_`
does not rebuild captured creators; direct mutation of the creator view does
not change `getNodeClass` or `getKinds`.

JavaScript creator-object inherited built-ins can return values that do not
conform to the native product type. The frozen raw-TeX contract maps these
invalid products to TypeError; it does not fabricate nodes or fall through to a
default creator. Construction requires a class record with a constructor
callback; `nodeIsKind` requires a class record with an instance predicate.
`nativeType` is retained as class metadata and is not a validity test.

When the `__proto__` creator has been assigned, missing own keys `call`,
`apply`, `bind`, `arguments`, and `caller` resolve through the assigned arrow
function's `Function.prototype`. The first three are truthy inherited
functions that are invalid native products; the latter two are throwing
accessors. Both cases raise TypeError without default fallback. Own entries
are checked first, so own null values still trigger generic fallback. The
view does not attempt to model arbitrary JavaScript prototype chains.

The view and captured callbacks borrow the factory lifetime. Although the
frozen representation uses a shared handle for stable getter identity, callers
must not retain or invoke that view after the factory/session is destroyed.
`AbstractFactory` is explicitly non-copyable and non-movable so creator
callbacks always refer to the session owner that created them.

The implementation unit covers only generic `Factory.ts`. `NodeFactory.ts`
and `WrapperFactory.ts` are separate bridge units owned by the parser agent;
their argument normalization and wrapper forwarding are not claimed here. This
packet does not implement Node, Wrapper, MML, or rendering behavior. No source
contract, generated oracle fixture, build file, or other owned file was edited.

Root owns compilation, frozen-oracle execution, mutation checks, and acceptance;
this evidence does not claim those checks have passed.
