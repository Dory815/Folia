# Root NodeUtil selected child-service acceptance

Accepted only getChildAt/setChild/copyChildren under the file/API duties frozen in
p23i-node-util-child-contract.md. Root reviewed the delivered three bodies:
current Get/Set, tagged values, slot-before-parent ordering, truthy-only virtual
parent writes, captured source with live length and virtual this.setChild all
retain the original operation order. The receiver owns no graph or extra state.

All **163 actual original observations** pass on the complete **90-module/all-unit
graph**. The same actual callable stubs had failed all 163 assertions before
delegation. Expected Source and generated native observations reproduce unchanged,
SHA `660c46a99de4f6cfa2f2ea31838fca3658b38cbdd75c04401c87cca29814d705`.
Root's independent method-scoped mutation run detects **23 compiling semantic
faults**, including Null/Undefined normalization, ignoring prototypes, wrong
nullish error members/indices, parent-before-slot writes, falsey skipping,
unconditional/lost/nonvirtual parent writes, append instead of indexed Set,
snapshot length, rebound-source traversal, missing virtual receiver dispatch,
prevalidating an empty destination and replacing the target handle. Every mutant
fails comparisons by assertions; working source and fixtures are byte guarded.
Results: build/math-port-node-util-child-mutations/results.json, SHA
`05ca7d676a8491abc9fbe38453b7e65d64a7849ac22be91f71f2d18e0c21da4c`.
The relevant full actual MML-return comparison was rerun with the added module:
189 assertions/three tests pass, and its inherited-core-slot mutant is detected.

Source and target arrays, captured handles, own/has/Get states, lengths, child
identities, parent values, raw result/error and callback order are observed.
The 163-case corpus and its domain limits are described in the frozen contract;
it does not certify arbitrary JavaScript property keys, proxies or foreign nodes.
The utility's shared native TreeCallGuard is a host resource limit, not a new
syntax or mathematical layout rule.

No whole-file NodeUtil acceptance follows from these three selected services.
Source299's four dependent superscript consumers still require inheritance.
Whole-file count stays 54/246 and Source299 binding stays 245/299. Full-core
stale observers, remaining MML/TeX/Common/fonts/SVG and Folia are still open.
