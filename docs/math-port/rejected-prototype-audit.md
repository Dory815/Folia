# Why the first prototype was rejected

This is a record of root's rejection, supported by direct upstream inspection
and three read-only agent audits. Nothing below is an accepted implementation
or evidence that native MathJax parity has been achieved.

The prototype and its tracked diff are preserved in
`build/math-port-rejected-prototype`. Production sources have been restored to
the preceding font-fix commit. The full core test suite passed after restoration.

## Structural failures

The prototype's `MathNode` was a custom partial TeX structure. It combined
tokens that MathJax separates into `mi`, `mn`, `mo`, flattened tables, and
substituted a few fields for MML attribute/property inheritance. A single
recursive parser replaced Configuration/TokenMap/Stack/StackItem dispatch.
One `Engine::Place` switch then interpreted styles, measured, stretched and
generated drawing commands. These are separate upstream responsibilities.

Root checked `ts/input/tex.ts`: its postfilters have ordered normalization
responsibilities. `Attributes.ts` explicitly separates four levels of values.
`MmlNode.ts` assigns TeX classes and inherited display/script semantics.
`output/common/Wrapper.ts` implements wrapper lifecycle and cached geometry;
per-kind common wrappers provide formulas. SVG wrappers then perform placement
and output-specific assembly. Those mechanisms were not reproduced merely by
giving the prototype similarly named structures and using upstream constants.

## Concrete behavioral failures

- Parsing lacked the upstream deferred stack mechanisms for `\over`, prime
  coordination, `\middle`, environments and styles. Column specifications and
  row/cell structure were discarded. Optional macro defaults and registration
  semantics were absent.
- Radicals used an arbitrary `.6` width and scaled the ordinary radical path.
  Tables used generic `.5` column and `.2` row spacing. Accents used a generic
  gap. These formulas were neither complete upstream ports nor validated.
- Fraction/script equations partially copied upstream formulas, but operated
  on a different semantic model and reduced BBox. Copying equations without
  their attribute, scale and wrapper prerequisites did not establish parity.
- Font outlines were source-derived records, but the resolver did not preserve
  the complete variant graph, link/SMP pointer resolution, remap maps or
  extension merge/index semantics. `LookupSymbol` was still an empty function.
- The comparator disabled production line breaking and compared only the
  first native fragment. It did not completely represent viewport transforms,
  inherited clips, paints, errors and SVG elements. A broad `.002em` tolerance
  could hide differences. Nonempty SVG/unit smoke tests did not catch this.

## Process failures and correction

Root dispatched broad parser/layout/font tasks before freezing a reviewed
source inventory, semantic interfaces, per-file responsibilities and complete
acceptance tests. Root then attempted to correct missing architecture with
successive patches and shared fields. That allowed a growing approximation to
be mistaken for a clean port and caused premature production wiring.

Correction: preserve the rejected work as evidence, restore the single current
application backend, define the architecture and source inventory first, and
dispatch dependency-ordered packets with immutable root-owned gates. Agents
cannot choose a smaller feature profile, modify expected results, or certify
their own completion. See `architecture.md` and `tasks.md`.
