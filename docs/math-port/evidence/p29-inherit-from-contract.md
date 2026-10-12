# Root template inheritance contract

Root froze 9,344 actual original inheritAttributesFrom observations, including
104 original errors. Source JSON SHA256:
4fbceb2e3fda0716ac4ca72bb772613327f1376624c1fbd90d1085031a2fc20c.
Original MmlNode.ts/CJS pins remain those in P28. The actual Attributes.js pin
is efa567c0c573f13f136f1d37b0af6bdc1a675f70cc13a9ae1cd7334321f30749.

Root owns original probes/expected values, native observer, interfaces, tests,
mutation gates and acceptance. Luna math_layout owns only the body of
AbstractMmlNode::inheritAttributesFrom(NodeValue) in mml_node.cpp and its
p29-inherit-from-base-implementation.md report. Luna tex_parser owns only the
body of AbstractMmlEmptyNode::inheritAttributesFrom(NodeValue) in mml_empty.cpp
and p29-inherit-from-empty-implementation.md. No other bodies, declarations,
helpers, oracle/fixture/data/pins, build/test/generation, commit or delegation.

The base method checks the tagged input at the first attributes access, reads
attributes once, then on that same retained object calls get(displaystyle),
get(scriptlevel), isSet(mathsize), conditionally get(mathsize), and finally the
input node's virtual getProperty(texprimestyle). It creates a fresh ordinary
attribute object and, only when mathsize isSet, a fresh ['math', value] pair.
The receiver's established setInheritedAttributes facade dispatches to virtual
inheritAttributes; do not qualify the base implementation or normalize its
raw/default arguments before that dispatch. Returned values and their identities
are preserved; prime || false preserves every truthy raw value and replaces
only falsey values with false. TS boolean/number assertions are erased.

Null/Undefined node tags, null attributes, virtual get/isSet/property/setter
failures and preceding partial effects must match the frozen records. Attribute
and source-node callbacks may rebind the source attributes; captured attribute
reads continue on the original handle. The second call resolves current state
again and creates a fresh bag and pair. Do not cache state between calls.
Use the existing TreeCallGuard at the base method's recursive host boundary.

The Empty override is the original unconditional no-op. It must not inspect
the argument, attributes, properties or receiver inheritance setter, even for
nullish arguments or throwing observer modes. Do not call the base method.

Root added virtual get/isSet slots and the C++ virtual destructor to Attributes
before delegation, preserving the existing method bodies. These slots implement
the actual Source overridable method boundary required by this consumer; do not
add special observer branches or a second attribute service. Other Attribute
slots are outside this narrow declaration change, so no whole Attributes runtime
acceptance is claimed. The two consumer bodies still contain actual stubs.

Root's first native observer build exposed an invalid host helper name in its
own emitter; Root corrected the Native-only expression to PropertyList::hasOwn,
kept the original JSON byte-identical, and reran. The final full 90-module graph
compiles and the real stub baseline exits 1 through actual Unimplemented calls,
with final working-source/fixture byte guards passing. Implementation assignment
follows this concrete red evidence, not a source-text placeholder assertion.

Acceptance requires unchanged original --check, complete 9,344-record parity,
effective compiling callback/order/raw-value/no-op faults, existing primitive
and inheritance regressions plus the normal core filtered runner. Other MML
verification/error consumers, TeX/parser/layout, SVG/D2D and Folia remain open.
