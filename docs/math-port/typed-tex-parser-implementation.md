# Native TeX parser implementation boundaries

Root implementation contract, 2026-10-20. Preserve the scope in `scope.json`
and the existing MathJax classes, files and call order. This is the next native
implementation layer after the verified MML/Common components. It is not a
claim that TeX rendering or Folia integration is complete.

## Module and file ownership

`src/math/input/tex/parser_family.ixx` remains the one primary module for the
mutually dependent parser, Stack, BaseItem, StackItemFactory, ParseOptions,
Tags, configuration and dispatch declarations. Its existing family files
become or remain partitions of that module; implementations use
`module folia.math.input.tex.parser; import :<partition>;`. Do not forward
declare a parser-owned class in a different named module and define it here.
Token, Macro, TokenMap and MapHandler participate in that same cyclic graph:
their original files become partitions too. Importing the old standalone
TokenMap primary would reintroduce its Context/Property dependencies and is
not an acceptable declaration dependency. Pure environment, session and scanner
utilities retain their existing independent modules.

| Existing files | Responsibility |
| --- | --- |
| `parser_types.ixx` | same-module forwards and the closed parser/stack control values; no service implementations |
| `stack_item.ixx`, `stack_item.cpp`, `base_item.cpp` | MmlStack node sequence and BaseItem's actual shared transition algorithm |
| `stack.ixx`, `stack.cpp` | accepted frame owners, active environment, ordered Push/Pop/replay/Prev |
| `stack_item_factory.ixx`, `stack_item_factory.cpp` | actual generic Tree Factory specialization returning unique owners, typed registrations, configuration borrow |
| `base/base_items.ixx`, `base_items.cpp` and existing family implementation units | the original concrete BaseItems and their named fields/checkItem methods |
| `parse_options.ixx`, `parse_options.cpp` | request services, options, parser scopes, node lists and root/error lifecycle |
| `configuration.*`, `map_handler.*`, `token_map.*`, `token.*`, `macro.*` | Source configuration composition, handler priority/lookup and typed token/command definitions |
| `tex_parser.*` | semantic character loop/dispatch, Push provenance, argument parsing and nested parser lifetimes |
| `tex_scanner.*` | existing UTF-16 lexical primitives and the sole current immutable source buffer |
| `tags.*`, `session.*` | tag algorithms/request state and the already native persistent macros/labels/counters, respectively |
| existing package directories/units | their configured package algorithms and concrete frame subclasses |

NodeFactory, the existing lexical/session/environment utilities and the MML
document retain their current native responsibilities. In particular,
MmlDocument owns the only MmlFactory. ParseOptions/NodeFactory do not create a
second document or MML owner. Do not add TexParseRequest, parser2, generic
runtime context, source-object identity services or a second node index.

## Stack control values

Use one closed `StackKind` for the actual configured frame kinds; package
overrides retain the Source kind (for example ColorArrayItem is still array).
The generic Factory's registered class predicate distinguishes actual classes.
Constructors accept named domain initialization records, selected from a
closed variant matching real Source constructor signatures. Attributes belong
to MML attribute values; frame properties become concrete members of the
responsible subclass, never an all-frame property bag or a string-keyed map.
Keep Source-public frame data as named typed public members where no C++
ownership invariant requires encapsulation, following the existing native
CommonWrapper pattern. Do not replace every field with redundant setter/getter
pairs. Stack ownership and control transitions remain encapsulated; real
Source protected methods and package inheritance remain intact.

`StackEntry` accepts only an owned frame, a borrowed MML node, or an explicit
empty entry. Source Push skips empty entries and wraps MML in an actual MmlItem.
It does not clone MML. Frame node sequences borrow nodes from the one document.
MmlStack.First is the last-pushed node; Last is the first-pushed node. Prev
pops a node, not a frame. Preserve the exact inferred/forceRow toMml behavior.
Peek follows Source slice(Size-count), including the signed-slice behavior when
count exceeds Size; it is not simply the last min(count,Size) nodes. BaseItem
isKind compares the actual kind value, while Factory class predicates have
their separate subclass/registration purpose. Script error keys sub/sup belong
to a closed StackErrorKey, not extra frame kinds. Root displacement fields
retain their signed relative em strings, including a leading plus.

ProofTreeItem overrides the full native toMml(bool,bool) signature, but its
actual Source override ignores incoming flags and invokes the base conversion
with defaults after inner-stack finalization. Do not reuse the old C++ body
that incorrectly forwards those flags. The Stack interface reexports
:stack_item so consumers destroying returned frame owners see complete types.

`CheckResult` has explicit Accept, Drop and Replay states. Replay owns its
replacement sequence; entries are newly owned frames, borrowed MML, reuse-top
or reuse-incoming tokens. An empty Replay still pops top. No raw frame pointer
is adopted into a unique_ptr, and neither replacement storage nor mutable frame
references escape an owner that will be discarded.

`checkItem(unique_ptr<BaseItem>& incoming)` receives the Push-local owner by
reference. The current top remains owned and alive throughout its virtual call.
Ordinary methods borrow incoming. ProofTreeItem instead moves it into its
actual inner Stack at the Source call site, then returns Drop; outer Push may
only destroy an owner it still holds. Accept/reuse-incoming require that owner
to remain available. The returned replay is interpreted only after the Source
Pop/environment transition; the extracted top and remaining incoming owner stay
alive until reuse tokens are resolved. Each existing owner is moved at most
once, and recursive replay finishes before the next original Push argument.

