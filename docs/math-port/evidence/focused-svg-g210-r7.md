# Focused native SVG evaluation, G210 r7

Observed 2026-10-19 local. This records the current working tree built by the
G210 r16 focused build; it does not establish acceptance of an isolated Git
commit or replace the full Gate209 evaluation.

Pinned MathJax source: 4.1.3, commit
`fb987178c2d279a99b0db5ea02e751691703955e`.

## Results

- API: 9/9 matches.
- Complete SVG bytes: 144/150 matches, acceptance false.
- Remaining formulas: `cases-numbered`, `empheq-left-brace`,
  `mathtools-prescripts`, each in continuous and fresh-session modes.
- Individually executed Factory/MML tests: 43 pass, 6 fail.
- The separate r16 shared canonical MML module test and the existing
  cross-Parser class-static lifetime test pass.
- All 2,176 recorded inputs and all three captured executable hashes remain
  unchanged through public r7 evaluation.

The remaining Factory/MML failures concern deferred callback lifetime,
replacement nodeMap behavior, constructor lifetime, registered class
prototype initialization, late field assignments and accessor-backed native
operations. The last three also fail in the r16 focused test selection.

## Provenance

Ignored generated records live under `build/math-port/`:

- `factory-focused-g210-r16-inputs.json` and `...-result.json`.
- `factory-public-focused-g210-r7-result.json` records actual process exits.
- `factory-public-focused-g210-r7-svg-comparison.json` compares against
  `profile75-full-svg-source-clean-g150-root.json`.
- `factory-public-focused-g210-r7-api-comparison.json` compares against
  `math-api-original.json`.

| Executable | SHA-256 |
| --- | --- |
| FoliaMathHostTests.exe | `ec52b90634d1b01b39fe61337f4d3cd8e1588b024f8645acc428b4b315076af2` |
| FoliaMathApiDump.exe | `1bcfb14fe460731099b36e4d219c6e20a144a8802621f9479bec513d574ee3fb` |
| FoliaMathProfile75FullSvgDump.exe | `00377ad88a6ee5b0e11f20f42a9cfa0b1d9a2b74f643101d1dc5fc8280e8895e` |

The Core executable was not rebuilt by this focused build. Public SVG parity,
all package/lifetime checks and the full native test graph are still required
before replacing Folia's math QuickJS path. Mermaid work remains paused.
