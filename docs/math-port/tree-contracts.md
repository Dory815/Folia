# P7 generic Node and Wrapper contract, root revision 3

Pinned MathJax 4.1.3 commit fb987178c2d279a99b0db5ea02e751691703955e:
Node.ts SHA256 532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0;
Wrapper.ts SHA256 8729a810aabd6eb3ad22021464f2f27d86116fae9edc0f818527f82e517bf7d9.
This packet covers every source field/method of those two files. MmlNode,
operators, fonts, parser, Common wrappers and rendering are not authorized.

## Native hierarchy and allocation

Root owns node_types.ixx and wrapper_types.ixx. Node<Product> is the typed
interface/storage bridge: one factory reference, parent pointer and shared
mutable child-array handle. AbstractNode<Product,Base=Node<Product>> derives
Base. Later MmlNode can extend Node<MmlNode> with its real semantic interface,
then both AbstractMmlNode and AbstractMmlEmptyNode use Base=MmlNode. This allows
both source inheritance branches to implement one family without a parallel AST
or duplicated tree fields. Base must derive Node<Product> and accept its factory.
AbstractEmptyNode<Product,Base> derives AbstractNode<Product,Base>.

Native constructors initialize storage only. Creators allocate the complete
derived object, services.adopt it, then call initializeNode(FactoryArgs args={}).
That method maps the source constructor's executable body: own ordered property
keys through virtual setProperty, then virtual setChildren only if input children
is nonempty. It accepts two optional PropertyList/FactoryChildren args; absent
or empty any means fresh defaults. Explicit null/invalid input raises TypeError
at its source phase. Extra args are ignored. No virtual source initialization in
a C++ base constructor. Future MML initialization calls the generic phase first,
then its own source phases, after full allocation/adoption.

Node and Wrapper storage objects are non-copyable/non-movable. References borrow
their owning factory/session; children/parents are borrowed pointers. Child arrays
are shared native handles matching JS array identity; replacement creates a new
handle. Do not detach old parents, enforce acyclicity, eliminate duplicates,
compact nulls, cache traversals or normalize malformed states.

## node.ixx exports and methods

Export-import node_types, import std, namespace folia::math::tree. Export the
two templates above, inheriting Base's constructor via an explicit storage-only
constructor. Protected PropertyList properties is fresh for each object. All
interface methods override virtual declarations, enabling later MML overrides.

- kind returns unknown. setProperty uses properties.set; getProperty uses get
  (inherited values observable); names uses keys; all-properties returns the
  same live handle. removeProperty deletes own names in argument order.
- isKind delegates to the stored factory with this Product identity, not kind
  text. Interfaces require actual compiled instance predicates.
- setChildren replaces childNodes with a fresh empty array before iteration,
  then virtual appendChild for each value. Iteration captures the supplied handle
  and reads its current length and values; no snapshot. A null supplied handle
  throws after replacement. Old children's parent links remain unchanged.
- appendChild pushes before writing child.parent. Null is pushed and then throws
  TypeError; preserve the partial mutation. Return the supplied child.
- replaceChild locates first identity match. If present, replace the slot, set
  newChild.parent, then clear oldChild.parent only if it still points here.
  Return newChild even when no match. Same-pointer replacement clears its own
  parent. Null dereferences raise TypeError at their actual phase, after prior
  mutations. Never return oldChild merely because the interface comment says so.
- removeChild removes the first identity match, then clears that child's parent
  unconditionally, including a parent now pointing elsewhere. Return supplied
  child. A matched null is removed before TypeError; unmatched null returns null.
- childIndex returns optional first identity index, including a null slot;
  missing is nullopt. Null childNodes throws for indexed operations.
- copy(bool keepIds=false) maps generic copy; the boolean is ignored here,
  reserving MML's existing optional keepIds override without hiding dispatch.
  Create this.kind through the current factory with omitted args. Replace the
  new node's properties with a fresh Object.assign result: assign, not setOwn.
  The TS source spells object spread, but both pinned CJS (ES5 __assign) and
  MJS (ES6 Object.assign) lower it to assignment. Folia's require profile resolves
  to CJS. An own __proto__ source key can change the destination prototype or
  ignore a scalar instead of becoming an own data key. This compiler-output
  fidelity mapping does not permit reinterpretation of other TS behavior.
  Property values/handles remain shallow aliases. Iterate the captured source
  child-array with live length, skip nulls and virtual-copy/append each child.
  Null source childNodes is the source's falsey [] fallback. New nodes need the
  same AbstractNode<Product,Base> binding; reject an invalid native binding rather
  than make a custom AST. Factory rebinding, inherited overrides and abrupt
  child-copy mutations remain observable.
