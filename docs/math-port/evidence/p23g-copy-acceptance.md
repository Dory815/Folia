# P23g Root copy acceptance

Root accepts only the restored `AbstractMmlNode::copy(bool)` body. The historical
54/246 source-file tally is unchanged. TeX, mathematical layout, SVG, complete MML,
the full core suite and Folia integration remain open.

## Original observations and executable comparison

Before delegation, the complete 89-module registry graph compiled and the actual
copy stub failed all 108 Source299-bound cases and all 32 additional metadata cases.
The contracts and immutable original generators were committed in d8ff2fc.
After delivery Root added six independent original boundary cases. Two exposed real
differences: a Null factory product and a Null inferred-row array had incorrect
TypeError messages. Root corrected both at their actual dereference boundaries.
These six cases were added after implementation; they are not claimed as pre-agent red.

Final `build/math-port-mml-copy-mutations/baseline-test.log` passes 108, 32 and 6
assertions in three independent Boost.UT suites (146 assertions / three tests).
`results.json` records 17 effective, compiling semantic mutants, all detected by
assertion failures; crashes and compile failures do not count. A preliminary
mis-scoped mutation anchor failed compilation, was rejected, and was corrected
before the complete final run. Working source and gold bytes stayed unchanged.

The mutants discriminate falsey Null materialization, direct push versus append,
root/child id scope, attribute-store identity, property-store replacement and
nested shallow identity, singleton inferred Get, captured/live child iteration,
destination rebinding, inherited attribute exclusion and both Null error messages.

Original generators reproduce the 108 binding, metadata SHA
`b9be470fecd1e2432639f036aebb41d9b91f986949890ec78baf0915dce8276b`
and boundary SHA
`14f4b2a0b46af33f11bb3d03bae1a75b820458fc3159757c9dc08790c20c6c95`.
Together with the existing 62 append bindings, 170/299 rows of that sparse corpus
now have native counterparts; 129 remain. Supplemental metadata and boundary rows
are separate corpora. `superscript-first-copy` still needs actual NodeUtil.setChild.

## Regression and limits

Root re-ran constructor (91 assertions / two tests / six mutants), append
(62 / one / five mutants), and actual sparse factory binding (116 / ten / one
graph-link mutation). All pass. The runner adds multiple immutable fixture TUs
and checks each independent Boost.UT suite summary; existing callers retain their
default behavior. No generic Tree/Wrapper algorithm was changed.

The typed normal-node family guard is not arbitrary JavaScript factory equivalence.
Normal explicit attributes and nested Property records are covered; arbitrary
accessor-injected attributes and every function/object attribute value are not.
The full core build last failed at old MML observers (p23f-full-core-status.md);
this focused graph does not override that result or authorize Folia integration.
