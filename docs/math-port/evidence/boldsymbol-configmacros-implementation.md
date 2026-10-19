# Boldsymbol and ConfigMacros source ownership

The pinned source methods are ported into separate loaded-package modules; neither package adds a TeX command-name branch to the parser.

| Source method / lines | Native owner | Responsibility |
| --- | --- | --- |
| `BoldsymbolConfiguration.ts:34-47` | `BoldsymbolConfiguration::initializeModule` | Register the actual `CommandMap` method and construct the package `Configuration`. |
| `BoldsymbolConfiguration.ts:56-64` | canonical `createBoldToken` function | Call `NodeFactory.createToken`, exclude `mtext`, read the current parser environment, set `fixBold`, and add the token to ParseOptions' actual list. |
| `BoldsymbolConfiguration.ts:73-88` | canonical `rewriteBoldTokens` function | Iterate the live `fixBold` array, map only the pinned variants, write `mathvariant`, and remove `fixBold`. Registered as the package postprocessor. |
| `ConfigMacrosConfiguration.ts:50-68` | canonical init/config functions | Create the three named token maps; append a local configuration at its Source priority; apply active, macro, then environment definitions. |
| `ConfigMacrosConfiguration.ts:76-108` | `setMacros` | Snapshot enumerable own definition keys, read each current value, preserve the template callback lookup before `slice`/`concat`, and add real `Macro` entries to the actual maps. |
| `ConfigMacrosConfiguration.ts:113-153` | active/macro/environment helpers | Retrieve actual maps from the real `SubHandlers`, preserve Source registration order, `BeginEnv` lookup per key, concat lookup, then the current environment value. |
| `ConfigMacrosConfiguration.ts:155-165` | configuration initializer | Register the actual `BeginEnvItem`, expandable `active`/`macros`/`environments` defaults, and init/config processors. |

`tools/math-port/boldsymbol-configmacros-original-oracle.cjs` locks both TypeScript and installed CJS hashes and records 16 real upstream `TeX.compile` cases. The corpus covers bold tokens, nested scope, mtext exclusion, variant rewriting, standalone ConfigMacros use, string and array macros, defaults, delimited templates, active characters, environments, and malformed macro definitions. It also records actual package map registrations and callback metadata. The current native candidate is `tools/math-port/boldsymbol-configmacros-reference/native-candidate.cpp`; it boots the actual runtime, NodeFactory, BasePackage, Newcommand, and TeX, then runs `TeX::compile` including postfilters.

The native creator `FunctionRef` has two invocation paths. `NodeFactory::create` passes the actual `NativeObjectRef` and typed `Property` arguments directly, which is the enabled package path. Calling the exposed callback object through the generic Context `call` path remains explicitly unsupported until the shared runtime provides canonical NodeFactory-instance and MML-node identity adapters; the option binding reports that boundary instead of coercing values or returning a fake `undefined` result.

Native acceptance remains pending: the registered modules and 16-row TeX.compile candidate still require Root's real build/run and strict original-tree comparison. The constructor-identity BaseItems candidate is a separate pending TU and is not a Boldsymbol/ConfigMacros acceptance claim.
