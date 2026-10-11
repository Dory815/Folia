# Root prerequisite: MathChoice argument dispatch

Root read complete pinned mathchoice.ts, then requested a read-only audit from
math_layout. No MathChoice implementation is authorized by this report.

MathJax MmlNode interface (MmlNode.ts:245-251) and MathChoice override
(mathchoice.ts:74-84) require all four arguments. AbstractMmlNode implementation
alone defines JS defaults for attributes/display/level/prime (MmlNode.ts:765-770).
MathChoice's override has no normalization: falsey display plus undefined level
produces NaN selection, triggers factory-created mrow, replaces its parent,
and forwards raw original values to the selected child after replacement.

Current native MmlNode virtual declaration supplies C++ defaults empty object,
false,0,false (mml_node.ixx:78). C++ selects defaults at the static callsite before
virtual dispatch. A future MathChoice override reached by an omitted call
through this interface would receive values that source did not receive. The
same issue must be reviewed for other no-default source overrides. An explicit
four-argument source call does not have this problem.

Native Property represents Missing/PropertyNull and host::inheritanceDefaults
normalizes Missing scalars only. AttributeList is PropertyList: an always-object
shared record, with no undefined/null/presence state. This is a separate argument
representation boundary; changing scalar defaults alone cannot represent raw
omitted/null attributes. Returning a guessed empty object would not prove source
runtime parity. Existing native recursive inheritance callsites pass four
explicit arguments; inspected normal MathMLCompile and base propagation calls
also do so. No production MathChoice or native TeX compilation exists yet.

Before freezing the packet, root must specify source-required typed-call surface
and any broader runtime argument binding, decide method-default ownership, and
test explicit Missing/null scalars and callback/factory/parent order against
actual CJS. Do not silently narrow a claimed complete runtime contract, inject
normalization into MathChoice, or mutate accepted foundation interfaces without
focused regression evidence. This finding is not a completed port/acceptance and
does not alter the 41/246 local file acceptance count or closed G0-G7 gates.
