# P17b Mi/Mo/Mstyle inheritance dispatch

This packet updates only the three concrete public source methods assigned by
the frozen [inheritance dispatch contract](../inheritance-dispatch-contract.md).
The pinned MathJax 4.1.3 source file hashes are:

- `MmlNodes/mi.ts`: `3BBDDAEAC72D2C0F2E5D8A0440C28188F98D1BF1481A0EC6872E103A50F146C2`
- `MmlNodes/mo.ts`: `2082CD372A4C21FA46C67AB3972B51BCC950F711DD3369CBB2BD7CED62C2F605`
- `MmlNodes/mstyle.ts`: `7442E2885D2F4605D42F5B4472B3183F9CBADBD8ED5C0AEDDF03B8F23C1D8616`

The native facade `MmlNode::setInheritedAttributes` forwards its four raw
Property values to virtual `inheritAttributes`; the method body has no C++
defaults. Each of these source methods now applies
`host::inheritanceDefaults(attributes, display, level, prime)` at its own body
entry, before its first source-side action. Only Missing values are defaulted
there. Each qualified `super.setInheritedAttributes(...)` call is represented
by the qualified native base `inheritAttributes(...)` body, avoiding a second
virtual dispatch. Child-hook algorithms are unchanged.

`MmlMi::inheritAttributes` corresponds to `mi.ts` lines 73–85. It defaults the
raw arguments, calls `AbstractMmlTokenNode::inheritAttributes` with the same
normalized Property handle and scalar values, then gets the text and runs the
single-character pattern. The final mathvariant test now reads the original
normalized argument using `host::get(attributes, "mathvariant")`, rather than
reading the node's current attribute collection. `host::truthy` preserves the
source `!attributes.mathvariant` test. The italic inherited write still occurs
only after those source checks.

`MmlMo::inheritAttributes` corresponds to `mo.ts` lines 413–425. It defaults
the four raw arguments at entry, invokes the qualified token-node base body,
then takes one text snapshot and retains the source order
`checkOperatorTable`, `checkPseudoScripts`, `checkPrimes`, `checkMathAccent`.
The argument carrier change does not alter those checks.

`MmlMstyle::inheritAttributes` corresponds to `mstyle.ts` lines 66–75. After
entry defaults, it writes inherited displaystyle and scriptlevel to the
current attributes handle, re-reads that handle for the second write, then
calls the qualified `AbstractMmlNode` body with the normalized arguments.
The two writes remain before the base call, preserving visible partial state
when the base method throws.

The implementation uses root-owned raw Property and host interfaces. No tests,
builds, fixtures, expectations, interfaces, or other source files were changed
or run in this assignment; the root owns those checks and acceptance.
