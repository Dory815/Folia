# Root contract: shared Source Factory Context module

Status: implementation and runtime acceptance pending. This contract does not
accept the current MML or Common factory bindings.

The pinned MathJax 4.1.3 CJS sources define one AbstractFactory class in the
core/Tree/Factory module. AbstractNodeFactory and AbstractWrapperFactory derive
from that same class. MmlFactory derives from AbstractNodeFactory; Common
factory classes derive through AbstractWrapperFactory.

| File under the pinned cjs/core/Tree directory | SHA-256 |
| --- | --- |
| Factory.js | 739db2e5760c29d7bb4f1bc7baa08ff8d727f4d117522aff5ede017f812cffef |
| NodeFactory.js | 148c9a3a2e469fc464036b88deb7067cd779989a43356812d31365a84ec75c54 |
| WrapperFactory.js | bbeb7c85e0dfd3c8e437d500d37f083a6b031cd9490b58d41c3285e8f7f679d5 |

The shared non-template Source module owns exactly one AbstractFactory
constructor, prototype and generic method table per RuntimeModules owner.
RuntimeModules is explicitly shared by parser and output services in the same
render service graph. It is not a template static or a process-wide cache.

The generic methods consume live receiver fields and preserve the emitted
Get, Set, ToPropertyKey, Call, construct, enumeration and iteration sequence.
An ordinary member invocation uses Context::call; a Source explicit .call or
.apply reads that property at its actual Source evaluation point. The module
must not duplicate generic methods in MML and Common adapters.

Product-specific FactoryContextBindings adapt native allocation, owner lookup,
field projection and instanceof. A derived receiver resolves to the existing
base subobject of its canonical shared owner. An adapter does not replace the
payload, create another identity registry or retain a second copy of nodeMap,
node or defaultKind. Generic Source construction initializes ordinary Context
fields and does not select a template product owner. A typed product adapter
must resolve or allocate its canonical owner independently of which adapter
was registered last. Ordinary AbstractFactory.call preserves unrelated native
payloads while performing the Source field assignments.

MML-specific Context methods own only the AbstractNodeFactory create override
and MML-specific members. Its prototype links to the canonical AbstractFactory
prototype. Common's AbstractWrapperFactory links to that same prototype and
contributes only its own Source wrapper behavior. Native-to-Context argument
conversion does not change the shared MML initializer algorithm.

Acceptance requires pinned CJS witnesses and native tests for complete
constructor/prototype identity and inheritance across MML/Common; new and
call on both generic and already-bound derived receivers; live replacement of
nodeMap/node/methods; ordinary invocation with a replaced function.call;
Symbol keys; getter/setter order and partial state after exceptions; and the
existing canonical-owner/lifetime tests. The actual public SVG, layout and
Renderer lifetime evaluations must subsequently pass. A local method test or
equal pairs of rendering errors does not satisfy renderer acceptance.