- findNodes invokes virtual walkTree, includes self, and asks each node's virtual
  isKind. Results are identity-preserving preorder pointers.
- walkTree invokes the callback on self before reading childNodes; capture that
  current array, then iterate live length and virtual child.walkTree, skipping
  nulls. Replacing this.childNodes during a child callback does not replace the
  captured iterator array. Preserve shared callback data and return its value.
  Null childNodes throws after the self callback.
- toString evaluates virtual kind first, then captures childNodes and its length
  once for Array.join(','). Child toString dispatch is virtual; later array
  replacement does not retarget join and appending does not extend its saved
  length. Each saved index reads the captured array's current value; removed or
  out-of-range entries and null are empty strings, retaining separators. Empty
  array gives unknown(). Null array throws after kind evaluation. Native UTF-8
  strings carry exact code points; no trimming/normalization.
  Array.join suppresses recursion into the same active array. After kind/null
  checks use root TreeJoinGuard<Product>(children.get()); if already active,
  return kind + "()" without converting children. This is stack-local array
  identity tracking, not tree normalization or a visited-node traversal cache.
  RAII releases the active join after return or abrupt child conversion.

AbstractEmptyNode overrides exactly setChildren (ignore), appendChild (return
argument), replaceChild (return oldChild), childIndex (always nullopt), walkTree
(self only) and toString (kind only). Other methods remain inherited, even if
callers directly attach children to an empty node. Do not add arity checks.

## wrapper.ixx exports and methods

Export-import wrapper_types. AbstractWrapper<Product,Source,Base=Wrapper<Product,Source>>
derives Base; storage-only constructor takes WrapperServices<Product,Source>&
and Source*. kind forwards node.kind and throws TypeError on null. node remains
mutable and borrowed. childNodes is optional: absent by default as the actual
pinned CJS constructor; a present null handle is different and throws on walk.

wrap(Source*,FactoryArgs args={}) is virtual but the base ignores args and calls
factory.wrap(source,{}) exactly as the concrete source method does. The factory
service itself supports extras; future CommonWrapper overrides consume parent.
Root adds WrapperServices to factory_types and makes WrapperFactory.wrap override
that service. No auto-wrap, cache, parent links, BBox or scene in this packet.

walkTree visits self first, then checks current childNodes presence; absent
stops, present null throws. Capture the current array and iterate live length,
skip nulls and virtual child.walkTree. Return the original data handle/value.

## Host callback/resource services

TreeVisitor<Product> receives Product* and const std::any& data synchronously.
Object data uses native shared service handles; primitive values copy by value.
No JS eval, proxies, accessors, arbitrary custom iterators or callback-language
runtime is introduced. PropertyList is the previously accepted scalar dictionary
host service; own scalar/spread and prototype identities remain required.

Recursive copy/walk/toString use root's TreeCallGuard<Product>, maximum 512
concurrent native calls, with RAII unwinding and RangeError before native stack
exhaustion. This is an explicit host resource adaptation, not a claim to the
VM's recursion capacity. Empty and wrapper recursion uses the same guard for
its product family. Future source overrides must share the guard. Root tests
the boundary and recovery; if 512 valid generic calls overflow Windows first,
the implementation is rejected and traversal representation must change.

## Exact ownership and acceptance

math_layout owns ONLY core/tree/node.ixx and evidence/p7-node.md.
tex_parser owns ONLY core/tree/wrapper.ixx and evidence/p7-wrapper.md.
math_fonts owns ONLY evidence/e10-tree-host-usage.md (read-only usage audit).
Root owns interfaces, services, fixtures/oracle, CMake, scope, acceptance, builds
and commits. No agent edits shared contracts, tests or another packet, builds,
commits, fills MML stubs or wires production. Report missing interfaces to root.

Root generates reference_tree.cpp from actual pinned JS before dispatch.
Expected states include parent/array/property identities, null/error partial
states, aliases, copy rebindings, constructor virtual dispatch, Empty overrides,
live traversal mutation, Wrapper absent/present children and dropped args.
Source-member review and compiling behavioral mutation detection are mandatory.
Native ownership/resource checks are separately identified from JS expectations.
