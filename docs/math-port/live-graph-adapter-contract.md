# Live native graph adapter responsibilities

Root owns these shared interfaces and their acceptance. Package owners continue
Source algorithms and closed typed native fixtures; they do not snapshot native
objects into Context dictionaries or replace missing peers.

The implemented `ContextPropertyBindings` is a view of existing Context records
and arrays. Its current focused checkpoint is recorded in
`evidence/context-property-binding-checkpoint.md`. Reverse exposure of records
created natively is still required.

Next shared dependencies:

- Native ordinary Property records must expose their actual field storage and
  prototype as a live Context record view. Enumeration, descriptor reads,
  assignment and deletion must use that same storage. Cached keys/values are
  not an authority. This includes BaseItem subclass static error records.
- Native class own data fields must read and write their actual owner through
  a generic record binding. Source own-field order and data-descriptor flags
  must stay observable; internal binding readers are not JS accessor getters.
  Install fields at their Source initialization phase. Do not invent a second
  C++/Context state to synchronize. Writable Source data fields retain the
  assigned raw value. Perform conversion and report errors when the Source
  algorithm consumes the field, preserving its actual operation order; do not
  reject an assignment merely because a later typed operation cannot use it.
- Factory, MathItem, MathDocument, MML node and child-array Context identities
  require their genuine native owners. Raw node pointers must retain the actual
  factory/graph lifetime. Constructors retain their captured actual class
  descriptors rather than guessing from a current kind registry. No no-op
  deleters, substitute factories or copied nodes are permitted.
- Direct BaseItems constructor calls resolve the supplied actual factory,
  adopt the real product, bind its constructor/owner before initialization,
  then run existing BaseItem and derived Source phases. They share the typed
  factory algorithm and canonical class descriptor.
- ParserRuntime owns one actual async JobQueue for its loaded runtime. Its
  `queue()` accessor returns that QueueRef. Typed retries use the
  existing native Promise service; the render/session owner drives the queue.
  Promise/error Context reflection remains a separate explicit dependency.

Required shared tests cover aliases, nested/cyclic fields, write-through,
deletion/redefinition and prototype changes within the supported profile,
descriptor/field order, actual constructor identity, lifetime retention,
invalid values and partial failures. Source-gold comparisons and real semantic
faults must accompany acceptance. A metadata or compile checkpoint alone does
not accept parser behavior, layout, SVG, or Folia integration.
