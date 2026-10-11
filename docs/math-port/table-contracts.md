# P14 table-family root-frozen implementation contract

Root has read the pinned mtable.ts, mtr.ts and mtd.ts and reviewed the three
read-only dependency reports. The source AST contains 26 members, including
21 executable members. Root interfaces, operation bindings, generated fields
and actual-CJS observations are frozen with this document before dispatch.
Implementation still requires root source review, regression and mutations.

## Files and class responsibilities

| Module | Classes and superclass | Agent-owned behavior after freeze |
| --- | --- | --- |
| mml_nodes/mtable | MmlMtable : AbstractMmlNode | setInheritedAttributes, setChildInheritedAttributes, verifyChildren, setTeXclass |
| mml_nodes/mtr | MmlMtr : AbstractMmlNode; MmlMlabeledtr : MmlMtr | Mtr setChildInheritedAttributes, verifyChildren, setTeXclass |
| mml_nodes/mtd | MmlMtd : AbstractMmlBaseNode | verifyChildren, setTeXclass |

Root owns the three IXX interfaces, generated binding CPP files, table_host
operation bindings, all CMake/test/data changes, source member ledgers and
acceptance. Each agent will own only its node CPP and a separate method report;
the existing audit reports are prerequisites, not implementation evidence.
Table nodes perform semantic inheritance, verification and tree repair. Table
measurement, row/column sizing, label placement and painting belong to later
Common/scene/backend packets and must not appear here.

Mtable's derived field phase replaces the generic properties record with a
fresh own {useHeight: true} record, then initializes inherited texclass to ORD.
It must preserve observations of the old properties record during superclass
callbacks. Mtr/Mtd retain the base class state; no extra ORD normalization.
Mlabeledtr inherits the identical Mtr defaults handle and class identity.
Its arity 1 requires at least one child and permits additional children.
Mtd arity -1 owns an inferredMrow; do not flatten it during table repair.

## Source operations and ordering

The locked CJS shallow-copy helper is Object.assign, as established in P13.
Inherited records/tuples are shared Property handles. Node algorithms use
factory class identity, so mlabeledtr satisfies isKind("mtr"). They use the
existing uniform tree edits and parent links, including source's explicit
temporary parent writes in table error repair.

For-of loops capture a child array once and observe its live size and slots.
Each later loop obtains a new capture. Mtable verification instead uses a
numeric loop over the live childNodes property and decrements its index after
removing a child. Consecutive non-row children form one row. These forms are
not interchangeable. Every callback receiver, argument read, parent write and
partial failure remains in source order, with TreeCallGuard and checked native
accesses.

Mtable's indentation entry captures this.attributes before the second incoming
tuple read and index-1 argument evaluation. Explicit indent attributes are
removed after the inherited write. indentAttributes is an exported const array;
its array binding is fixed, while elements and length are mutable. The existing
native vector can retain that binding for this algorithm if traversed by live
index and if each yielded name is copied before callbacks; a C++ range-for
would retain invalid iterators across growth. The frozen actual-CJS oracle
witnesses growth during the first virtual attributes read and verifies that
the appended name is inherited and its explicit value removed in the same call.

Mtable recomputes display using explicit displaystyle OR the default, converts
that result to boolean, and uses boolean explicit data-cramped for each row's
prime input. Mtr preserves its raw display/level/prime inputs. For a label row,
Mtr prepends the raw parent side value and literal top to the alignment lists.
Both algorithms preserve shift-or-current-slot fallback, shared tuple updates
and fresh vertical-align tuples. Capture a tuple's assignment receiver before
the right side, and evaluate this.kind before the second tuple element.

Root's table_host will bind string-only split to accepted UTF-16 string_util
whitespace behavior. Source TypeScript casts do not coerce non-strings. It will
also bind checked node/attributes access, exact factory argument shapes and
strict Property index writes. It must contain no row/cell policy or geometry.
ErrorOptions already preserves a PropertyList handle for mError; no separate
boolean options mirror is permitted. Source verifyTree defaults to null and
returns before entering protected hooks; native optional options already
preserves that entry. Protected verification receives the caller's non-null
PropertyList handle. No nullable-hook API change is required for this profile.

table_host exports primitive tableNode/tableChildren/tableChild checks,
tableSplit/tableShift for local alignment lists, strict tableSetIndex for
captured Property receivers, and tableCreate with exact zero/two-argument
factory shapes. Use arithmeticAttributes to capture virtual attributes;
host::get performs raw tuple reads. Each setTeXclass and inheritance/verification
method uses TreeCallGuard. Do not add algorithms to these helpers or edit them.

All source reads of kind, arity, attributes, isKind and tree edits stay virtual.
Superclass method calls are explicit qualified calls. Mtable's hook and outer
override do not add entry defaults; the superclass supplies its own defaults.
Build both source argument fields in their written order before calling the
virtual addInheritedAttributes. Capture the rowalign/columnalign tuple before
shift-or-fallback evaluation. Empty shifted strings fall back to the previous
slot; label side is a raw Property value. Both output loops use the same local
attribute handle and retain callbacks' shared-tuple mutations.

| Agent | Only writable CPP | Only writable method report |
| --- | --- | --- |
| tex_parser | src/math/core/mml_tree/mml_nodes/mtable.cpp | docs/math-port/evidence/p14-mtable.md |
| math_layout | src/math/core/mml_tree/mml_nodes/mtr.cpp | docs/math-port/evidence/p14-mtr.md |
| math_fonts | src/math/core/mml_tree/mml_nodes/mtd.cpp | docs/math-port/evidence/p14-mtd.md |

No agent builds, tests, commits, delegation, shared edits, factory registration
or changed expected observations. Missing contract requirements must be
reported to root. Root alone may correct interfaces or extend tests.

## Required independent acceptance

The initial frozen oracle has 3,783 assertions / 116 cases, with 240 raw hook
rows and source traces. Root extends any remaining branch gaps before accepting
the packet. Required observations include defaults, field phases and identities;
outer inheritance and complete parent/child snapshots; grouping versus per-child
repair; valid and invalid parents; fixMtables false/truthy branches; short/full
errors; labels, empty rows and inferred cells; alignment list fallback and raw
value failures; shared tuple mutation; live arrays and virtual receiver traces.
Add nested arithmetic/table compositions using the real accepted node classes.
Agents cannot edit observations or declare a failed example outside scope.

Root reviews all 26 member mappings, reproduces pinned generators, runs focused
and core regression tests, and requires compiling mutants to fail unchanged
assertions. Compilation failures and crashes do not count. No full MML factory,
parser, layout or production gate is passed by this packet.
