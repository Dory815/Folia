# P19b ParseMethod receiver audit

Read-only audit of the pinned MathJax 4.1.3 profile at upstream commit
`fb987178c2d279a99b0db5ea02e751691703955e`, using `scope.json`'s 38 TeX
packages and the installed CJS distribution. No source, API, expectation, or
build files were changed.

## Source pins inspected

| Boundary | Upstream SHA-256 | Installed CJS SHA-256 | Receiver-sensitive sites |
| --- | --- | --- | --- |
| `ts/input/tex/TokenMap.ts` | `0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e` | `9e3f086080a6ab052b68b22a5d9ee3dbc5e17d7b0c03a31e789664b01df36cf7` | `AbstractTokenMap.parse` 123–126; `MacroMap.parse` 315–327; `CommandMap.parse` 335–352; `EnvironmentMap.parse` 379–391 |
| `ts/input/tex/MapHandler.ts` | `7cacdaef3ed1df90b4f05cc0b3183a2956ae8f848f20ae8833932faa4efcb2a2` | `d82e3034776d2392689aad86500b5e60dbe0aa5540853b48f308841879f3cf67` | `SubHandler.parse` 112–130, especially line 127 fallback call |
| `ts/input/tex/ParseMethods.ts` | `1cdc4c6828078cd40863269864cdd6b7e5d22024e2ed36f4eeafb673781a5a70` | `2af91730bf39c97e86169c68cdc8c1054c05c58216f60a2f2d8e54ec75294851` | `ParseMethods.environment` 212–215 |
| `ts/input/tex/Types.ts` | `924d0b48bf7b5fdb0ccc995171fbdfb001bc3ac51a0af1409bb8c1d953430927` | — | `ParseMethod` signature 37–42 |
| `ts/input/tex/newcommand/NewcommandMethods.ts` | `2d3a5422123d9afb36e9277f506d5a24a7a1d3295163a34371c8fb7400b41cfa` | `7aa2e76b2263b2f12961958c86e2325acba83083334ca54f6949f86a25bd80cb` | `Let` 142–155 creates a dynamic ParseMethod wrapper for CharacterMap callbacks |
| `ts/util/FunctionList.ts` | `d4106c42268e0d7cb3549f8ffdbeb5bab87eae6daffae7314d5e724045a6918f` | `571f57801b863b2d9f01a66dc72f2836b7fd06a1a48a3fe0057e4ec3d51a7278` | `execute` 75–84 and `asyncExecute` 105–116 invoke each prioritized record's `.item` |


## Invocation receiver ledger

| Call site | Effective JavaScript `this` for the called ParseMethod | State/read-order effects |
| --- | --- | --- |
| `AbstractTokenMap.parse` | `undefined` in the strict CJS module | Stores `parser = parserFor(token)`, then reads `mapped = lookup(token)`, then calls local `parser(env, mapped)`. The local function value is not called as a property of the map. |
| `MacroMap.parse` | `undefined` | Looks up the Macro, then calls `parserFor` (which looks up again); after truthy checks, calls local parser with `(env, macro.token, ...macro.args)`. |
| `CommandMap.parse` | `undefined` | After lookup and parser resolution, reads `env.currentCS`, writes `"\\" + token`, then calls local parser with `(env, "\\" + macro.token, ...macro.args)`. It restores `currentCS` only after a normal callback return. If argument evaluation or the callback throws, the changed currentCS remains. |
| `EnvironmentMap.parse` outer parser | The `EnvironmentMap` instance (`this`) | The call is syntactically `this.parser(...)`, so property lookup and member-call receiver are the map. `parser` is a mutable inherited getter/setter field. Arguments are `(env, macro.token, envParser, macro.args)`; `macro.args` is passed as one array argument. |
| `ParseMethods.environment` nested `func` | `undefined` | The outer environment parser creates a `begin` item and calls its local `func(parser, mml, ...args.slice(1))`, not `func.apply` and not a member call. |
| `NewcommandMethods.Let` generated wrapper | The wrapper is an arrow and therefore has lexical `this` from the bare-called `Let` callback (`undefined`); its nested underlying parser receives the captured `CharacterMap` as `this` | In the reachable `\let` path for a non-delimiter CharacterMap, source line 153 creates `(p) => map.parser(p, macro)`. The arrow captures map and Macro identities, then performs a live member read/call of `map.parser` on every invocation. Replacing the map's parser after `\let` therefore changes behavior. The arrow itself is later stored as a Macro function and called as a local parser by MacroMap. |
| `SubHandler.parse` fallback | The selected `PrioritizedListItem` record | Source line 127 calls `Array.from(this._fallback)[0].item(env, token)`. This is a member call whose receiver is the same `{item, priority}` record object, not the FunctionList, not the function, and not the original fallback registration array. No fallback result is returned. |
| `FunctionList.execute` / `asyncExecute` | The corresponding prioritized record | Lines 80 and 111 likewise call `item.item(...)`; their callback receiver is the record. This is shared FunctionList behavior for callbacks, including fallback/configuration lists. |

