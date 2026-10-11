# Root P17b — raw inherited-attribute dispatch acceptance

Root inspected the complete source public overrides and their callsites before
freezing inheritance-dispatch-contract.md and dividing implementation work.
There is one raw public facade and one virtual method-body dispatch; qualified
source super calls execute the qualified body. Source owner defaults run only
inside Base/Mi/Mo/Mstyle. Mtable performs its raw indent pre-pass before Base;
Empty remains a no-op. Child hooks and merge helpers retain the accepted Property
carrier. Explicit source empty objects are explicit PropertyList constructions.

Root reviewed Base/Empty, tuple unpacking, object operations and every changed
Mi/Mo/Mstyle/Mtable/Mtr body against pinned TS and actual CJS. Existing algorithm
responsibilities and tree/class boundaries are preserved. Six source-member
targets in the P8b/P9/P12/P14 ledgers now name inheritAttributes, documenting
the facade/body adaptation without changing source AST hashes or acceptance
counts. Agent method reports are p17b-defaulting-methods.md and
p17b-table-methods.md. Independent read-only review found no concrete algorithm
discrepancy in the declared typed-host scope; root owns executable acceptance.

The pinned CJS __read helper returns noniterable receivers unchanged, followed
by emitted indexed reads. Root's new oracle exposed an overly strict native
tuple rejection, which was corrected to typed indexed access. Strings still
iterate Unicode code points, while Object.keys/string spreading enumerate UTF16
units. Nullish indexed receivers throw; scalar/record origins remain raw until
property-key coercion. noInherit optional chains avoid eager kind reads.

Actual-CJS oracle: 249 cases and complete-state comparisons, with 350 independent
encoded source values. Cases cover five selected bodies through base pointers;
argument arity/omission/explicit Missing/null/scalars/strings/arrays/records;
default fresh-object identity; raw tuple origins, holes and indexed records;
hook failures; partial inherited/explicit/prime state; raw object-spread results,
prototype setter behavior, shallow nested alias identity and fresh output;
and ordered attributes/kind/hook observations. Source state comes from the
pinned package, never native expectations. The initial JS accessor witness was
shadowed by an own field; root corrected it with an explicit instance accessor
before using it for acceptance. Expected observations are generated thereafter
directly from unchanged MathJax execution.

All twelve affected/current generators reproduce their fixtures with --check.
Existing CJS algorithms and expected observations remain unchanged; native
witness signatures and explicit source-object construction were mechanically
adapted to the raw carrier. Focused existing Math regression passes, followed
by all 249 new cases. The complete core build/test exits 0 in build/p17b-core.log.

Fourteen isolated deliberate mutants compile and fail unchanged assertions
with test exit 1, never a build failure counted as detection. They cover missing
versus null defaults, all three default scalars, shared default objects, eager
copying, premature Mtable defaulting, omitted Mstyle entry defaults, UTF16 versus
code-point tuples, noniterable rejection, prototype assignment, merge filtering
and eager optional-chain kind access. Results are p17b-inheritance-mutations.json;
ignored per-mutant logs retain compilation and assertion evidence. The runner
verified all working implementation and fixture bytes unchanged after the run.

This accepts a correction to existing source packets, not an additional full
source file or a renderer. Local acceptance remains 46/246. Arbitrary JS
Symbol/accessor/proxy/custom-iterator objects are outside the declared native
Property host; this packet does not claim an arbitrary JS runtime. Full TeX,
Common/SVG, G0–G7 and Folia replacement remain incomplete. MathChoice has a
separate implementation packet and is not accepted by this boundary test.
