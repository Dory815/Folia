# AMS and Newcommand package ownership and acceptance

This is part of the existing complete enabled-package TeX profile, not a smaller
replacement profile. Root owns shared contracts, CMake, builds and final
acceptance. The assigned Luna owns implementation and differential verification
of both complete packages, including their dependency relationship.

Use the pinned MathJax 4.1.3 Source, commit
`fb987178c2d279a99b0db5ea02e751691703955e`. Read actual files under
`tools/mathjax-bundle/node_modules/@mathjax/src/ts/input/tex/{ams,newcommand}`;
original executable witnesses use the matching pinned CJS. Record exact file
hashes and member-to-native mappings before claiming parity.

Keep these Source responsibilities in separate files/modules under
`src/math/input/tex/ams` and `src/math/input/tex/newcommand`:

| Source | Native responsibility |
| --- | --- |
| AmsItems.ts | Actual MultlineItem and FlalignItem subclasses and reductions |
| AmsMethods.ts | Source AMS ParseMethod table and method bodies |
| AmsMappings.ts | Source token, delimiter, command and environment maps |
| AmsConfiguration.ts | Actual AmsTags class and loaded-module Configuration |
| NewcommandItems.ts | Actual BeginEnvItem subclass and reductions |
| NewcommandUtil.ts | Macro parsing/substitution and Source-owned dynamic tables |
| NewcommandMethods.ts | Source command bodies and mutation/error ordering |
| NewcommandMappings.ts | Source command map registration |
| NewcommandConfiguration.ts | Idempotent NewcommandConfig and package Configuration |

Use standalone modules importing `folia.math.input.tex.parser`, as existing
BaseMethods does, where there is no genuine additional cycle. Do not make a
generic command switch stand in for these Source files. Do not add primary
parser partitions to hide an unnecessary module dependency. Identify actual
cycles or missing shared API contracts explicitly before changing ownership.

Reuse actual BaseItems, NodeFactory, TokenMap/MapHandler, Tags, FunctionRef,
ParserRuntime, Property, Options, RegExp and Unicode services. AMS's use of
NewcommandConfig is real: preserve its once-per-ParseOptions packageData guard,
table mutation, insertion priorities and original callback arguments. Every
package registers canonical actual constructor and callback identities once
per loaded runtime. Classes retain genuine factory owners and Source
initialization/rest-argument semantics. No fake peers, snapshots of native
objects into Context records, no-op pointer deleters, fallback package bodies
or changes to frozen Source results are allowed. Coordinate generic class
binding needs with the BaseItems owner and Root; package method algorithms are
the package owner's independent responsibility.

Supply a deterministic complete original oracle, actual C++ candidate fixtures
and strict comparator using the existing real runtime graph. Owned tools,
fixtures and evidence use `ams-` and `newcommand-` prefixes. Cover all members,
normal and malformed inputs, macro definition/redefinition, parameter and
optional argument substitution, protected/recursive macro limits, dynamic
table aliases, new environments, AMS symbols and delimiters, matrix/align/
cases/binomial/multline/flalign/tag/label/reference branches, row/column limits,
spacing, error IDs and mutation state after exceptions. Observe complete MML
trees (attributes, properties, ordered children), real parser cursor/state,
table and class identities. Preserve every case in the existing 37-case
BaseMethods oracle, including enabled extensions.

Compilation and registered method names alone do not pass. Root runs the
actual native candidates, reviews Source responsibilities and meaningful
semantic faults, and records acceptance only after required behavior agrees.
Remaining enabled packages, common layout, SVG and Folia integration keep
their full independent acceptance gates.
