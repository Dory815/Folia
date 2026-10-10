# Root acceptance audit of P8b MmlNode

Status: local root acceptance passed. This packet is the MML base
hierarchy only, not a complete mathematical tree/parser/layout/rendering gate.

Root froze declarations, native host boundaries, constructor/default bindings,
task file ownership and reference fixtures before agent implementation in
commit 0b315bc. Three agents implemented disjoint normal, token/layout/base,
and empty/text/XML files. An independent read-only agent review supplemented
root's source review. Root alone owns shared interfaces, oracle, mutations,
builds, scope decisions and acceptance.

The source is pinned MathJax-src 4.1.3 MmlNode.ts, SHA-256
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`.
The installed executable CJS SHA-256 is
`86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45`.
`p8b-members.json` maps all 111 AST-inventoried members, including all 85
executable members, to native responsibility. Root reviewed bodies, field
initialization, virtual dispatch, evaluation order, aliases and partial errors.
This ledger covers this one upstream file, not its unimplemented dependencies.

## Review corrections

Root rejected bypassing virtual TeX-class accessors, retargeting a child array
during iteration, virtual core() use in Layout's literal-child class transfer,
wrong right-associative core-state clearing order, finite-arity null-array
checks after delegated hooks, and early invalid-column spacing return. The
reference witnesses preserve source operation order and partial state.

Independent review identified a short mError null dereference after virtual
parent replacement clears the error node's attributes. Root froze a CJS
witness, and implementation now fetches and checks a fresh attributes handle
after replacement before title assignment. The earlier p8b-review report is
a historical snapshot; its remaining finding has been repaired and verified.

The first mutation run exposed a root test gap: unknown-attribute errors could
mask missing mathvariant verification. Root added a separate runtime-generated
variant-only case; expected values were never derived from native results.

## Validation

- Oracle --check verifies clone/package source equality and runtime resolution
  and hashes. With P9's default-argument regression it reproduces 42 actual-CJS
  cases with 1,077 assertions.
- Seven further native assertions freeze the literal 512-call resource budget,
  overflow RangeError, shared normal/token traversal and subsequent recovery.
  They do not claim the VM has the same stack capacity.
- Native filtered build passes all 1,084 assertions in 43 tests. The complete
  `cmd /c build_test.bat 2>&1` build and core suite pass.
- `mml-node-mutation-test.cjs` passes an isolated baseline and detects all
  twenty deliberate regressions: signed spacing, virtual class dispatch,
  invalid-column evaluation, root id removal, child keepIds forwarding,
  attribute copy, inferred conversion alias, captured empty-array iteration,
  stop-inherit copy, null/undefined display distinction, raw child levels,
  variant validation, Layout core dispatch, token inheritance identity,
  snapshot scripts, Text raw debug field, XML adaptor omission, Empty class,
  resource capacity and token walk identity. All mutations compile and exit 1
  through assertion failures. Working sources and fixtures remain byte-equal.
  Logs/results remain ignored under build/math-port-mml-node-mutations.

P9's concrete-node differential tests exposed a prerequisite bug: explicit
Missing scalar arguments did not apply source default parameters. Root repaired
the shared method-entry boundary, froze a CJS regression and verified the whole
core suite plus all twenty base mutations again. Constructor-default resolvers,
post-super concrete field phases and nullable ErrorOptions extend the native
binding for real callers; they do not change the original source-method mapping.

## Explicit host boundaries

The interfaces are compiled class/factory services, UTF-16 TextNode strings,
typed nullable numeric TeX classes, ordinary PropertyList scalar records,
two-slot origin tuples, and opaque XML handles with borrowed adaptor services.
They do not expose arbitrary calls from JavaScript, sparse arrays, custom
iterators, proxies, descriptors or arbitrary JS method evaluation. Text/XML
setters accept their source-declared native types, not arbitrary erased JS
runtime values. Future HtmlNode must bind real textContent/DOM services.

Read-only TEXCLASS/TEXCLASSNAMES are the documented raw-TeX host adaptation;
mutable variant/indent collections and policy/default records preserve required
live identity and copy/alias behavior. Post-adoption initialization preserves
the source constructor's virtual calls; one generic store owns parent/children/
properties. All recursive overrides share the accepted tree resource service.

Concrete MML nodes, operator dictionary, MmlFactory registration, TeX handlers,
fonts, Common wrappers, extracted shared placements, SVG and Folia integration
remain unaccepted. No production renderer has been replaced by this packet.

Eleven of 246 inventoried sources now have local acceptance. The other 235
sources and unreviewed external dependencies keep full G0 closed. Full G1–G7
remain closed; local base-method tests cannot substitute for those gates.
