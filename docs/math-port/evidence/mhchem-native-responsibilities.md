# Mhchem native responsibility map

The reference is MathJax 4.1.3 at
`fb987178c2d279a99b0db5ea02e751691703955e`. Its package import
`#mhchem/mhchemParser.js` resolves through `package.json` to
`mhchemparser/esm/mhchemParser.js`; the compiled CJS package's own imports map
resolves to `mhchemparser/dist/mhchemParser.js`. Both repository dependency locks
pin mhchemparser 4.2.1. The installed package includes the original TypeScript,
declarations, ESM, CommonJS and Apache-2.0 license; it has no runtime dependencies.
Exact input and generated-data hashes are in `mhchem-source-lock.json`.
Both locks record the npm distribution integrity as
`sha512-kYmyrCirqJf3zZ9t/0wGgRZ4/ZJw//VwaRVGA75C4nhE60vtnIzhl9J9ndkX/h6hxSN7pjg/cE0VxbnNM+bnDQ==`.

Source responsibilities are assigned before implementation:

| Original responsibility | Native file |
| --- | --- |
| `MhchemConfiguration.ts`: commands, characters, replacements, Machine and relmo | `src/math/input/tex/mhchem/configuration.{ixx,cpp}` |
| `mhchemParser.ts`: toTex, createTransitions and parser.go | `src/math/dependencies/mhchem_parser/parser.cpp` |
| pattern literals and all pattern functions, findObserveGroups and match_ | `patterns.cpp`, `patterns_data.inc` |
| generic actions and fourteen machines' local actions | `actions.cpp` |
| every machine's original ordered transition table | `transitions_data.inc` |
| `_mhchemTexify`: go, _goInner, _go2, arrow/bond/operator maps | `texify.cpp` |
| `mhchemParser.d.ts`: parsed records, Buffer, Match and transition types | `parser.ixx` |

The native package keeps one configuration source pair with typed Machine/relmo
callbacks, immutable replacement data, and original command/character maps.
Initialization receives ConfigurationHandler and MapHandler; RuntimeModules,
ParserRuntime and generic exported object/callable records are withdrawn.
Replacement helpers receive the two original TokenMap references explicitly.
Machine obtains retained handler maps, preserving their ordinary mutations;
a later registry replacement must not redirect its lexical map references.
Ownership uses ordinary native map references with no cycle or synthetic JS
module object. See native-raii-architecture.md for the current integration plan.

`mhchem-generate-data.cjs` executes the original only as a development data
transcription tool. It writes the literal regular-expression definitions and
expanded ordered transition records, with source hashes. No function bodies,
chemical examples, expected TeX or renderer output enter the generated native
data. Native C++ owns pattern matching, buffer mutations, state transitions,
recursive parsing, error construction and TeX generation. Generated TeX is fed
back into the existing TeX parser, then its ordinary MML/Common/Scene route.

The current target registers these original owners. Independent canonical
witnesses are `tools/math-port/mhchem-parser-source.cjs` and `mhchem-source.cjs`;
neither requires ignored draft scripts or corpora. Root bytechecked both from
outside the repository and ran the full current math target successfully.
Acceptance counts and limits are in `native-raii-acceptance.md` and the current
checkpoint in `HANDOFF.md`; historical isolated draft checks are not acceptance.
