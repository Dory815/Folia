# P15a root implementation contract — multiscript semantics

Pinned source: MathJax-src 4.1.3, mmultiscripts.ts SHA-256
bb8d8d2180ef483b86b0b1570b919ba424ed54db1f30ef6e0e697cfab0bf5cf7.
Root read the complete source and reviewed p15-mmultiscripts-audit.md.
The AST inventory has 13 members, ten executable. Six accessors and three
fresh static defaults copies are root-generated. The remaining four methods
are the sole implementation assignment after root freezes the source oracle.

## Architecture and exclusive ownership

MmlMmultiscripts derives from MmlMsubsup; MmlMprescripts and MmlNone derive
from AbstractMmlNode. Each keeps its source class identity and defaults handle.
Mmultiscripts arity 1 means minimum one, not fixed one. Marker/None arity 0
permits no children. Semantic inheritance, placeholder insertion and
verification remain here; script measurement, pre/post placement and paint
are later Common/scene responsibilities. No TeX commands or output belong here.

Root owns mmultiscripts.ixx, mmultiscripts_bindings.cpp, multiscript_host.ixx,
source hashes/member ledger, actual-CJS oracle, tests, CMake and acceptance.
The implementation agent may write ONLY:

* src/math/core/mml_tree/mml_nodes/mmultiscripts.cpp
* docs/math-port/evidence/p15a-mmultiscripts.md

No builds, tests, commits, delegation, registry changes, shared-file edits or
expected-result changes by the agent. Missing operations must be reported to
root; do not invent helper semantics. Root alone reviews and accepts.

## Ordered implementation obligations

Every method uses TreeCallGuard. Calls to kind/isKind/appendChild/verifyTree/
mError remain virtual; superclass calls are qualified. Keep raw Property
values and shared AttributeList handles. The inherited Msubsup algorithm is
not called by the multiscript inheritance override.

The base child's inheritance method is accessed first and receives the four
original values. An absent/null base fails before normalization. The script
loop starts i=1, n=0 and rereads the current childNodes property for EVERY
condition and indexed access. Do not capture one vector for the whole loop,
use a fixed Msubsup count, or range-for over a vector that callbacks may edit.

Only the first mprescripts marker sets the local prescripts flag. For an even
marker index, create none with exactly zero factory arguments, then capture
the CURRENT child array and splice there. Preserve that array's identity.
Assign none.parent=this AFTER insertion, then increment i once locally in
addition to the loop increment. The inserted none is not classified/inherited
in that loop. Marker visits do not increment n. Other children receive the
same attributes, false, JS level+1 and raw prime || (n%2==0); then n increments.
Use host::add for JS addition and preserve a truthy raw prime operand.

After scanning, recompute parity on current childNodes, using prescripts to
select the threshold. If a none must be appended, factory creation precedes
virtual appendChild. After that callback, capture current childNodes for the
index receiver, read current childNodes length again, and access its final
child. That inheritance call uses ORIGINAL prime, not alternating primestyle.
Keep factory/append/null failures and partial mutations in source order.

verifyChildren captures raw options.fixMmultiscripts ONCE before its live
indexed scan. A repeated marker calls child.mError with child.kind read before
this.kind, the original options handle and short=true. First-even-marker and
final-parity errors call this.mError with short=false when captured fix is
falsey. Returning errors permit further scanning and the qualified
MmlMsubsup::verifyChildren call last; throwing errors interrupt it. Verification
reports errors and does not perform the inheritance-time placeholder repair.

Mprescripts/None verifyTree take the established optional PropertyList API.
They call AbstractMmlNode::verifyTree FIRST, including null options, then read
the current direct parent edge (not logical Parent()) and test mmultiscripts
class identity. An inferredMrow parent is invalid even when its logical parent
is mmultiscripts. Invalid parent calls
mError(kind + " must be a child of mmultiscripts", same nullable options, true).
Null options make the superclass return; they do NOT skip this parent check.
Use multiscriptErrorOptions to preserve null on that later mError call.
The accepted mError can create/annotate an error node before its null-option
read throws; preserve that partial state and the existing parent-merror early
return. Do not substitute empty options or prevalidate them before creation.

## Primitive host boundary

multiscriptChildren captures a current typed array; multiscriptSlot yields
nullable raw pointers; multiscriptNode validates at method/property access.
multiscriptChild is a current-property indexed read. multiscriptInsert binds
positive splice(i,0,node), clamps against current length, preserves the handle
and never sets parent. multiscriptCreate performs the exact zero-argument
factory operation and does not prematurely reject a null result.

The accepted native child array is a dense vector of node pointers. Null and
out-of-range reads fail at required-node access; no arbitrary JS Proxy/index
accessors or array prototype replacement is introduced. This binding must
preserve observable source order for supported factory/tree callbacks and
malformed dense arrays. It is an explicit host representation, not permission
to silently ignore malformed states or omitted source branches.

## Root acceptance requirements

The oracle must invoke actual pinned CJS classes and methods to obtain all
expected values. Required witnesses: complete defaults/getters and distinct
defaults identities; raw display/level/prime matrix; pre/post marker positions,
missing partner insertion and unchanged array identity; unclassified inserted
none versus inherited appended none; attribute aliases; live growth/rebinding
from base/child/marker/factory/append callbacks; null/empty partial failures;
captured fix despite callback mutation; duplicate marker message ordering and
short flags; source returning/throwing error behavior and superclass dispatch;
actual error trees; marker/None superclass-before-parent order with null
options, arity/attributes and valid/invalid parents; complete outer inheritance.

Root reviews all 13 member mappings, reproduces bindings/oracle, runs focused
and full core checks, and uses isolated compiling mutants that must fail
unchanged assertions. Crashes/build failures are rejected. No packet or
overall renderer gate passes merely because a library compiles.

Initial frozen oracle: 1,286 assertions / 222 cases, 480 raw inheritance rows
and 1,161 source-value records. Root may extend gaps using actual CJS
observations; agents cannot alter expectations or reduce this coverage.
