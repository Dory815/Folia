# Root contract: one MML algorithm over live receivers

The complete Tree/MML candidate remains unaccepted. Independent review of
0006 through 7D7C8C36 finds one recurring boundary error: Context methods
delegate to native typed methods before Source has required a native product.
Special copy/arity/attributes callbacks then reproduce only parts of the same
algorithm. Passing the fixed SVG profile does not prove this boundary correct.

## Ownership and dependency graph

`core/mml_tree/mml_node_algorithms.ixx` owns stateless, Source-aligned bodies
for setChildren, appendChild, replaceChild, copy and inferred-child conversion.
It layers on the candidate generic TreeNodeAlgorithms, ChildValue and live
ChildArray interfaces. Its templates must not import the completed MmlNode
class, avoiding a cycle with the native node module.

An operation view supplies primitive field Get/Set, calls, coercions,
iteration, property assignment and native-result projection. A native view
uses the current native owners and Property values; a Context view uses the
exact receiver Value and current descriptors. The view stores no field values
or second semantic tree. Each Source expression performs its read at that
point; the view must not pre-read or cache arity, kind, factory or attributes.

The private native operation carrier may include an exact raw Context Value
alongside Property, ChildValue and selected-callable tokens. This is a finite
carrier for foreign primitive Get/call results and preserves their identity;
it does not expand the shared Property API with std::any or own another set
of fields. A Source callback which may be retained must capture its receiver
view by value, never a reference to the current operation's stack frame.

MmlNodeContextFields remains the sole field bridge for adopted products.
Class descriptors own allocation of exactly one real native product and its
actual constructor identity. Neither allocator nor method entry may require
the mutable Source factory field to already be a canonical native factory.
An unadopted ordinary `.call` receiver must reach Source operations in their
original order, preserving its descriptors, prototype and partial writes.

MmlNodeContextMethods installs prototype methods/getters and creates operation
views. It does not select a second MML algorithm for native-bound receivers.
Both native methods and Context entry points invoke MmlNodeAlgorithms.
Results remain ChildValue/raw Value until a caller actually requires a native
MmlNode. Context copy returns an arbitrary factory result unchanged. The
existing typed MmlNode* API checks/projects only after the shared algorithm.

Remove sourceCopyCreator, sourceCopyProperties, sourceCopyAttributes,
sourceInferredReplacement and sourceOperationArity callbacks from the
candidate AbstractMmlNode. Primitive operation hooks are acceptable; separate
implementations of whole MML operations are not.

## Source order and acceptance

The pinned reference remains MathJax 4.1.3 commit
`fb987178c2d279a99b0db5ea02e751691703955e`, including emitted CJS semantics.
The independent witness is currently
`tools/math-port/drafts/g176-core-tree-seam-review/mml-call-source-witness.cjs`;
Root must freeze its reviewed bytes before the shared build.

Required observations include:

- `factory` and its `create` member are read before the `kind` argument in
  copy. Foreign factories and ordinary returned objects retain identity.
- Ordinary method calls use abstract Call. Read a function's `.call` only
  where the emitted helper explicitly does so.
- Arity and child isInferred getters run at every actual Source read, including
  two changing arity reads in inferred append.
- Inferred conversion uses the current factory and replacement methods, and
  assigns the original attributes object exactly rather than copying it.
- Raw child length uses Source truthiness and strict equality. Copy uses its
  live iterator, preserves falsy slots where Source does, and handles foreign
  children with their current copy/forEach methods.
- Properties assignment follows the actual Object.assign key snapshot,
  live descriptor/enumerability, Get and Set sequence, including Symbols and
  nullish sources. Foreign attributes use their current methods at the actual
  conditional/loop points.
- Constructors preserve new/call behavior, inferred rows, static defaults,
  native ownership and every existing descriptor/error-order test.

Root owns the formal architecture, source integration and shared build.
Luna owns candidate implementation and its method/source mapping; the
independent reviewer checks exact hashes and reachable witnesses. Candidate
patches must preserve the current request-owner retirement and shared
argument-phase changes. No Source capture/comparer or unrelated work may be
replaced. Acceptance requires the focused native observations, full existing
tests and independent SVG/Scene comparisons, not just an apply check.
