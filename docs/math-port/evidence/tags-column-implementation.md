# Tags and ColumnParser implementation ledger

This is an implementation map for pinned MathJax 4.1.3 `Tags.ts` and
`ColumnParser.ts`, not an acceptance report. The original-side observer is
`tools/math-port/parser-family-original-oracle.cjs`; its locked check recorded
65 observations and verified the 19 source-package hashes. That check does not
exercise the native implementation. Root owns compilation, differential runs,
and acceptance.

## Tags

| Source member | Native owner | Notes / boundary |
|---|---|---|
| `Label` / `TagInfo` constructors | `Label::initialize`, `TagInfo::initialize`; canonical constructors are registered by `ParserRuntime` | Both wrappers retain the actual Context object identity. Defaults apply only to `Undefined`; all provided values, including arrays, keep their original tag and identity. CJS constructor arities are 2 and 8. |
| `AbstractTags` field initialization | `initializeAbstractTags` | Initializes the one canonical Context instance’s own fields in source order. Dictionaries, history, stack, current `TagInfo`, and `enTag` are Context values; no C++ mirror is authoritative. |
| Prototype methods/accessors | `AbstractTags::initializeModule` | Installs methods on the real `AbstractTags`, `NoTags`, and `AllTags` prototypes after runtime class metadata exists. Methods are canonical `FunctionRef` bodies, and method lengths plus getter/setter descriptors are installed there. |
| `start`, `env`, `end`, `tag`, `notag`, `noTag`, `label` | Matching `AbstractTags` bodies and canonical accessor callbacks | `currentTag`, `stack`, and `history` are read/written through Context on each source operation. Label access uses the virtual methods in the typed path. |
| `formatUrl`, `formatTag`, `formatRef`, `formatId`, `formatNumber` | Matching bodies and URI/whitespace helpers | URL encoding and isolated-surrogate errors use the native UTF-16 utilities. |
| `autoTag`, `clearTag`, `getTag`, `resetTag`, `reset` | Matching bodies | Raw Context values and nullish/truthy checks are retained; reset creates fresh Context dictionaries/arrays. |
| `startEquation`, `finishEquation` | Canonical `FunctionRef` wrappers plus typed bodies | The wrappers accept an already-bound native MathItem. Converting arbitrary Context objects to MathItem is still explicitly `Unimplemented`; no substitute TypeError method is installed. The typed `inputData` Property graph also cannot retain Context object/symbol counters. |
| `finalize`, `makeTag`, `makeId` | Canonical `FunctionRef` wrappers and matching typed bodies | Tag formatting uses Context arrays and dynamic `map`/`join`; parsing and MML construction route through the actual TexParser and NodeFactory. MML/EnvList conversion from arbitrary Context values remains an explicit adapter boundary. |
| `NoTags` overrides, `AllTags.finalize` | Matching overrides plus prototype callbacks | The history search reads each item’s `taggable` property; nullish entries must throw on that read. |
| `TagsFactory` state and methods | `TagsFactory::initializeModule`, `add`, `addTags`, `create`, `setDefault`, `getDefault` | One actual factory registry and `OPTIONS` Context object are initialized after the real class descriptors. Descriptor allocation owns the sole constructor/identity/initialization phase. |

The `Tags` C++ bodies now resolve internal `this.method(...)` operations via
the current Context property and invoke it with the canonical Tags receiver, so
an instance/prototype override participates in dispatch. `getTag` and `makeTag`
use their canonical NodeValue-returning `FunctionRef` bodies for native typed
calls; the Context result adapter can represent only null/undefined nodes.
MathItem, arbitrary MML nodes, and EnvList still need canonical cross-graph
bindings before ordinary Context calls can pass through those methods.

## ColumnParser

| Source member | Native owner | Notes / boundary |
|---|---|---|
| `ColumnState` record | `ColumnState::get/set` | The proxy stores only a tagged `options::Value` receiver. Template/index/array fields live on the original Source state object; ordinary objects and primitive property access go through Context. |
| Class prototype and constructor fields | `ColumnParser::initializeModule`, `initializeColumnParser(actualOwner)` | The module hook installs real prototype methods once after constructor metadata is available. Construction binds the actual shared ColumnParser owner, then initializes own `columnHandler` and `MAXCOLUMNS` fields in Source order. Handler functions close over that genuine parser instance. |
| `process` | `ColumnParser::process` plus its prototype binding | Creates one ordinary state object, retains the parser and ArrayItem identities and their Context fields, strips one enclosing brace pair when applicable, scans code points, rereads the live limit, and dynamically invokes the current `processColumn` and finalizer methods in source order. The prototype binding resolves actual `TexParser` and `ArrayItem` owners through runtime identity bindings. |
| `processColumn` | `ColumnParser::processColumn` | Reads `columnHandler` for own-property testing, then reads it again for indexed lookup and uses that second table as callback receiver. |
| `setColumnAlign`, `setColumnWidths`, `setColumnSpacing`, `setColumnLines`, `setPadding` | Matching protected bodies | Array calls (`join`, spread/iteration, `push`, `slice`, `map`, `pop`) use the owning Context’s operations. Target records and frame handles are retained at their Source evaluation points. |
| `getColumn`, `getDimen`, `getAlign`, `getBraces` | Matching public bodies and prototype callbacks | Uses the real UnitUtil matcher, UTF-16 brace scanning, raw template state, and TexError types. `getColumn` retains raw `ralign`/`calign` values until the Source truthy fallback. |
| `macroColumn`, `addRule`, `addAt`, `addBang`, `repeat`, `brace` | Matching public bodies and prototype callbacks | Routes substitution through ParseUtil, keeps source order for array reads/writes, and performs Source `parseInt`/string validation and repeated-template expansion. Internal Source method calls resolve current Context properties and pass the canonical ColumnParser receiver. |

The Context prototype binding for `process` depends on genuine `ArrayItem` and
`TexParser` instance bindings in `ParserRuntime`; resolving an unregistered or
foreign object remains an error. Its state record and ArrayItem fields are
Context values. The implementation does not manufacture a surrogate ArrayItem
or mirror its fields. Full interoperability still depends on the Source
BaseItems allocator binding each actual instance.

## Verification status

Root's prior combined compile compiled and linked the Tags and ColumnParser
bodies before the latest dynamic-dispatch revisions. This worker has not run a
build or native test. The latest changes—including canonical FunctionRef
wrappers, current-instance/prototype method dispatch, raw ColumnState array and
getColumn values, and Context↔MML adapter boundaries—still require Root's next
compile and original-CJS differential review. No native parity or full-profile
acceptance is claimed here.
