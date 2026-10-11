# P23f: actual MML constructors and child construction

Root owns this contract, Source observers, fixtures, API boundaries, builds and acceptance. This packet does not certify the entire MML family, TeX, layout, SVG or Folia integration.

## Architecture and ownership

The sole node storage is `tree::ChildArray<MmlNode>` and the sole parent storage is the generic node's tagged `NodeValue`. Undefined, Null, holes, inherited slots and array identity remain distinct. Do not add vectors, raw-parent mirrors, implicit pointer conversions, fallback implementations or a second factory/registry. The portable module boundaries and the existing original class hierarchy remain in place.

`mml_node.cpp` owns AbstractMmlNode construction and its child policy. Generic Node owns actual array mutation and parent assignment. `class_binding.ixx` owns adoption and actual constructor-class binding; `mml.cpp` owns the original registry; `Attributes` owns attribute layering. This packet must not move these responsibilities.

The definition freeze updates incompatible signatures and places actual throwing stubs at unported tree consumers. They are deliberately unusable and must not enter Folia. Initial compile attempts exposed additional private mfenced/munderover helpers and bare-parent consumers; Root froze these too, rather than introducing dense array compatibility. The empty mfenced separator binding now allocates the sole ChildArray representation, and empty-node Parent reads the generic parent accessor directly.

## Authorized Luna implementation

Only the bodies `AbstractMmlNode::initializeMml`, `AbstractMmlNode::setChildren`, and `AbstractMmlNode::appendChild` in `src/math/core/mml_tree/mml_node.cpp`, plus the existing private `child_at` required-dereference helper in that file. The helper must take the exact Source member name; indexed Get remains tagged until the actual dereference. No other method, declaration, test, golden value, build script, manifest or source module may change. The agent may write its findings in `p23f-constructor-implementation.md` in this directory. No agent build, commit or delegation.

Compare `.reference/MathJax-src/ts/core/MmlTree/MmlNode.ts` constructor and lines 595–633, and pinned CJS. Root's loaded Source pins are embedded in `p23f-constructor-source.json` (MathJax 4.1.3, Node 24.13.0). Do not treat prior native code as specification.

- Initialize prevClass, prevLevel and texclass to Null before callbacks, as the actual CJS constructor does. Negative arity creates the inferred row through the same factory, installs a new array, then writes its parent. Call virtual setChildren even for empty children, before creating Attributes. Resolve registry defaults for the live kind, then math defaults in Source order. Apply scalar own properties with Source key order; Object.assign ignores Null/Undefined properties inputs. Preserve the native bounded FactoryArgs input contract; unsupported native types need explicit errors, never guessed conversion.
- Negative-arity setChildren dereferences indexed Get of child zero at member `setChildren`, then calls that node virtually. Otherwise invoke Generic::setChildren, which owns for-of's captured array object/live length and dispatches virtual appendChild.
- Negative-arity append dereferences child zero at member `appendChild`, delegates, then returns the original tagged argument. Otherwise read `child.isInferred` before any array mutation: Null/Undefined must throw at that exact read.
- An inferred child with infinite parent arity flattens the captured original child array using its forEach captured initial length and live Has/Get. Holes are skipped; own Undefined/Null and prototype slots are visited. Parent setter callbacks can mutate/rebind arrays. The append callback calls Generic::appendChild explicitly, preserving the original super call and post-write failure ordering. Return the original inferred child.
- A finite-arity inferred child becomes a factory-created mrow. Call its virtual setChildren on the original live child-array handle, alias its Attributes, capture getPropertyNames and obtain each property live before setting it. Then return Generic::appendChild's tagged result. Do not clone attributes or flatten this branch.
- Retain existing recursion/resource guards when needed; never turn a tagged Get into falsey Null early or provide defaults masking Source errors.

## Root red evidence and acceptance

`node tools/math-port/mml-constructor-oracle.cjs --check` executes the original full registry: 88 cases, 19 errors. SHA256 `1bc4f06019829ae17175d0310160265982267df4686ee184c926c441d92b10c5`. `freeze-mml-constructor.cjs --check` reproduces native inputs and expected strings strictly from those original results.

`node tools/math-port/mml-constructor-red.cjs` builds all 89 explicit math module interfaces and all production implementation units from CMake, including the real registry and factory. It compiles and executes 2 tests / 91 assertions: 88 fail, including actual constructor/setChildren/appendChild entry points reporting Unimplemented; 3 coincidentally pass and are not accepted semantic behavior. This is an assertion failure, not a compile failure or crash. Logs and red-result.json live in `build/math-port-mml-constructor-red`.

The Source observer registers completed factory products after construction, discovers retained failed outer objects through actual parent identity, and records tagged slots, presence, key order, attributes and virtual setChildren callback order. Attribute presence is Boolean plus explicit scalar contents; it does not certify Undefined versus Null attribute-handle identity.

Root will build and independently compare all 88 cases, then require effective compiling semantic mutations. These constructor cases do not certify inferred append, copy, inheritance or predicates: the existing Source 299/return32 corpora must subsequently bind to actual native classes and pass targeted adversarial callback cases. Whole-file and whole-port tallies do not advance on constructor-only acceptance. Folia remains disconnected until full SVG/native rendering acceptance.
