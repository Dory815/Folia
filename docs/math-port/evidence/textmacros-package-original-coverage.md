# TextMacros package original-source coverage

This ledger is paired with `textmacros-package-original-oracle.cjs` and its generated
`textmacros-package-original-source.json`. The oracle evaluates the pinned original
CJS classes and peers; it is a source-observation artifact, not native acceptance.

| Source responsibility | Oracle observation(s) | Remaining native dependency / gap |
|---|---|---|
| `TextMacrosConfiguration.CONFIG`: inner parser configuration, init, ParseOptions, shared options/packageData, tag registration, internalMath install | `textmacros-package-configuration-identity-and-canonical-function`, `internalmath-callback-normalization-level-font-and-shared-state` | The source observer exercises actual TeX/ParseOptions/TagsFactory. Native package hookup and Context-visible TextParser fields remain integration dependencies. |
| `TextMacrosConfiguration.PREPROCESSORS`: install MmlFactory and clear ephemeral parser state | `textmacros-package-configuration-identity-and-canonical-function` | A runnable candidate invokes the actual registered filter with its real `InputJaxFilterData`, ParseOptions, and MmlFactory; Root has not wired it into the acceptance target. |
| `TextBaseConfiguration` handlers, fallbacks, item registry | `textmacros-mapping-registration-and-canonical-method-aliases`, `textmacros-method-branch-corpus` | Requires exact native BaseItem constructor identities plus actual ParserConfiguration/handler integration. |
| `TextMacrosMappings` special-character map | `textparser-spacing-specials-quotes-and-groups` | Native `MacroMap`/`CommandMap` registration uses the actual method FunctionRefs. |
| `TextMacrosMappings` command map and aliases | `textmacros-mapping-registration-and-canonical-method-aliases`, `textmacros-method-branch-corpus` | BaseMethods aliases use canonical FunctionRefs; they are not reimplemented here. |
| `TextMacrosMethods.Comment`, `Math`, `MathModeOnly`, `Misplaced` | `textparser-spacing-specials-quotes-and-groups`, `textparser-math-delimiters-and-mode-errors` | Native implementation must preserve UTF-16 parser positions, brace/error behavior, and the actual TeXParser switch to math mode. |
| `OpenBrace`, `CloseBrace`, smart quotes, `Tilde`, `Space`, `SelfQuote`, `Insert` | `textparser-spacing-specials-quotes-and-groups`, `textparser-font-size-and-text-argument-macros` | Native implementation must keep the actual environment stack and text buffer as the sole parser state. |
| `Accent`, `Emph`, `TextFont`, `SetFont`, `SetSize` | `textparser-font-size-and-text-argument-macros` | Requires actual TextParser ParseArg/ParseTextArg and NodeFactory/MML identities. |
| `CheckAutoload` | `textmacros-method-branch-corpus` | Native path throws typed RetryError carrying a real session-queue PromiseRef. JavaScript-visible Error/Promise reflection and legacy `MathJax.Callback.After` remain outside this adapter. |
| BaseMethods aliases (`Macro`, `Spacer`, `Hskip`, `rule`, `Rule`, `HandleRef`, `UnderOver`, `Lap`, `Phantom`, `Smash`, `MmlToken`) | `textmacros-package-configuration-identity-and-canonical-function`, `textmacros-mapping-registration-and-canonical-method-aliases`, `textmacros-method-branch-corpus` | Canonical FunctionRefs must be shared from BaseMethods; no duplicate implementations are acceptable. |
| `TextParser.copyLists` / package data sharing | `textparser-copyLists-transfers-actual-node-list-membership` | Requires same native `mml::Children` identities and actual inner/outer ParseOptions graph. |

The 8 observations exercise real source classes and actual TeX/ParseOptions/MML
peers where constructed. They do not claim native package parity, full TeX input,
or renderer acceptance. `textmacros-native-candidate.cpp` checks package graph
identity and runs the actual registered preprocessor but is not yet part of the
accepted native gate. Runtime package hook ordering and live Context reflection
for TextParser fields remain integration work.
