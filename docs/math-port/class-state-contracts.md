# P12 prerequisite — raw TeX class state

Root owns this boundary, tests, oracles and all changed declarations. No Mo
implementation task is authorized until this prerequisite passes independently.

MmlNode.texClass and prevClass are Property values, not optional numbers. Null,
Missing, zero, NaN, booleans, strings and reference handles remain distinct.
There is one class state per source field; no numeric mirror or fallback API.
Setters transfer the raw value. Base field initialization and transfers use
explicit PropertyNull, while concrete protected initializers retain their source
values. Empty nodes retain numeric NONE getters and read-only setter failures.

Getter methods are non-const because source getters may write attributes and
properties. No const_cast or hidden alternate mutation channel is permitted.
setTeXclass uses nullish selection, updateTeXclass transfers raw classes/levels
in source order, and mrow's class initialization uses nullish rather than truthy.

texSpacing reads prevClass for its nullish test, rereads it only when non-nullish,
then reads texClass once and applies truthy || ORD. NONE tests are strict raw
numeric equality. Indexed spacing lookup uses source property keys, preserving
string keys, prototype hits, invalid rows versus invalid columns, and native
bound coercion. Script-level short circuit is evaluated after the column lookup,
including a missing column, before Math.abs and the final length lookup.
The private source tables keep their exact values and standard array prototypes;
they are not publicly writable. Arbitrary JS descriptors/evaluated conversions
remain outside the compiled host API; no JS evaluator is introduced.

Freeze source-backed raw-state matrices and read/write traces before changing
implementation. Existing MML/leaf/script fixture expectations must retain their
source values while their native bindings switch to raw Property observations.
Require the final core suite and compiling assertion-failing mutants for null
versus Missing, truthy normalization, strict NONE, source getter read count,
raw transfers and read-only empty setters. Recheck existing affected mutation
anchors rather than deleting verification that exposed an interface issue.
