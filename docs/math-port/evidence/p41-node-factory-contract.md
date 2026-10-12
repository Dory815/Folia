# P41 TeX NodeFactory contract

Source: MathJax 4.1.3 NodeFactory.ts, pinned SHA-256
bf20e8d4f7ed43450df42d65556ccbceea1469541b214c37544f180aa205c840.
This factory is distinct from the MML constructor registry. It creates the same
existing MML graph and notifies ParseOptions; no new AST or compatibility path.

Root owns node_factory.ixx, node_factory_host.ixx, node_factory_bindings.cpp,
CMake, expectations, tests and acceptance. Luna mml_row_mi owns only the nine
semantic method bodies in node_factory.cpp after the compiled stub fails against
the original. Other agents review only. No agent edits interfaces, hosts, tests,
builds or expected data, and no agent accepts its own work.

## Allocation, services and arguments

NodeFactory itself is the canonical NativeObject shared identity. ::make models
Source allocation; subclasses also require shared ownership before invocation.
Default creator FunctionRef identities are shared across factory instances, as
Source static function values are. Calls use the canonical FunctionBody and
FunctionArgs; no second NodeFactoryMethod identity/closure type.

The TeX/session owns MmlFactory and ParseOptions across all parser/package
callbacks. ParseOptions owns NodeFactory; NodeFactory's configuration and
MmlFactory references are borrowed. Configuration begins Missing (undefined),
MmlFactory begins Null. These are separate variant states; no dummy service is
installed. Future ParseOptions implements the narrow addNode interface and
stores the authoritative node lists itself. Callback products remain owned by
the MML factory. Factories, callbacks and nodes cannot outlive that session.

Canonical scalar callback slots are Property (strings use lossless WTF-8);
children are the existing Children handle; optional text/result nodes are
NodeValue or MmlNode*. The first callback argument is the same NativeObjectRef
factory identity; the function receiver is Missing. Raw rest length is retained,
including the required undefined rest[0] when no arguments were provided.
Default adapters supply fresh empty children and Missing scalar slots, leaving
explicit Null untouched. Static createToken remains callable by Physics with
the same creator function identity available through get/set.

Enabled package calls use node/token/text/error dispatch and string text values.
Physics gets, replaces and restores token; configuration maps are data records.
The registry is the existing shared PropertyList with ordinary prototype lookup
and assignment. get returns raw Property identity, including inherited builtins;
truthy non-callable data entries fail only when invoked. setCreators includes
enumerable prototype data keys and reads each current value at visitation, not
just own keys or an early key/value snapshot. Accessor descriptors, FunctionRef
as __proto__, arbitrary builtin creator execution returning non-MML products,
and concurrently mutated key/prototype shapes are not expressible in this typed
package-map profile. They are not requirements of the public TeX-string API;
this packet does not claim whole-JavaScript object reflection parity.

## Nine semantic methods

- createNode: default only Missing definition; create(kind, zero constructor
  args), setChildren always, append truthy text, NodeUtil.setProperties last.
  Preserve prior writes when a later method throws; ignore setter returns.
- createToken: defaults only Missing definition/text; create text first, then
  node(kind, fresh [], definition, exact returned text). Preserve Null products.
- createText: nullish text returns Null before touching MmlFactory; otherwise
  create text and return the virtual setText result exactly.
- createError: text -> mtext -> merror; nested create registration is retained,
  including nullable products and original message in data-mjx-error.
- setMmlFactory/set: assign without premature validation.
- setCreators: iterate inherited data keys, virtual this.set with current value.
- create: get requested entry, truthy fallback to node, extract callback, call
  with receiver Missing and exact args. On successful return only, requested
  dispatch exactly node registers original rest[0] and result (even Null).
  Read the current configuration after callback completion; return result after
  registration. No registration follows a thrown creator.
- get: raw lookup, no fallback or callable normalization.

## Acceptance

Execute pinned original TS/CJS loader observations before releasing bodies.
Compare actual MML graph/products, callback identity and argument order,
Null/Missing defaults, constructor argument count, method/callback ordering,
live property values, shared data-map prototypes, nullable setter results and
partial errors. Compile semantic faults that distinguish these responsibilities,
then run the complete normal core build/tests. TeXParser, packages, mathematical
layout, fonts, shared scene, SVG and Folia integration remain open. Mermaid stays
paused throughout this work.
