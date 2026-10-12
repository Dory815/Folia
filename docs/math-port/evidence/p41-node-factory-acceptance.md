# P41 TeX NodeFactory acceptance

Root accepts the nine NodeFactory semantic methods within the typed package
profile in p41-node-factory-contract.md. The module remains separate from the
MML constructor registry and uses the existing MML graph, FunctionRef identities,
PropertyList, Children and NodeValue. It has no parsing or layout responsibility.

Root froze declarations, allocation/identity/default-callable bindings, hosts
and 68 actual-original observations in 7c01ca0 before Luna implemented method
bodies. The compiled stub build exits 1 with 68 Unimplemented failures in
ignored build/p41-red.log. Root reviewed every method and independently reviewed
the service/ownership boundary; implementation was limited to node_factory.cpp.

The final original oracle executes pinned NodeFactory.ts/CJS with the existing
locked compiled-package loader and reproduces 78 cases under --check. Native
comparison passes all 78 assertions. Additional original cases exercise current
map values and inherited key order, nullable virtual text setters through
text/token/error creation, nullable MML factory products, registration failures,
fresh configuration after creator callback, raw child-array identity and
setChildren before properties. These are service-boundary tests; their Config
observer is not an implementation or acceptance of ParseOptions.

Review corrections:

- Native test definition records must be carried as Property inside std::any,
  as specified by the contract; direct PropertyList carriers were corrected.
  Original expected values did not change.
- Pinned compiled NodeFactory invokes func.apply, so its truthy non-callable
  failure says func.apply is not a function. The host bridge now preserves that
  observed failure rather than using the TypeScript call-expression spelling.
- The unknown-kind observation exposed a pre-existing core factory discrepancy.
  AbstractNodeFactory's direct dispatch must fail as this.node[kind] is not a
  function. Root corrected that branch in core/tree/factory.ixx and updated the
  old mutation target literal. No NodeFactory exception translation or fallback
  was added; the responsible factory boundary now reports the original error.

Nine independent semantic faults all compile and fail assertions: wrong truthy
fallback, registration on other dispatch keys, stale configuration captured
before callback, lost nullable setter return, lost token empty-text default,
omitted text append, properties applied before children, lost raw error message
and nonzero MML constructor argument count. The mutation runner restores source
and passes the baseline again; source hash/outcomes are in the mutations JSON.
Independent review runs against restored source, not temporary mutant files.

The complete normal cmd /c build_test.bat builds and passes, exit 0, in ignored
build/p41-full-core.log. folia_math contains 94 module interfaces. Core factory
regressions are included; no editor or platform code changed.

Configuration and MML factory references remain borrowed within the TeX/session
lifetime. Configuration begins Missing and MML factory begins Null. ParseOptions
will implement the narrow registration service using its own authoritative
node lists; this packet does not create a substitute options object. Physics's
token callback save/replace/restore uses the same FunctionRef identity.

The disclosed data-map and MML-product profile excludes arbitrary JavaScript
reflection, builtin creator execution returning non-MML values, and accessor or
function-prototype maps. Read-only caller audits found no enabled-package path
requiring those operations. Required TeX packages remain unchanged. Actual
ParseOptions, StackItemFactory/BaseItems/Stack, TeXParser, fonts/common layout,
shared scene/SVG and Folia integration remain open. Mermaid remains paused.
