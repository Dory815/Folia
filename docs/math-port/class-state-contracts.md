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

The numeric `length` row exposes Number.prototype through `__proto__`.
Root binds its unmodified primitive zero, Object parent, Number constructor,
toString and valueOf identities separately from Object/Array intrinsics.
Number-object creation, mutable intrinsic globals and unbound formatting
callbacks are not services offered by this API. Future callers must request a
reviewed binding rather than silently reuse a different intrinsic identity.

Freeze source-backed raw-state matrices and read/write traces before changing
implementation. Existing MML/leaf/script fixture expectations must retain their
source values while their native bindings switch to raw Property observations.
Require the final core suite and compiling assertion-failing mutants for null
versus Missing, truthy normalization, strict NONE, source getter read count,
raw transfers and read-only empty setters. Recheck existing affected mutation
anchors rather than deleting verification that exposed an interface issue.

Final source fixture: 29 raw values, 1,682 previous/current class rows, 2,523
previous/current script-level rows, initialization/identity checks, null node
guards, previous-node getter/write traces and partial failure, plus bound Number
metadata. It emits 13,441 assertions in nine cases. MmlNode and NodeFactory TS
and executable hashes are pinned; output is reproducible with --check. The
20-mutation packet owns isolated copies and must compile before an assertion
failure counts as detection. These requirements accept only this prerequisite,
not Mo, a complete semantic tree, any renderer or Folia integration.
