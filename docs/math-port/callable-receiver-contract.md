# P19b2 root-frozen shared native callable and receiver protocol

This is the next prerequisite packet, not accepted implementation. Root owns
interfaces, build/test manifests, Source observations and acceptance. It follows
complete FunctionList acceptance and the TokenMap/MapHandler receiver audits.
TokenMap/MapHandler APIs remain unfrozen until the remaining regex, constructor
and registry boundaries are reviewed.

## One function identity and one invocation path

The same Source ParseMethod can be a map parser and an item in SubHandler's
actual FunctionList. Do not add a second callback field, duplicate fallback
wrapper identities, a substitute PrioritizedList, a fake priority record, or
erase the Source receiver. Use one shared native callable:

```cpp
// folia.math.util.object_types; namespace folia::math
struct NativeObject { virtual ~NativeObject() = default; };
using NativeObjectRef = std::shared_ptr<NativeObject>;

// folia.math.util.callable_types; namespace folia::math
using FunctionReceiver = std::variant<Missing, NativeObjectRef>;
using FunctionArgs = std::span<const std::any>;
// FunctionBody retains one shared Callback, including mutable closure state.
struct Function {
    FunctionBody call;
};
using FunctionRef = std::shared_ptr<Function>;
```

FunctionBody is a copyable handle over one shared native std::function. Saving
the active body keeps it alive through replacement without cloning mutable
closure state. It is the sole callback field, not another invocation path.

NativeObject is an identity/lifetime marker for explicitly ported objects,
not a property evaluator, prototype VM, string-tag dispatcher or generic JS
object substitute. Missing is the strict bare-call receiver. A shared object
is the actual receiver with its original control block and native type. Null
shared handles are invalid native input. Retaining a receiver retains that same
object, including a priority record removed during a callback.

PrioritizedListItem<T> derives from NativeObject, preserving exact priority/item
fields, the existing `{priority,item}` initialization spelling and accepted
source algorithms. There is still one record/storage. functions::invoke takes
the actual shared FunctionListItem handle, captures its item FunctionRef before
invocation and supplies that same record as FunctionReceiver. Sync and async
FunctionList both use it; storage/iterator/Promise behavior is unchanged.

## Source routing and order

| Source call | Receiver |
| --- | --- |
| AbstractTokenMap/MacroMap/CommandMap local parser | Missing |
| EnvironmentMap `this.parser(...)` | Exact EnvironmentMap |
| SubHandler first fallback `.item(...)` | Exact record from its actual FunctionList |
| FunctionList execute/asyncExecute | Exact priority record |
| Newcommand.Let character-map wrapper | Captured CharacterMap on its live member parser call |

The Let closure retains map/Token identity and reads the map's current parser
every time. Environment dispatch captures its member function and receiver at
the Source expression stage before argument getters. CommandMap restores
currentCS only after normal return. This protocol cannot conceal failures with
a cleanup restore. Complete maps require separate member ledgers/Source traces.

## TeX bridge, not another callable

ParseCallable/ParseCallableRef name that same Function/FunctionRef. ParseMethod
names the existing FunctionValue alternatives (Property or FunctionRef). There
is one function object/body/identity, with no old/new callback switch. In TeX:

```cpp
using ParseBody = std::function<ParseResult(
    const FunctionReceiver&, ParserContext&, const ParseAtom&, ParseRest)>;
ParseCallableRef makeParseCallable(ParseBody);
ParseResult invokeParseMethod(const ParseMethod&, const FunctionReceiver&,
    ParserContext&, const ParseAtom&, ParseRest = {});
```

Generic slots are `[reference_wrapper<ParserContext>, ParseAtom, ...rest]`.
Validate native slot types. ParserContext is borrowed during synchronous typed
invocation; parser/session ownership is a separate responsibility. Async owners
must retain their own arguments, never assume this bridge owns that parser.
Receiver ownership is independent. Original Property/Token/StackItem/ParseMethod/
raw-array identities in rest std::any slots are retained.

Flatten ParseResult's active alternative into std::any: Property(false) must be
the exact false seen by FunctionList, not a truthy boxed ParseResult variant.
Invocation reconstructs Property/StackItemRef/ParseSymbolRef alternatives;
native empty any is Source undefined. Preserve result identity and propagate
exceptions. Invalid native protocols and non-callable methods throw the existing
TypeError carrier. No TeX algorithm or JS evaluation belongs in either helper.

## Ownership and module graph

- Root owns object_types.ixx (marker only), callable_types.ixx (declarations),
  PrioritizedList's marker/construction adaptation, FunctionList-specific types
  and its validated shared-record invocation adapter.
- Root freezes input/tex/types.ixx declarations. Luna tex_parser may implement
  only the typed bridge in parse_callable.cpp after a specific dispatch.
- Luna math_layout may change only FunctionList.cpp's two invocation call sites
  after a specific dispatch, without changing any accepted algorithm.
- Root owns every oracle/expected value/native stimulus/test/mutation/manifest,
  build, status, acceptance and Folia wiring. This contract itself authorizes
  no agent edits; root writes declarations before assigning implementations.

object_types imports std only. callable_types depends on the marker/Property.
PrioritizedList depends on the marker, never TeX/MML. FunctionList-specific types
depend on shared callables/PrioritizedList/Promise; TeX types on shared callables/
Token. Keep named C++23 modules/import std, portability and no TokenMap cycle.

## Acceptance

Preserve all 403 actual-Source FunctionList observations and 29 native assertions,
changing only native binding syntax, never expected values. Re-run its complete
compiling mutation set and primitive/priority regressions. Preserve all accepted
Token/Macro data observations and their focused checks. New actual-CJS probes
must check strict undefined, actual map/priority-record identity, live item and
priority, mutation/removal/retention, map parser replacement, argument/result
identity, typed false through FunctionList and exception propagation. Native
lifetime witnesses retain a removed record and release it with the last saved
receiver. Compiling receiver/argument/boxing/error mutants fail assertions;
compile failures/crashes are not acceptance. Run full core build/tests. This
host packet increases no TS file count and authorizes no Folia switch.
