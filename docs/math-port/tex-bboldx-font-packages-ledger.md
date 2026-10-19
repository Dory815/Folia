# BBM, BBoldx, and Dsfont package port

This candidate follows the locked MathJax 4.1.3 package files:

| Source owner | Native owner | Responsibility |
| --- | --- | --- |
| `input/tex/bbm/BbmConfiguration.ts` | `src/math/input/tex/bbm/configuration.{ixx,cpp}` | `BbmMethods` callables, `bbm` CommandMap, options, and Configuration |
| `input/tex/bboldx/BboldxMethods.ts` | `src/math/input/tex/bboldx/methods.{ixx,cpp}` | canonical Base/TextMacros aliases, font selection, math/text token handlers |
| `input/tex/bboldx/BboldxMappings.ts` | `src/math/input/tex/bboldx/mappings.{ixx,cpp}` | all eight CharacterMaps and both CommandMaps, preserving Source names, keys, and values |
| `input/tex/bboldx/BboldxConfiguration.ts` | `src/math/input/tex/bboldx/configuration.{ixx,cpp}` | text-bboldx Configuration, option defaults, priority 3, and live TextMacros registration callback |
| `input/tex/dsfont/DsfontConfiguration.ts` | `src/math/input/tex/dsfont/configuration.{ixx,cpp}` | `ChooseFont`, CommandMap, option defaults, and Configuration |

`tools/math-port/tex-bboldx-font-packages-original.cjs` validates the full pinned source lock before loading modules and records parser inputs, resulting trees/errors, package data, map entries, callable metadata, and aliases. The original twelve rows remain unchanged; four additional rows cover explicit BBM option initialization, simultaneous `bfbb`/`light` precedence, text-mode dotless aliases, and text-mode light selection. The native candidate emits the same records, and `compare-tex-bboldx-font-packages.cjs` compares every field without normalization or omission.

The extension-font data remains owned by the NewCM font modules. These TeX packages only select their Source font identifiers through the canonical BaseMethods/Font methods.

The Source oracle completes all 16 rows after validating 246 locked source files. Its alias witnesses confirm the shared ChooseFont/character/delimiter/BaseMethods identities. The maps contain four BBM commands; four 36-entry character maps; four six-entry delimiter maps; six commands in each math/text BBoldx command map; and one Dsfont command. `BboldxMethods` is retained once on the `ParserRuntime`, because its character maps and command maps share Source callable identities. BBM and BBoldx selectors preserve missing/raw arguments through their BaseMethods call instead of converting absent arguments to empty strings.

The isolated Gate101 package comparison compiled the current BBM implementation and the complete package observer into `build/math-port/isolated/font_packages`, using the captured Gate101 module interfaces and libraries without rebuilding shared targets. The executable ran all 16 cases and exited 0. The strict comparator matched 13/16: BBM's `mathversion{bold}` transition now selects `-bbm-bold`, and the three remaining records are only the live `textmacros` packageData projection in the original text case, the dotless-alias text case, and the light-option text case. No tree, error, option, command-map, character-map, delimiter-map, or alias difference remains in the other thirteen records. These are isolated observations against Gate101 dependencies; parser projection work is still in progress and must be followed by a current shared compile and comparison before acceptance. Process details and exact source, IFC, library, and executable hashes are recorded in `build/math-port/isolated/font_packages/provenance.json`; raw native JSON and stderr are alongside it.
