# P48 actual core InputJax owner, fields and gates

Source: pinned core/InputJax.ts, SHA256
99af0b13253a27a3c3db639f5cc6531580166aa4e569bc225fb7327257696948.

Root freezes two interface modules and two exclusive body files. InputJax extends
MathItem's existing compile service; that back-edge remains compile-only and does
not import the parser or duplicate jax state. AbstractInputJax stays abstract;
compile belongs to the eventual actual input subclass.

The fields options, preFilters, postFilters, adaptor and mmlFactory belong only to
this owner. options is the existing Context/Value graph, not PropertyList. Filters
are the actual FunctionList. MmlFactory is the actual borrowed factory. The base
only stores the complete opaque generic adaptor identity; DOM methods are not
claimed or implemented by that storage binding. Nullable references retain
Missing/Null. Proto results use existing scalar protoItem records or complete
opaque records with native node locations, with one fresh shared array per call.

Class NAME and OPTIONS exist only as properties on the actual Context constructor
object. InputJaxModule is loaded once per runtime/Context and retains that class
handle; it is not constructed once per jax. An instance retains its actual Context
instance handle, whose constructor property remains live. There is no cached NAME
or second class options table. Context owns the bound native instance payload;
Context and callable bindings outlive all retained objects. This follows P46's
single runtime/lifetime decision.

Root's factory allocates the full native subclass and binds it before executing
the explicit Source base constructor phase. Derived Source fields must execute
after that base phase in the eventual TeX factory. Native allocation constructors
must not prematurely initialize derived Source fields or call Source virtuals.
The base allocation initializes only Source adaptor/mmlFactory null defaults;
options and filter lists are absent until their Source assignments.

Exclusive implementation ownership:

- input_jax_constructor.cpp: Luna implements only initializeInputJax. Default
  supplied Undefined to fresh {}; capture instance.constructor once; evaluate
  fresh destination and captured CLASS.OPTIONS; existing defaultOptions then
  userOptions; assign options; build preFilters then postFilters from current
  options properties using P47's real FunctionList bridge. Exceptions propagate,
  preserving completed assignments. Never snapshot both filter arrays up front.
- input_jax.cpp: a separate Luna implements name, processStrings, setters,
  initialize/reset, findMath and executeFilters. name freshly reads current
  instance.constructor then NAME. Setters preserve complete provided identity;
  initialize/reset are no-ops; processStrings true; findMath fresh empty array.
  executeFilters allocates one shared mutable record with math/document/data,
  passes exactly that record as the sole FunctionArgs slot, ignores the returned
  boolean, and returns current data only on normal completion. No copied record,
  coercion, narrowed data type, catch, or second callback registry.
- Root owns declarations, class/allocation host, actual Source oracle, Native
  fixture, red baseline, review, builds, fault injection and acceptance. Agents
  cannot edit those files or build/test/commit.

The filter record uses existing MathItem/MathDocument reference identities and
heterogeneous std::any data. Mutable object inputs use retained owners, immutable
scalar inputs use scalar values; arrays/nodes/parsers are never deep-copied here.

This is a real generic InputJax prerequisite. The complete TeX parser/package
pipeline, native fonts, common layout, SVG parity and Folia integration still
remain necessary to achieve the unchanged goal. Mermaid remains paused.
