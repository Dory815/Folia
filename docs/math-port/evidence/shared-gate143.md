# Gates142–143: complete layout profile matches; renderer acceptance remains open

The shared graph now registers the actual Bussproofs configuration through
its existing ParserRuntime and RuntimeModules. No second parser, document or
layout service is introduced. Package callback names and arities are attached
to the actual FunctionRef before its first Context binding. The actual
ParserConfiguration configurations PrioritizedList now exposes its native
owner, items and records with stable Context identities.

Gate142 captures 1,995 inputs without changes, but fails compilation because
the new projection implementation imports a partition using an invalid full
module name. Gate143 changes that declaration to `import :runtime;`; all 53
targets compile, and the 1,995 captured inputs remain unchanged through every
evaluation. Executables are run directly from build/core and hashed before
and after use. No executable or directory copy is made.

The locked complete layout comparison now matches **75/75**, with all **38/38**
packages actually registered and no configuration warning. This comparison
includes the expected error records; it does not mean 75 successful formulas
or complete SVG coverage. Its native observation SHA-256 is
`cce35982d3e2e4db6df5b969e9608fbaea5202b1521310942120f4477b269453`.
The public API comparison remains 9/9, and the complete texttip SVG is exact.

Host, Parser, Foundation, Regexp and LinkedList suites pass. Complete Core
has two failures in newly added tests: the ordinary-formula Scene assertion
expects ASCII x although the Source SVG uses mathematical italic U+1D465;
the Bboldx metadata fixture raises an option exception during initialization.
These failures are not suppressed. The package metadata comparison itself
has progressed beyond the previously incorrect callback names and arities.

Original Bussproofs, utility, expanded (21/21) and session comparisons pass.
The repaired StackOwners fixture now compiles through MathItem.compile and
reads its actual root, but a later check fails with
`MML Push did not use the bound Context array owner`. This still requires a
Source comparison; the fixture is not accepted as passing. ParserFields
replacement, alias and OpenBrace/CloseBrace observations pass.

Fonts remain 13/16 and next-six remains 18/20. Their first difference now
reaches `configurations.items[0].item.preprocessors[0][0].length`, Source 1
versus native 0. Two standalone XML foreignObject namespace differences also
remain. Other package, parser projection, host/layout and attribute
comparisons pass. There are 25 package captures (only StackOwners fails) and
20 terminal comparisons (only XML, fonts and next-six fail).

All evaluation processes are terminal. Evidence under build/math-port includes
the Gate143 input manifests and empty post-evaluation change report,
diagnostic-exits-143-root.json, package-captures-g143.json,
checkpoint-comparisons-g143.json, the complete layout report and individual
executable provenance. The final state records evaluationExitCode 1, so this
checkpoint is not an accepted renderer release.

Factory raw field semantics, replaceable list owners, live XML identity,
lifetime reclamation and cancellation remain under implementation or review.
Folia still uses its math QuickJS worker; Mermaid remains paused.