Keep the finite TexEnvironment shared value and separate StackGlobal ownership
rules in `native-raii-architecture.md`. In particular Pop does not clear a
surviving frame's environment: actual BaseItem.env is a prototype accessor and
Source's delete expression does not remove its backing value. No frame owns a
parser, document or session. The scanner snapshot passed to startStr contains
only immutable UTF-16 text, so replacing parser input cannot invalidate it.

## Parser and request services

ParseOptions owns request-scoped NodeFactory/StackItemFactory, node-list
membership and parser-scope bookkeeping, and borrows the one MmlDocument and
native session. Node-list keys are the existing NodeKind plus the finite
auxiliary keys already identified in the 38-package inventory. Node pointers
are document borrows. addNode preserves duplicate/order behavior; getList
performs actual final-root ancestry pruning. Copying MML must also copy its
auxiliary-list membership through this request service, without reviving the
comma-joined generic `in-lists` property.

Keep Source parser nesting order and make its lifetime exception-safe using a
lexical scope guard. The current parser is a borrowed pointer, never retained
by session state, package callbacks or asynchronous work. All nested parsers
share the same document, configuration and request services. Clearing a request
resets only the fields Source clears; persistent macro/tag effects follow the
existing session contract and must not acquire an invented rollback policy.

The immutable configured dispatch tables carry named typed callbacks and
finite mathematical definition data. Their real package initialization and
priority ordering remain required work. Do not substitute opaque function
values, receivers, dynamic JS argument conversion, or an arbitrary packageData
map. Each configured package's request/session/environment state has the owner
already listed in the closed inventory.

Source ParseOptions itself lives with its TeX jax, and clear() does not clear
packageData. In the native ownership split, persistent color definitions belong
to the original ColorModel class owned optionally by the renderer/session, and persistent group/
definition scope data belongs to a named BegingroupSessionState. These records
must not retain a request's parser, MML nodes or mutable environment. Request
ParseOptions borrows those explicitly typed states. A per-render scratch
replacement of all package state would change the continuous-session profile.
The earlier ColorPackageState sketch does not justify a second color map:
ColorModel already has precisely this responsibility in ColorUtil.ts. Keep its
original color/util and color/constants files and native value ownership.

The existing NodeFactory module exposes only a typed NodeKindRegistration
borrow for standard-kind registration. makeNode notifies after successful
construction; makeToken registers its outer node, and makeError registers
mtext then merror. Text and MmlFactory's implicit inferred rows do not notify.
ParseOptions implements this protocol using its one node-list store; auxiliary
lists remain its direct typed overload. Standalone document construction may
omit registration. No parser primary import, duplicate node index or owning
observer is added to NodeFactory. Native token-kind preconditions are checked
before allocation; invalid generic JS API calls are not a native simulation
requirement.

The proposed core::MathItem dependency also needs its native declaration:
the old module still imports the generic object and DOM graph. Preserve the
original expression-state responsibility in core/math_item; do not use that
old primary as a shortcut or duplicate it in a parser-local item facade.
The concrete Tags and input-jax lifecycle contract must retain display changes
and tag recompile state/counter values at this boundary.

For the configured MtMultlined constructor call, retain the observed true
default-tag behavior as a distinct MultlinedItemInit marker. Source passes a
parser object into Tags.defaultTags, but its actual getTag branch only consumes
the truth value; native code does not retain that incidental object identity.
The constructor witness also distinguishes Source's DummyItem class (inherited
kind `base`) from its `dummy` default lookup key. Do not silently repair or
claim stock fallback wiring that the pinned Source does not provide.

## Review and acceptance

Root first resolves the constructor and request-service declarations against
the existing 38-package inventory. Body assignments name exact files and the
frozen signatures; Luna writes ignored candidates, and Root reviews/promotes,
builds, runs native tests and commits in responsibility-sized batches. The
existing C++ algorithms are the implementation baseline, with pinned TS/CJS
used to resolve semantic questions. Do not retranslate the entire parser.

The actual Source transition evidence already exists in the ignored
`tex-stack-transition-source` packet: 24 base cases plus six package cases,
with hash-locked originals and explicit stub limits. Its fixed observations
cover ordinary accept/drop, empty and nonempty replay, top/incoming reuse,
bare MML, nested-stack transfer, environment copy/aliasing, Prev/toMml and
errors. Native tests must use actual native document/factory/frame classes and
verify these state changes and owner destruction on success and exception.
Package custom transition rows remain distinct from complete package parsing.

Parser acceptance additionally checks real token dispatch, currentCS and UTF-16
positions, Source provenance stamping, nested ParseArg/ParseUpTo, expansion
ordering and limits, configuration precedence, node-list lifecycle, tags and
error conversion. Preserve every configured package. Scanner, stack or a base
expression passing alone does not establish the complete TeX input layer.

Final integration still requires native TeX to actual MML, the existing shared
Common layout to completed immutable Scene, the SVG serializer, the unchanged
profile oracle, and Folia's existing SVG path replacing its math QuickJS worker.
No D2D pixel gate or Mermaid work is added here.
