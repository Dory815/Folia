# P49A actual native payload identities in the Context graph

Root owns this host binding; it implements no MathJax parser algorithm. One
NativeIdentityBindings<T> per native reference domain/runtime attaches actual
typed owners to actual Context objects and keeps a canonical identity index.
It creates no math tree, factory registry, class metadata, option dictionary,
Source array snapshot or callback body. There is no synchronized second data
representation: Context holds dictionaries/arrays, and payloads refer to actual
native owners. Math remains portable C++23.

bind associates an actual existing Context constructor/object with its native
reference. instance creates an object of an actual supplied Context constructor
only when that native identity has no existing binding. Repeated binding returns
the canonical handle; resolve validates payload type and canonical registration.
Foreign contexts, null native identities, occupied payloads and unrelated/unbound
object references are explicit host errors, rather than fake values. Native
shared descriptors retain their actual shared owners in the Context payload.
Raw MML node pointers retain a shared reference to their actual owning factory
graph in the same typed payload; unanchored pointers are rejected. The Context
payload retains that graph even when the registry object or caller's shared
factory reference is destroyed. The pointer itself never pretends to own a node.
clear explicitly releases this domain's bound payloads only at runtime/domain
teardown after Source consumers have released the references. Stale Context
handles fail typed resolution rather than projecting freed native pointers.
Registry indexes must not be used after that teardown. Context outlives registry
operations. Shared descriptors already retain their owner through identity.

Configuration classes/tags/factories still belong to their existing or forthcoming
actual owners. Their constructor objects need actual class-property bindings;
this utility does not manufacture a Source class implementation or mirror class
statics. Functions keep P47's converter-only canonical FunctionRef binding.

Root's original observations and native tests must exercise actual original
Options/defaultOptions and real MML nodes/class descriptors for alias retention,
copy boundaries, dictionary enumeration and node-list-shaped arrays. Host error
and lifetime tests are additional native contract tests, not MathJax semantic
coverage. This is necessary plumbing for P49's declaration graph, not a parser,
general JavaScript evaluator, or complete native TeX/SVG acceptance.