The profile's actual built-in ParseMethod bodies were checked across all
enabled package configuration, mapping, and `*Methods.ts` sources. No registered
handler body depends on `this` or stores mutable state on its function object.
The reachable `\let` wrapper is the exception in receiver routing: it passes
the captured CharacterMap as receiver to the underlying parser, although current
built-in parser functions do not inspect that receiver. No registered
ParseMethod body calls `.call`, `.apply`, or `.bind`. Mutable state belongs to
the parser/context, token map and its parser property, Macro argument arrays,
attributes, and prioritized records. This is not a claim that arbitrary
injected JavaScript callbacks are supported by the native raw-TeX API.

Other `this` matches in the enabled package sources are not ParseMethod bodies:
Cases/Colortbl stack-item or tag classes, TagFormat's formatter callback,
Configuration's own processors, and `MathtoolsItems.ts:70`'s stack-item
`super.EndTable.call(this)`. That last `.call` is a real reachable call, but it
is stack-item dispatch rather than a ParseMethod invocation. It should not be
folded into the ParseMethod callback receiver contract.

## Actual CJS probes

The probes ran in separate Node `v24.13.0` processes through
`tools/math-port/reference-profile.cjs::createReference()`, which verifies the
pinned profile SHA, dependency lock, and installed `@mathjax/src` version 4.1.3
before returning the actual CJS `TokenMap.js`, `MapHandler.js`, and
`ParseMethods.js` modules. The test callback functions below are probes of the
actual call sites, not profile package handlers or an assertion that external
JS injection belongs in native scope.

Probe 1 constructed actual CJS `MacroMap`, `CommandMap`, `EnvironmentMap`, and
`SubHandler` instances, with strict callbacks recording `this`. Result:

```json
{
  "macroPlainCallThisIsUndefined": true,
  "commandPlainCallThisIsUndefined": true,
  "commandRestored": true,
  "environmentMemberCallThisIsMap": true,
  "fallbackMemberCallThisIsRecord": true,
  "fallbackThisHasItem": true,
  "fallbackThisPriority": 5
}
```

Probe 2 wrapped the actual installed `ParseMethods.environment` method, then
called it through an actual `EnvironmentMap`; the nested environment function
recorded its own receiver. A throwing actual `CommandMap` callback checked
partial parser state. Results:

```json
{
  "commandThrowLeavesCurrentCS": true,
  "environmentParserReceiverMap": true,
  "nestedEnvironmentFunctionReceiverUndefined": true,
  "actualEnvironmentMethodName": "environment"
}
```

These results establish the JavaScript receiver and mutation semantics at the
locked CJS call sites. The success case also confirms CommandMap restores
currentCS after the callback returns normally; the throw case confirms there is
no `finally` restoration.

Probe 3 used the locked profile's real `lcGreek` CharacterMap, temporarily
wrapped its actual parser, and converted `\\let\\foo\\alpha\\foo` through the
actual TeX document. The observation was:

```json
{
  "mapName": "lcGreek",
  "observations": [
    {"receiverIsMap": true, "currentCS": "\\foo", "token": "α"}
  ]
}
```

The wrapper was restored in `finally`. This confirms the `\\let` branch's
reachable generated arrow performs `map.parser(p, macro)` with the captured
map as the underlying parser receiver and passes the token object captured by
lookup.

## Difference from current native types

`src/math/input/tex/types.ixx` represents `ParseMethod` as a
`variant<Property, ParseCallableRef>`, where `ParseCallable::call` receives
`ParserContext&`, `ParseAtom`, and `ParseRest`. It has no JavaScript call
receiver. `ParserContext` currently only exposes currentCS, and the callback
protocol carries no `EnvironmentMap` identity or prioritized-record identity.
The parser/token values and result types are also typed host carriers rather
than the source's complete `TexParser`, function, and symbol objects.

For the pinned built-in handlers, no callback observes `this`, so the four
receiver categories do not require a callable JS compatibility layer. If the
next contract requires source-like user-supplied ParseCallable probes or future
callbacks that inspect their receiver, the typed seam must explicitly identify
the call site receiver: undefined for bare calls, the exact map instance for
EnvironmentMap.parser, and the exact prioritized item record for fallback
calls. Passing only a generic callback or a function name cannot reproduce
those identities. Root should decide whether that extension is in scope before
freezing the TokenMap/MapHandler API; this audit proposes no implementation.
