# Root P17a local acceptance — native TeX token/type foundation

Four complete pinned source responsibilities: Token.ts (all eight members),
Types.ts (six aliases with the frozen callable/storage adaptation),
HandlerTypes.ts (all 12+4 string enum entries), and TexConstants.ts
(complete global, 16 groups and 98 string entries). Source hashes are checked
by tex-token-reference.cjs against scope and the locked complete CJS package.
The root AST member ledger covers eight methods plus the constant global;
type aliases and enum declarations have no executable methods.

Root read every source and reviewed each native method. Token storage and
getters preserve raw values and graph identity. Macro defaults only Missing
to a fresh array; its callback identity and explicit null/scalars are preserved.
C++ named-module cycles are removed with independent Token, types and Macro
modules. ParserContext and StackItemContext are stateless identity interfaces;
the future actual parser/stack classes own their source state and behavior.
No parser, map or TeX package implementation is claimed by these interfaces.
Constants retain original order, names and source spellings, including inital
and LINBREAK. Exported table handles retain the source's shared mutable objects.

Root actual-CJS oracle: 44 cases, 161 assertions (128 source-derived assertions
plus 33 native callable protocol assertions), 155 encoded source value records.
Coverage includes missing/null/scalars, signed zero, nonfinite numbers, Unicode
and lone surrogates, objects/arrays, identity and external mutations, distinct
fresh default arrays, callback versus raw non-callables, all constant values
and order. Native protocol witnesses check parser, Token, stack and symbol
identity, heterogeneous std::any arguments, and distinct Missing/false results.
Generation and --check execute the pinned CJS; no native implementation is used
to generate expected values.

14 isolated mutants compiled and failed unchanged assertions, with test exit 1:
constructor assignment omissions, each getter, missing-array initialization,
shared default arrays, null normalization and argument cloning. Original files
and fixtures were byte-checked unchanged. Evidence is p17a-token-mutations.json;
ignored build/math-port-tex-token-mutations contains individual compile/test logs.
Focused build and full core runner passed; full log build/p17a-core.log.

This accepts four local source files and advances scope to 46/246. Full
configuration/parser/stack, Common layout, SVG and G0–G7 remain incomplete.
Folia continues to use its existing math runtime until the full replacement
passes; no alternate production path or feature flag was added.
