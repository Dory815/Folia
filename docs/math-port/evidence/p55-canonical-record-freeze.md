# P55 canonical configuration-record boundary freeze

Source Configuration owns ordinary Context handler/fallback/node records.
Existing SubHandlers and NodeFactory operate on their actual native registries,
but their input methods currently accept Property records. Copying complete
Configuration records into Property dictionaries would create a second owner
and move getter evaluation ahead of Source operation order. Root instead adds
typed Context input overloads to these actual classes, retaining the one
semantic registry and the same canonical FunctionRef identities.

Responsibilities and exclusive body assignments:

- map_handler.cpp: Context overloads for SubHandlers/SubHandler add and remove;
  existing Property input bindings and Context input bindings share class-owned
  addMapIterator/removeMapIterator/addHandlerNames/removeHandlerNames reducers.
  No new configuration graph or Source parser class is introduced.
- node_factory.cpp: Context setCreators reads the original record live, resolves
  the existing canonical callable and calls the actual virtual set method.
  Both input bindings share installCreators; the factory registry remains its
  actual existing owner. Context for-in includes prototype keys, non-enumerable
  shadowing and deleted-slot checks, without reading values ahead of iteration.
- tags.cpp: complete actual Label and TagInfo record constructors, including all
  eight original TagInfo parameters/defaults. The remaining Tags bodies are not
  implemented or accepted by this stage.

Root owns interfaces, original oracle, tests, build and acceptance. Luna may
edit only its assigned implementation unit, necessary private local helpers and
the corresponding existing adapter methods to share the one reducer. No data,
gold, interface, build, other Source class or test edits; no agent build/commit.

Required read order: SubHandlers snapshots Object.keys(handlers), performs
actual get/set handler dispatch, then Gets handlers[name] and fallbacks[name]
in argument order. Only after both Gets may SubHandler copy the names array,
as Source maps.slice().reverse() explicitly requires. A fallback getter may
mutate the same names array. Add uses a length-captured source copy; remove
iterates the actual array with live length/Get. Missing handlers during remove
skip both argument Gets. Missing token maps abort before registering fallback.

Native profiles are ordinary Context arrays/records with accessor effects and
canonical bound native callables, retaining UTF-16 primitive strings through the
existing lossless Property scalar conversion. Arbitrary JS execution, overridden
array iterator/slice protocols and unbound object-to-primitive evaluation are
outside the native host profile. They must never be silently emulated by string
special cases. Records and nested arrays are borrowed, not authoritative copies.
Only Source-mandated array copies and computed native registry entries are owned.

The Root oracle executes actual hash-verified pinned MapHandler, CharacterMap,
NodeFactory, Label and TagInfo classes. All 21 observations are frozen before
implementation in p55-parser-record-source.json and reference_parser_records.cpp.
It checks getter order, inherited/absent handler keys, array copy/iteration timing,
fallback identity/removal, warning short-circuit, live/deleted creator properties,
prototype shadowing, virtual set, function/scalar identities and all record fields.
No fake TeX parser/jax/reducer is used. Native tests instantiate the actual token
maps and factories; protected-state inspection/virtual set tracing are test probes.

The normal core target compiles these interfaces and fixtures. The frozen stubs
execute and fail the 21 assertions through Unimplemented (build/p55-parser-record-
stub-red.log). After implementation: original baseline, independent review,
compiling semantic faults, restored baseline and full core verification must pass.
This is necessary parser-family plumbing, not complete TeX/package/rendering
acceptance. The main Configuration/ParserConfiguration/Tags/ColumnParser bodies
still require their full original operation-order and lifetime contracts.

Root review then added two independent original for-in observations. Deleting
an own property can reveal an inherited property of the same name, so checking
only the original owner's hasOwn is wrong; a new own non-enumerable shadow may
also supply the value for an already-enumerated inherited name. The expanded
23-case original fixture fails the initial native body before correction
(build/p55-node-prototype-red.log, exit 1). These are actual pinned Node/Source
observations, not expected values adjusted to the native body. TokenMap Source
and CJS hashes and scope.json guards are now included explicitly.

Root also froze argument-Get order and remove's length-shrink barrier, plus an
actual slice-hole witness: a later source getter adds an indexed Array.prototype
value, which the new slice array's hole observes during reverse. Filling holes
with Undefined in a vector loses that behavior. The 26-case expanded fixture
fails the initial body before correction (build/p55-slice-hole-red.log, exit 1).
The Context binding must create a real temporary ordinary slice array, use
CreateDataProperty for present copied slots, perform reverse Has/Get/Set/Delete
in Source order, then iterate its live length/Get while the one addMapIterator
reducer registers each map. Property inputs supply their existing equivalent
native snapshot iterator. This is a Source-mandated temporary, not a second
authoritative Configuration record. Keep missing-map short-circuit before
reading any later copied-array slot.

The 27th original observation exposes reverse operation order by retaining its
actual getter receiver. Lower Get deletes the upper property before Upper Has;
performing both Has checks before either Get incorrectly creates an own
Undefined lower slot. The initial body fails this original observation
(build/p55-reverse-order-red.log, exit 1). Reverse must perform Lower Has/Get,
then Upper Has/Get, then the appropriate Set/Delete branch.
