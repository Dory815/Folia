# P42 parser stack boundary plan

This is the remaining Source audit and design frontier. P42A has accepted the
MmlStack responsibility only; see p42-mml-stack-acceptance.md. P41 has accepted
NodeFactory. Root must freeze the remaining interfaces and actual-original
observations before assigning BaseItem/StackItemFactory/Stack bodies to Luna.

## Source responsibilities

- StackItem.ts: NodeStack interface; MmlStack owns each item's MML node list;
  StackItem interface; BaseItem supplies properties/environment, checkItem,
  error messages and LaTeX attributes; StackItemClass supplies factory metadata.
- StackItemFactory.ts: generic AbstractFactory specialization and explicit
  null configuration edge; DummyItem/default metadata. It is a separate creator
  registry with StackItem products, not the MML constructor registry.
- Stack.ts: the parser's stack of StackItem identities, reduction through
  checkItem, environment copy/alias propagation, Top/Prev/Pop and height.
- base/BaseItems.ts: all concrete reducers, including Start/Stop/Open/Close,
  prime/scripts/fraction/fences/style/function/negation/dots, tables and equation
  arrays. ArrayItem additionally reads/writes the real parser string/index and
  uses the real tags, ParseUtil and UnitUtil services. Do not replace reducers
  with a custom parse tree or introduce a second ParseOptions.

There are distinct Source containers: per-item MmlStack node lists, the Stack
item list, and the existing MML graph's child arrays. All entries reference the
same MML/StackItem objects; the node lists are not substitute graph storage.

## Native interfaces requiring decisions

Existing types.ixx owns StackItemContext/StackItemRef, used by ParseAtom and
ParseResult. Extend or relocate that one identity boundary coherently; do not
create an old/new parallel stack-item model. Generic FactoryServices currently
adopts unique products and returns borrowed pointers, while StackItemRef is a
shared identity. Freeze one explicit ownership bridge before implementation:
the session owns products and all back-edges are borrowed; callback-held refs
must point at those exact objects, never at reconstructed facade objects.

Preserve Source constructor phases explicitly, as in the existing MML factory
bindings. BaseItem's constructor invokes virtual isOpen before derived field
initializers. A C++ base constructor cannot dispatch that call to the derived
getter. Do not translate it mechanically into a constructor that always sees
BaseItem.isOpen=false. Allocate/adopt first, capture actual class metadata, then
run the Source initialization phases with the right raw fields and virtual
dispatch. Derived static errors and shared success/fail tuple identity also
belong to the actual class binding, not a guessed kind-name switch.

NodeStack methods and StackItem interface methods must retain virtual dispatch.
MmlStack owns startStr/startI/stopI and the one node-list handle. Its constructor
retains that handle, Clear rebinds, Peek slices, and empty Pop/read operations
return Missing. Empty First assignment writes the non-index array property -1;
it does not allocate an enormous native index or increase length. Preserve this
through a semantic array-property adaptation, not a text-specific exception.

Stack props differ from MML scalar attributes: Source Prop includes MmlNode and
nested PropList. Freeze a typed value carrier that retains NodeValue identity
and shared property objects; do not stringify nodes or duplicate their graph.
Environment/global are ordinary shared scalar property records. BaseItem's env
begins Missing unless its virtual isOpen makes a new object during construction.
StackItemFactory.configuration begins explicit Null. Item.factory and
ArrayItem.parser point back into the same session-owned services.

CheckType is a tuple whose replacement-list slot can be Null; BaseItem.fail and
success are shared static tuples, and SubsupItem.checkItem can itself be null.
Stack.Push must preserve falsey skipping, reduction ordering, recursive result
processing, null-item suppression and environment propagation. Stack.Pop deletes
env only for non-open items and resets current env to the next item's exact
record or a fresh empty record. Use no generic tree-editing algorithm here.

## Verified default-registry trap

Original CJS observation: StackItemFactory.DefaultStackItems has one key base,
because DummyItem inherits BaseItem.kind. The defaultKind instance field is
dummy. However new StackItemFactory().getKinds() is empty: inherited Factory
initialization consumes constructor.defaultNodes, not DefaultStackItems. With
no explicit item classes, create(base), create(dummy) and create(unknown) all
throw TypeError Cannot read properties of undefined (reading 'apply'). The
enabled parser supplies its configured item classes explicitly. Do not silently
auto-register dummy/base according to the comments or rename the static field.

## Integration sequence

Next Root interface decisions, audited read-only after P42A:

- Keep stack_services.ixx as the borrowed configuration/NodeFactory leaf. When
  adding BaseItem, move StackItemFactoryService/StackFactoryReference to their
  Source-owner stack_item module, where BaseItem can be forward-declared without
  declaring the same entity in two named modules.
- One concrete StackItemFactory inherits AbstractFactory<BaseItem> and a narrow
  independent service interface. That interface must not also inherit generic
  FactoryServices (no diamond). Its borrowed itemFactory() accessor returns the
  same concrete factory, not another registry or wrapper. Existing MmlStack
  create always uses configuration.nodeFactory; reducers' this.factory.create
  uses the StackItem constructor registry. Never switch by a guessed kind string.
- Override adopt to retain one shared BaseItem identity in the factory pool;
  reference(raw) returns that same existing StackItemContext shared identity.
  Every class binding explicitly adopts before Source constructor phases.
- Capture actual class static errors metadata, including inherited live error
  arrays; do not read metadata from a later kind registration. Run virtual isOpen
  during the Source BaseItem phase with derived fields still Missing, rather
  than in a C++ base constructor or after native derived field initialization.
- StackProperty must contain canonical scalar Property, existing NodeValue and
  recursive shared records, without reversing the generic Property -> MML
  dependency. Retain existing scalar PropertyList identity for style/color;
  do not promote-by-copy and claim aliases survived. Freeze record mutation,
  Object.assign key/value order and actual package domains before releasing it.

Freeze Source service/identity/value interfaces and original boundary gold;
implement complete MmlStack/BaseItem and StackItemFactory responsibilities;
implement Stack and all concrete BaseItems with their dependencies. Complete
ParseOptions/configuration/tags/TeXParser using these same products and services,
then port enabled package methods and establish full TeX -> MML comparisons.
Mathematical fonts/common wrappers/shared scene/SVG and Folia replacement remain
required. Mermaid stays paused. No renderer acceptance follows from this plan.
