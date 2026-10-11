# P19c — pinned-source regex call-site audit

This is a source inventory for the pinned MathJax 4.1.3 tree at commit
`fb987178c2d279a99b0db5ea02e751691703955e` and the 38-package profile in
`tools/mathjax-bundle/entry.cjs`. It does not select an engine or claim native
regex compatibility or parser acceptance.

I parsed all 246 `scope.json` source files with the pinned TypeScript 5.9.3
compiler API. Every scanned file hash matches its `scope.json` hash. The
machine-readable report records 227 regex literals, 17 `RegExp` constructor or
call sites, 216 regex-valued method sites, and three direct `lastIndex`
observations. It records each hit's source path, file SHA-256, line, member,
expression, and relevant arguments. The AST pass follows regex-valued locals
and properties within each file; the mhchem `Map` destructuring call is added
with an explicit note because its regex value crosses a collection iterator.
Method-name and alias inference can still over- or under-approximate unusual
indirect calls, so the JSON is evidence for review rather than a completeness
claim about arbitrary runtime values.

## Core input and token-map paths

`BaseConfiguration.ts` (SHA-256
`578d2c6b03f2f3e902476a0b73a5323b12262215fcbdc831a1135c1d9c1737ad`) supplies
the enabled base defaults at lines 219–229: `numberPattern` is
`/^(?:[0-9]+(?:\{,\}[0-9]{3})*(?:\.[0-9]*)?|\.[0-9]+)/`, `initialDigit` is
`/[0-9.,]/`, `identifierPattern` is `/^[a-zA-Z]+/`, and `initialLetter` is
`/[a-zA-Z]/`. Its CONFIG hook at lines 162–169 first copies truthy legacy
`digits` into `numberPattern`, then registers `initialDigit` and `initialLetter`
with `RegExpMap`.

`TokenMap.ts` (SHA-256
`0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e`),
`RegExpMap.contains` line 167 calls `_regExp.test(token)` without resetting
`lastIndex`. The default initial patterns are non-global and non-sticky, so
they do not carry state. External TeX configuration can supply a RegExp with
`g` or `y` before the base CONFIG hook; then `.test`'s built-in `lastIndex`
state is observable across token probes. A later `\setOptions` update to
`initialDigit`/`initialLetter` changes the option slot but does not replace the
already-constructed `RegExpMap` instance.

`ParseMethods.ts` (SHA-256
`1cdc4c6828078cd40863269864cdd6b7e5d22024e2ed36f4eeafb673781a5a70`) reads
`parser.configuration.options['numberPattern']` at lines 80–83 and passes it
to `String.match` on each digit parse. This is a live option read. The default
has no `g`/`y` flag; a configured RegExp can change match result shape/state
under standard ECMAScript `String.match` semantics.

## Dynamic pattern sources

The enabled `setoptions` package has a reachable dynamic source path. In
`SetOptionsConfiguration.ts` (SHA-256
`ccaa243137c68208e7f442013aa5dc6cc04b735ac3e0ca099ea8f30be557790d`),
`SetOptionsUtil.filterValue` lines 126–139 accepts a slash-delimited string
matching `/^\/(.*)\/([dgimsuvy]*)$/`, then calls `new RegExp(match[1],
match[2])`. The parsed pattern and flags come from TeX input; invalid syntax or
flags can throw from the constructor. `SetOptions` applies this to accepted
option values at lines 148–165. In particular, setting the `tex` option
`numberPattern` changes the pattern read by `ParseMethods.digit` above. This is
production-reachable syntax in the configured profile, not merely a JavaScript
host injection. The `d` and `v` flags are admitted by this parser even though
the cited `numberPattern` consumer only calls `String.match`.

Other generated or user-configurable patterns include:

- `FindTeX.ts` (SHA-256
  `65355d94cac0c1e8a790313c52e231212733b5a98d6f6697c2c7864812a72944`):
  dynamically joins configured delimiters into a global `start` RegExp
  (lines 130–140), builds close patterns from delimiter text (`endPattern`,
  lines 161–168), and executes them (`findEnd`, line 191; `findMathInString`,
  lines 221–254). It explicitly sets `pattern.lastIndex` before `exec`, resets
  `this.start.lastIndex` to 0, and advances it after a recognized item. These
  are the three direct `lastIndex` reads/writes in the scanned TS.
- `BaseMethods.ts` (SHA-256
  `7336240c8f9c5037f9fdcd5a84ea079312854f1ef3f406aeb5a8b6a8ff765cda`):
  constructs an environment-ending regex from environment text at lines
  1638–1640 and uses it in `String.match` at line 1687. The pattern is
  non-global/non-sticky; the source escapes `*` before interpolation.
- `HTMLDomStrings.ts` (SHA-256
  `e4a7da03465d6e69ebdca829b007d9905f180b0d4107ea5a1c5a44737c2712c8`):
  compiles `skipHtmlTags`, `ignoreHtmlClass`, and `processHtmlClass` from
  option arrays in `getPatterns`, lines 150–160, then calls `.exec` at
  lines 242, 247, and 253. These values are host-supplied HTML configuration.
  The file is in the profile dependency closure, but `entry.cjs`'s
  `render(tex, ...)` calls `document.convert` and does not run the DOM
  `findMath` scan; treat this as a reachable handler/host path, not a proven
  call on the formula-render path.
- `mo.ts` (SHA-256
  `2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605`)
  builds four static patterns at lines 92, 110, 145, and 169 and uses them at
  lines 444, 520, 538, 578, and 589 for operator-name, pseudo-script, prime,
  and accent classification.

## Replacement and mhchem

`MhchemConfiguration.ts` (SHA-256
`bacc438afa26698e9e7d8ad6e4a6094eefa12f0d265c4384d60c0dbb474c4ace`) defines
six global regex literals at lines 66–88. `Machine` calls `tex.replace(pattern,
name as string)` at line 105 while iterating the `MhchemReplacements` map.
Most map keys are replacement strings using `$1`/`$2`; the final arrow rule's
key is a function (the TypeScript cast does not change its runtime value), so
the call also requires the `String.replace` callback form, including capture
arguments and callback-return replacement semantics. `Machine` is registered
as `\ce`/`\pu` through the enabled `mhchem` package. Its regex set is
statically source-defined; the parser's generated TeX remains input-dependent.
The imported `#mhchem/mhchemParser.js` is package code outside this 246-file
TS closure, so this report does not inventory regexes internal to that parser.

Across the closure, the identified operations are `RegExp.test` (1 site),
`RegExp.exec` (5), `String.match` (107), `String.replace` (71), and
`String.split` (32). No regex-valued `String.search` or `replaceAll` call site
was found. `String.split` must retain ECMAScript capture-result behavior for
patterns with capturing groups; `String.replace` must support replacement
tokens as well as callback replacements. The JSON also records ordinary
regex literals used by utility validation/formatting.
No pinned-source reads of `RegExp.flags` or `RegExp.source` were found; the
three property observations are `lastIndex` in `FindTeX`. The report records
callbacks syntactically visible in regex replacements plus the mhchem
collection-iterator callback case described above.

## Reachability boundary

Regex literals in the 246-file closure include core MML, output, DOM, and
utility code in addition to the enabled TeX parser. The source inventory does
not equate inclusion in the dependency closure with execution for every
formula. Static TeX and SVG operations are distinguishable from external DOM
configuration (`HTMLDomStrings`), externally configured options, and
input-driven `\setOptions` patterns as described above. The package parser
behind mhchem is outside the pinned TS scope file list. No engine, subset, or
fallback decision is made here.
