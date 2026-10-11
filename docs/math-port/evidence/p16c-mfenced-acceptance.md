# Root P16c local MmlMfenced semantic acceptance

Status: complete ten-member semantic source file locally accepted. This is not
TeX parsing, Common measurement/placement, SVG, native backend, or Folia release
acceptance. Full G0–G7 remains closed. Production math still has its single
existing path until the complete native pipeline passes those gates.

Root read the entire pinned mfenced.ts before freezing mfenced-contract.md,
owned declaration/constants/factory bindings and independently reviewed the
four method implementations. The member ledger records all ten members with
exact TypeScript source/member hashes and target responsibilities. Defaults,
post-superclass field initialization and kind belong to generated bindings;
class chaining, fake inheritance, fake creation and factory construction remain
the source node's methods. No renderer logic or added fake real-tree children.

The accepted methods preserve live array reads around callbacks, captured
counts/slice/concat/push receivers, null skips, repeated-state retention,
ASCII-only delimiter cleaning, UTF-16 separator indexing/padding, source-owned
virtual defaults and text-setter returned identity. Factory lifetime and the
existing typed TextNode/attribute/constructor services are native host bindings,
not a JavaScript VM or a second MathML representation. Arbitrary dynamically
invented JS methods/properties are not exposed by that typed host interface.

## Independent source observations

mfenced-reference.cjs checks the pinned source hash and actual bundled source,
then loads the real 38-package profile/complete registered CJS MmlFactory.
mfenced-bindings-data.cjs verifies the TypeScript AST has exactly ten members
and extracts the six constants/field/accessor responsibilities from source.
mfenced-oracle.cjs executes actual CJS methods with unchanged source constants
and independent callback/factory witnesses; no expected value comes from CPP.

Final reference corpus: **225 cases, 984 assertions, 509 encoded source value
records**. Coverage includes constructor phase/fresh arrays/default identity,
empty/one/many/null children; custom/absent delimiters; ASCII/non-ASCII whitespace;
astral and lone-surrogate separators; dense and sparse padding/repetition;
raw attribute failures; partial failures and live/snapshot/captured receiver
mutations; class return chaining; fake-before-real inheritance and raw arguments;
direct omitted/undefined/null/zero/raw tex values and properties forwarding;
returned/absent/throwing TextNode products/setters; setter-before-parent order;
and real Mo/TextNode inheritance including raw tex class and spacing fields.

Root initially omitted the existing munderover registration in its native test
factory. Real Mo.checkMathAccent queries that class even when the parent is a
fenced node. Root corrected the fixture registration to match actual CJS; no
production algorithm changed for that failure. Initial review also removed a
duplicate agent kind getter and moved the null push-receiver failure before
the fake callback, matching source evaluation order.

## Mutation and build evidence

Root's isolated mfenced-mutation-test.cjs copies accepted prerequisites and the
unchanged generated oracle into ignored build output. The unmodified baseline
compiles and passes. All **30 final mutations compile and exit 1 from failed
assertions**, rather than crashing or failing compilation. They remove class
hooks/chaining/count endpoints, fake/real inheritance/order, source whitespace
rules, final-unit padding/index skipping, captured push receiver, raw virtual
defaults, setter return/text, null default semantics, parent order/assignment,
field resets/INNER class and independent defaults. Working sources and fixture
bytes are checked unchanged afterward.

The first mutation run exposed a real test coverage gap: first-unit padding
survived because sparse child arrays did not consume their padded tail. Root
added actual-source dense 3/4/5-child cases before rerunning the entire matrix;
the mutation then failed. The rejected first run is not counted as acceptance.

Root validation:

- node tools/math-port/mfenced-bindings-data.cjs --check: success.
- node tools/math-port/mfenced-oracle.cjs --check: exact regeneration success.
- isolated baseline and final 30 compiling mutations: success.
- cmd /c build_test.bat: full core build/test exit 0.
- git diff --check: success.

Ignored logs: build/p16c-core-final.log, build/p16c-mutations2.log and
build/math-port-mfenced-mutations/{baseline,mutation}-build/test logs. The
committed mutation record records observed exits; generated build files remain
ignored. These checks do not imply complete MmlFactory/profile registration,
Common/SVG wrapper coverage, worker/resource acceptance or application wiring.
