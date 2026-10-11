# P22b TokenMap implementation evidence

This report records the implementation in `src/math/input/tex/token_map.cpp`
against the root-frozen contract `docs/math-port/p22b-token-map-contract.md`.
The pinned TypeScript source is
`.reference/MathJax-src/ts/input/tex/TokenMap.ts`, SHA-256
`0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e`.
The installed CJS counterpart is
`tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/TokenMap.js`,
SHA-256 `9e3f086080a6ab052b68b22a5d9ee3dbc5e17d7b0c03a31e789664b01df36cf7`.
The CJS file confirms spread lowering and call form; the pinned TS remains the
source authority. This is an implementation map, not an acceptance claim.

## Source member mapping

| Pinned Source member | Native implementation | Behavior represented |
|---|---|---|
| `parseResult` (78–80) | `parseResult` (127–132) | Converts only `Property(Missing{})` to true; preserves false, null, references, and other results. |
| `AbstractTokenMap.constructor` (94–100) | `AbstractTokenMap::initializeSource` (134–138), called by `TokenMapHost::create` after final shared allocation | Stores name before parser, then registers that exact shared object immediately. Root's tag constructor is allocation-only. |
| `name` getter (104–106) | `AbstractTokenMap::name` (140–142) | Returns stored Property verbatim, including Missing. |
| `parserFor` (116–118) | `AbstractTokenMap::parserFor` (152–155) | Calls virtual `contains`; reads parser only on truthy result, otherwise returns null. |
| `parse` (123–127) | `AbstractTokenMap::parse` (157–165) | Calls `parserFor`, then virtual `lookup` unconditionally; JS truthiness gates invocation; passes the mapped value as a ParseAtom and converts only an undefined result. |
| parser setter/getter (129–135) | `AbstractTokenMap::parser` overloads (144–150) | Getter returns current method; setter replaces it. Base initialization writes its private state directly, not through this virtual setter. |
| `RegExpMap.constructor` (155–163) | `RegExpMap::initializeSource` (167–170) | Runs/registers base initialization before assigning the RegExpRef. |
| `RegExpMap.contains` (166–168) | `RegExpMap::contains` (172–175) | Calls the same live regex service's `test` method. No regex engine is introduced here. |
| `RegExpMap.lookup` (173–175) | `RegExpMap::lookup` (177–180) | Calls `contains` again and returns the original input token or null, retaining the inherited parse double-test sequence. |
| initialized `AbstractParseMap.map` field (186) | `map_` optional storage emplaced in `AbstractParseMap::initializeSource` (182–185) | The optional distinguishes pre-initialization from an empty initialized Map. It is initialized only after base registration. |
| `AbstractParseMap.lookup` (191–193) | `AbstractParseMap::lookup` (187–193) | SameValueZero key search; returns Missing when absent, distinct from stored null. |
| `contains` (198–200) | `AbstractParseMap::contains` (195–200) | Independent key-presence search, regardless of mapped value. |
| `add` (208–210) | `AbstractParseMap::add` (202–214) | SameValueZero replacement preserves order; new keys append; numeric zero is canonicalized to +0. |
| `remove` (217–219) | `AbstractParseMap::remove` (216–222) | Deletes a matching key without changing other entries' order. |
| `CharacterMap.constructor` (235–250) | `CharacterMap::initializeSource` (224–243) | Calls base registration/map init, snapshots Object.keys, then reads each value live, decodes string vs pair, creates Token, and invokes virtual `add`. Array pair reads are bounded by captured length; ordinary values use indexed property reads. |
| `DelimiterMap.parse` (260–262) | `DelimiterMap::parse` (245–249) | Concatenates backslash with the input token before explicitly dispatching through base parse; subsequent contains/lookup remain virtual. |
| `MacroMap.constructor` (281–305) | `MacroMap::initializeSource` (251–271) | Defaults functionMap only for Missing, registers and initializes map, eagerly snapshots all entries, resolves method strings by property access, slices array args from one, creates Macro objects, and calls virtual add. |
| `MacroMap.parserFor` (307–310) | `MacroMap::parserFor` (274–278) | Performs its own lookup, applies JS truthiness, then reads the selected Macro's live func. |
| `MacroMap.parse` (315–325) | `MacroMap::parse` plus `invokeBare` (280–287; helper 113–123) | Captures first lookup, invokes parserFor for the second lookup, checks both, then reads token and args and spreads current args into bare-call rest slots. |
| `CommandMap.parse` (335–348) | `CommandMap::parse` (289–307) | Keeps both lookups; saves and changes currentCS before token/args reads; invokes bare parser; restores currentCS only after normal return; then applies parseResult. Exceptions leave the changed state. |
| `EnvironmentMap.constructor` (366–376) | `EnvironmentMap::initializeSource` (309–313) | Runs MacroMap initialization/population first, then sets the general parser through the virtual setter. |
| `EnvironmentMap.parse` (379–389) | `EnvironmentMap::parse` (315–330) | Captures macro and environment parser via two lookups, then stages reads in source order; passes parser and the same Macro.args Property as two unspread slots and uses the exact shared EnvironmentMap owner as receiver. |

## Shared helpers and evaluation constraints

The private helpers at native lines 19–123 implement JS truthiness for the
typed method/lookup alternatives, convert lookup results to ParseAtom without
copying Token or Macro identity, perform default-hint token concatenation,
bounded two-slot destructuring, property reads from supported raw values,
method resolution from functionMap, eager Object.entries capture, Environment
call-slot canonicalization, and bare parser invocation through the existing
`arguments::expand` and `invokeParseMethod` protocols. The helpers do not add a
second object/function representation.

All order-sensitive getter, lookup, registry, and callback operations are
staged in locals. `AbstractParseMap::map_` is ordered data separate from the
ordinary PropertyList graph; SameValueZero keys support NaN and zero identity
as in JS Map while not conflating numeric and string keys. CharacterMap delays
per-value reads until after the key snapshot; MacroMap captures all key/value
pairs before population. Macro/Command spread current args at invocation;
Environment passes the aliased args array as one argument. The command path has
no scope guard, preserving Source's non-finally restoration.

## Boundaries and status

RegExpMap uses only the frozen `RegExpRef::test` service and adds no regex
implementation or fallback. The implementation is limited to the accepted
Property/callable and typed map protocol; arbitrary JS proxies, descriptors,
custom iterators, and a general JavaScript VM are outside this packet, as the
contract states. No declaration, fixture, test, manifest, or other source file
was intentionally edited for this packet. `git diff --check --
src/math/input/tex/token_map.cpp` completed without whitespace errors. No build,
test, mutation, or integration command was run. Root owns independent source
review and acceptance; completion here does not assert parity or gate passage.
