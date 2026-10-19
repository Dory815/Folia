# TexParser pinned-source coverage

Oracle: `tools/math-port/tex-parser-original-oracle.cjs`; pinned source is
`.reference/MathJax-src/ts/input/tex/TexParser.ts`, checked against the installed
MathJax 4.1.3 CJS and the locked dependency set. Generated source-only output is
`tex-parser-original-source.json`. The oracle constructs the real `TeX`,
`ParseOptions`, `TexParser`, `Stack`, handlers, `StackItemFactory`, MML factory,
and MML nodes; it does not synthesize a parser or configuration.

| Source responsibility | Original observations |
| --- | --- |
| Constructor, env `isInner` deletion/copy, initial parse, stop item, parser push | `constructor-real-parse-empty-token-macro-and-mml`; `constructor-env-isInner-own-test-delete-and-stack-copy`; `constructor-malformed-env-null-before-parser-push` |
| Emitted prototype own methods and accessor metadata | `texparser-emitted-prototype-method-metadata` |
| `options`, `itemFactory`, `tags`, `string` accessors | `parser-getters-setter-and-default-string-replacement` |
| `parse`, `updateResult`, `composeLatex`, `composeBraces`, `composeBracedContent` | Constructor parse samples exercise normal parse path; focused raw-handler callback side-effect witness is pending native implementation integration |
| `lookup`, `contains`, `toString` | `parser-parse-lookup-contains-tostring-live-subhandlers` |
| `Parse`, `Push`, `PushAll`, `mml`, `trimTex` | constructor parse corpus (including exact `x+1` MML tree); `parser-push-and-pushall-actual-inferred-mml-and-stackitem` |
| delimiter conversion, code point, space skipping, CS and argument scanning | `parser-getnext-spaces-ascii-unicode-and-end`; `parser-getcs-control-sequence-ascii-astral-and-empty`; `parser-getargument-single-control-group-nesting-and-errors`; `parser-getbrackets-matching-escapes-braces-default-and-errors`; `parser-getdelimiter-base-delimiters-and-errors`; `parser-getdimen-braced-and-unbraced-source-consumption`; `parser-getupto-braces-commands-token-and-error-state`; `parser-delimiter-arg-and-star-state` |
| Nested parser entry points, node factory forwarding and token variant | `parser-parsearg-parseupto-real-nested-parser-and-latex`; `parser-create-real-node-token-mathvariant-and-factory-errors` |

Known coverage limit: the private LaTeX update helpers are exercised through
real constructor parsing, but need an additional focused corpus for braces,
sub/superscript accumulation, existing `data-latex-item`, and ordered node
attribute side effects before Native acceptance. `toString` currently observes
live handler replacement, not handler-key enumeration mutation during iteration.

The dedicated native candidate now boots one real `Context`, `ParserRuntime`,
`TeX`, BasePackage, `ParseOptions`, and MML factory, then checks the `x+1`
TexParser result against the original inferred-row tree and data-latex attribute.
The larger candidate also drives actual TextParser outputs through MML. Candidate
compilation and execution are tracked by Root's `FoliaMathParserTests` gate; this
does not claim full handler-map or Context-field reflection parity.
