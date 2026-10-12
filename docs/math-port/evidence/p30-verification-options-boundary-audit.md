# P30 verification-options boundary audit

Read-only inventory of the current native options coordinate and the pinned source call semantics. This report identifies affected production and observer sites for Root's design work; it does not propose a replacement API.

## Pinned source behavior

Primary source is MathJax 4.1.3 `MmlNode.ts`, SHA-256 `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`; emitted CJS is `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlNode.js`, SHA-256 `86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45`.

The source declarations and runtime defaults differ by method and override:

| Source member | Default at that implementation | Omitted / explicit `undefined` | Explicit `null` |
|---|---|---|---|
| `AbstractMmlNode.verifyTree(options: PropertyList = null)` (`MmlNode.ts:906`) | `null` | Default parameter converts it to `null`; returns immediately | Returns immediately |
| `MmlMath.verifyTree(options: PropertyList = null)` (`MmlNodes/math.ts:126`) | `null` | Converts it to `null`, passes null to `super`; after that, may still run its raw-parent check and pass null to `mError` | Same base no-op, then same possible parent check with null |
| `AbstractMmlEmptyNode.verifyTree(_options: PropertyList)` (`MmlNode.ts:1417`) | no default | Receives `undefined`, but body is a no-op | Body is a no-op |
| `HtmlNode.verifyTree(options: PropertyList)` (`MmlNodes/HtmlNode.ts:96`) | no default | Receives `undefined`; if it has a truthy non-token parent, forwards `undefined` to `mError` | If it has a truthy non-token parent, forwards null to `mError` |
| `MmlMprescripts.verifyTree(options: PropertyList)` (`MmlNodes/mmultiscripts.ts:177`) | no default | `super.verifyTree(undefined)` applies the base default, then the subclass still checks parent and can pass original `undefined` to `mError` | Base no-op, then parent check can pass null to `mError` |
| `MmlNone.verifyTree(options: PropertyList)` (`mmultiscripts.ts:221`) | no default | Same two-stage behavior as `MmlMprescripts` | Same as `MmlMprescripts` |
| `mError(message, options, short = false)` (`MmlNode.ts:987`; empty override 1422) | Only `short` defaults | `options` remains undefined; after its parent-`merror` early return check, ordinary path creates an error node and sets `data-mjx-message`, then reading `options.fullErrors` throws | Same sequence, but the options value is null rather than undefined |
| `verifyAttributes(options)` / `verifyChildren(options)` (`MmlNode.ts:934,973`) | no default | A direct call receives undefined and accesses option fields/children per the method; no global default is applied | Direct base `verifyAttributes(null)` fails on property access; `verifyChildren` itself does not read options before walking children |

`MmlNode`'s interface marks `verifyTree(options?: PropertyList)` and `mError(..., short?: boolean)` optional (`MmlNode.ts:268,275`), but concrete defaults are determined by the implementation actually called. In particular, default-parameter evaluation occurs on the invoked function, not inherited from an abstract interface. A derived method can receive the original `undefined`, call `super.verifyTree(undefined)` (where the base method turns it into null), and then use the still-original derived parameter later. This behavior is visible in the two multiscript overrides. `MmlMath` itself has a null default; `HtmlNode`, empty nodes, and the multiscript marker classes do not.

The standard MathML compile path supplies a `verify` value explicitly: `MathMLCompile.ts:58–63` initializes it from `AbstractMmlNode.verifyDefaults`, and `compile()` calls `mml.verifyTree(this.options['verify'])` at line 104. The source default verification object has `checkArity=true`, `checkAttributes=false`, `checkMathvariants=true`, `fullErrors=false`, `fixMmultiscripts=true`, and `fixMtables=true` (`MmlNode.ts:386–393`). This does not erase direct-call cases or user-supplied null/undefined values.

`mError` takes options as a required positional parameter; only its third `short` parameter defaults false. Its source implementation reads the original options value, and null and undefined produce different callback-visible values even though both throw on `.fullErrors` in the ordinary branch. The early return for an existing `merror` parent occurs before any options read, so either value can return null without throwing. The empty-node override ignores all arguments unconditionally.

Root's actual pinned-source boundary capture is saved at `build/p30-original-options-boundary.json` (12 cases). With a non-token parent, `MmlMath.verifyTree()` / explicit undefined / explicit null all pass null to its `mError` path because that override itself has `= null`; each records an `merror` with its message before throwing `Cannot read properties of null (reading 'fullErrors')`. For `MmlMprescripts` and `MmlNone`, omitted and explicit undefined reach `mError` as undefined after the base call's default-null no-op, and throw `Cannot read properties of undefined (reading 'fullErrors')`; explicit null instead throws the corresponding null message. In all six failing marker cases, the `merror` factory creation and `data-mjx-message` write are visible before the exception. These records confirm that a base method's local default does not rewrite the derived method's original parameter for later derived code.

## Current production declarations and conversions

All declarations below currently use either `std::optional<PropertyList>` or the wrapper `ErrorOptions` rather than a single `Property` argument:

