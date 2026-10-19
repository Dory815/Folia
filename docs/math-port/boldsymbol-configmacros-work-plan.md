# Boldsymbol and ConfigMacros ownership and acceptance

These are two complete, separate Source package responsibilities, in addition
to the owner's continuing BaseItems/Begingroup integration fixes. Neither is
accepted until its real native behavior matches the pinned original.

| Source | Native module / files | Required responsibility |
| --- | --- | --- |
| `ts/input/tex/boldsymbol/BoldsymbolConfiguration.ts` | `folia.math.input.tex.boldsymbol.configuration`, `input/tex/boldsymbol/configuration.{ixx,cpp}` | Full BOLDVARIANT table, canonical Boldsymbol callback, actual token creator replacement, fixBold list/property ownership, ordered rewriteBoldTokens postfilter and Configuration registration |
| `ts/input/tex/configmacros/ConfigMacrosConfiguration.ts` | `folia.math.input.tex.configmacros.configuration`, `input/tex/configmacros/configuration.{ixx,cpp}` | Three actual maps, local configuration append at priority 3, init/config callback order and receiver contracts, live option definitions, templated/default-argument macros, active characters, actual BeginEnvItem registration, environment concatenation and expandable option semantics |

Use the actual parser facade, existing NodeFactory, Configuration, maps,
Newcommand callback table and Source BeginEnvItem class. Do not put either
package in the parser facade or dispatch by command name in TexParser. Do not
silently enable a package when another module is loaded. Helpers can remain
private in the matching Source file; do not invent Source modules that do not
exist. A real missing shared capability must be reported with its Source call
site and the existing dependency contract.

The owner writes the full implementations, Source method ledger and actual
native candidate independently. Root owns CMake, shared builds and final
acceptance. Preserve the existing BaseMethods corpus, all its package lists and
inputs, and all previously frozen expected results.

Compare real native trees, explicit/inherited attributes, fixBold property
removal, list identities, handler priorities, configured Macro function and
argument identities, and exact errors against actual pinned CJS. Include
nested boldsymbol, existing variants, mtext exclusion, creator invocation and
postfilter timing; configured active characters, string/array definitions,
parameters/default arguments/templates, local environments, expandable option
merge behavior, and separate TeX instances. Check new custom callbacks through
the real FunctionRef/Context binding, preserving their Source lengths and
argument/receiver adaptation. Complete TeX.compile comparisons are required;
a direct parser tree without postfilters cannot accept Boldsymbol.

This packet adds neither common layout nor SVG/Folia acceptance. No fallback,
QuickJS call, expected-result deletion or alternative AST is permitted.