| Native file | Declarations/methods |
|---|---|
| `src/math/core/mml_tree/mml_node.ixx:18–29` | `ErrorOptions` stores `std::optional<PropertyList>`; default construction creates an empty object, `nullptr` creates no value, and `value()` returns the optional. It has no `Missing` alternative and cannot carry primitive option values. |
| `mml_node.ixx:81–82` | `MmlNode::verifyTree(std::optional<PropertyList> options={})`; `MmlNode::mError(..., ErrorOptions, bool shortError=false)`. |
| `mml_node.ixx:141–142,159–160` | `AbstractMmlNode` repeats `verifyTree`/`mError`; its virtual `verifyAttributes(PropertyList)` and `verifyChildren(PropertyList)` require a non-null object value. |
| `mml_node.ixx:223–224` | `AbstractMmlEmptyNode` repeats optional `verifyTree` and `ErrorOptions` `mError`. |
| `mml_nodes/math.ixx:6` | `MmlMath::verifyTree(std::optional<PropertyList>={})`. |
| `mml_nodes/html_node.ixx:18` | `HtmlNode::verifyTree(std::optional<PropertyList>={})`, despite source having no default parameter. |
| `mml_nodes/mmultiscripts.ixx:14,22,30` | `MmlMmultiscripts::verifyChildren(PropertyList)`; `MmlMprescripts` and `MmlNone` each declare optional `verifyTree` with a C++ empty default, despite source having no default. |
| `mml_nodes/maction.ixx:20`; `mml_nodes/mglyph.ixx:13` | Derived `verifyAttributes(PropertyList)` overrides. |
| `mml_nodes/mtable.ixx:18`; `mtr.ixx:16`; `mtd.ixx:16` | Derived `verifyChildren(PropertyList)` overrides. |

Relevant implementation-side conversions and calls:

* `src/math/core/mml_tree/mml_nodes/html_node.cpp:19–21` has `htmlErrorOptions(optional<PropertyList>)`, mapping an engaged object to `ErrorOptions(object)` and no value to `ErrorOptions(nullptr)`.
* `src/math/core/mml_tree/multiscript_host.ixx:27–29` has the equivalent `multiscriptErrorOptions` conversion.
* `src/math/core/mml_tree/mml_nodes/mglyph.cpp:24–27` wraps its required `PropertyList` argument with `ErrorOptions(options)` for `mError`.
* `src/math/core/mml_tree/mml_node.cpp:558–605` contains the base verification/error implementations and calls; `mml_empty.cpp:48–49` contains empty-node overrides. `math.cpp:58`, `html_node.cpp:52`, and `mmultiscripts.cpp:19–30` contain subclass override bodies. The current source at those sites includes accepted P29 base/empty inheritance changes, but this audit does not change any implementation.

The concrete boundary losses are: `std::optional<PropertyList>` collapses explicit source null with absent/undefined; its default `{}` adds a C++ call-site default even to source overrides that have none; `ErrorOptions` can distinguish null from an object but not null from undefined, and default construction means an empty object rather than undefined; `PropertyList` parameters cannot carry the source's null/undefined/primitive options value. C++ default arguments are substituted at the statically selected declaration's call site; unlike JS default parameters, they do not run inside the dynamically invoked override. Passing a `PropertyList` through the current wrappers retains object-handle identity, so the issue is state representation/default evaluation, not necessarily object cloning.

## Native oracle and generated-fixture sites

These generator templates contain C++ signatures, conversions, or stimuli tied to the current options types. Their generated fixture counterparts also encode the current mapping; future work should treat the original JS stimuli and expected values as frozen and update only the Native emitter representation under Root's gate.

| Generator | Generated fixture | Current options-dependent constructs |
|---|---|---|
| `tools/math-port/mml-node-oracle.cjs:56,127,161,202` | `tests/math/reference_mml_node.cpp` | `std::optional<PropertyList>` callback/override; `ErrorOptions` callback; direct `std::nullopt` and property-list calls; verification and mError observers. |
| `tools/math-port/html-node-oracle.cjs:34,39,60–61` | `tests/math/reference_html_node.cpp` | optional received error options; `ErrorOptions` override; emitter converts the `null` stimulus to `std::nullopt`, losing source null-vs-undefined distinction. |
| `tools/math-port/multiscript-oracle.cjs:51` | `tests/math/reference_multiscript.cpp` | optional `verifyTree` overrides and `ErrorOptions` observers; generated source has both `std::nullopt` and object options cases. |
| `tools/math-port/simple-mml-oracle.cjs:29,33,59` | `tests/math/reference_simple_mml.cpp` | optional error-options storage, `ErrorOptions` override, error wrappers, and verifyAttributes objects. |
| `tools/math-port/table-oracle.cjs:29,32` | `tests/math/reference_table.cpp` | `ErrorOptions` override and optional `verifyTree` override used by sinks. |
| `tools/math-port/maction-oracle.cjs:63` | `tests/math/reference_maction.cpp` | `ErrorOptions` override for verification callback observation. |
| `tools/math-port/leaf-oracle.cjs` | `tests/math/reference_leaf.cpp` | direct omitted `verifyTree()` call; this distinguishes source default behavior from an explicitly constructed empty object at the call boundary. |

Related mutation templates that assert today’s wrapper/conversion spellings include `html-node-mutation-test.cjs`, `leaf-mutation-test.cjs`, `multiscript-mutation-test.cjs`, `simple-mml-mutation-test.cjs`, and `table-mutation-test.cjs`; `maction-mutation-test.cjs` exercises its options callback path. The generated fixtures are not independent sources of JS semantics, but their C++ calls, observer signatures, and exact-null stimuli will need to remain coherent with any Root-frozen native coordinate.

## Scope boundary

This inventory does not select a sentinel, change method defaults, or prescribe overloads. It distinguishes three currently observable situations: source default-parameter conversion within a particular implementation, explicit null passed through an override, and omitted/undefined reaching an override without a default. Root must freeze which of those are representable at the typed native boundary before any migration. All methods retain their existing separate roles: options defaulting in `verifyTree`, option consumption in `verifyAttributes`/`verifyChildren`, and pass-through to `mError`.
