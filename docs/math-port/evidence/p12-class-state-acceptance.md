# P12a root acceptance — raw class-state prerequisite

Status: locally accepted by root after final core and MML mutation rerun.
This is a correction to an existing local prerequisite, not a new source-file
acceptance, Mo acceptance, or a renderer gate.

## Authority and review

Root owns the architecture, exact interfaces, source expectations, mutation
tools, build and acceptance. Contract/fixture commits 8260318, 2132dcc,
3b3ecb3 and ead1847 precede the implementation commit 78f6212. The three
agents own only the read-only reports listed in tasks.md. They cannot edit
shared code, redefine compatibility, update expected values or certify a gate.

The actual profile probe in mo-dependency-probe.cjs establishes that
`\mmlToken{mo}{toString}` and `\mmlToken{mo}{constructor}` can retain undefined
classes. The earlier optional-number bridge was therefore rejected. Each
native source class field now has one Property store, preserving null, Missing,
all scalars and reference identities. Class getters are non-const because Mo's
source getter can write attributes and properties. No numeric mirror remains.

Root reviewed MmlNode.ts class accessors, setTeXclass, updateTeXclass,
getPrevClass and texSpacing; Empty getters/setters and mrow initialization.
Transfers retain raw values and source write order. Spacing retains conditional
prevClass rereads, the captured current class, strict numeric NONE comparisons,
property-key lookup and script-level short circuit after the column read.
The private source spacing arrays retain values and standard array prototypes.

## Independent verification

class-state-oracle.cjs executes pinned actual MmlNode and NodeFactory CJS under
Node 24.13.0, verifies TS hashes and clone/package equality, and emits 13,441
assertions in nine cases. It covers 29 raw values, 1,682 class rows and 2,523
raw previous/current script-level rows, storage initialization/identity, getter
read/write traces, null-node guards, partial failure and bound Number metadata.
Native observations never generate expected values. --check reproduces output.

The initial 5,466-assertion matrix found two numeric prototype mismatches:
`TEXSPACE.length.__proto__` is Number.prototype, whose primitive value is zero.
Root supplied distinct Number prototype/constructor/toString/valueOf bindings;
later source-derived metadata review established toString's arity, repaired to one.
An agent's additional prototype-name/length concern was rejected by root:
the early prototype branch already returns Missing. The report was corrected.

All 20 isolated class-state mutants compiled and failed assertions. They cover
null/Missing initialization, setter coercion, truthy selection, lost rereads,
virtual getter bypass, loose NONE comparisons, raw transfer/clear ordering,
current/previous script suppression, early invalid-column return, Number boxing,
unboxing/arity, spacing signs and previous-node transfer. Working sources and
fixtures remained byte-equal. Results are ignored under
build/math-port-class-state-mutations/results.json.

Existing actual-CJS fixtures reproduce unchanged source expectations: MML
1,084 assertions/43 cases, leaf 669/27, scripts 1,689/18 and operators 13,578/9.
Before the final helper cleanup, the full core suite passed, along with all
20 MML, 29 leaf and 28 script compiling/assertion-failing mutants. Root then
removed three unnecessary const_cast calls by making the child-read helper
accept a const parent reference. No semantic child operation or ownership
changed. After this correction, the complete `cmd /c build_test.bat 2>&1`
suite passed again, including all 13,441 class-state assertions. All twenty MML
mutants also compiled and failed assertions again with working sources/fixtures
byte-equal. Final logs are build/p12-core-final.log and
build/p12-mml-final-mutations.log. Overall, this packet verifies 20 new mutants
and revalidates 77 existing MML/leaf/script mutants; compilation failures or
crashes never count as detection.

## Boundaries

Number bindings cover the unmodified required prototype identity/coercion
operations, not arbitrary JS Number objects, mutable global intrinsics,
formatting callbacks, descriptors or function evaluation. The read-only
p12-number-boxing-review.md lists remaining unbound methods. Future packets
must add reviewed bindings for required callers instead of treating the host
as a complete JavaScript runtime. No evaluator is introduced.

The final getter helper reads the parent's child-array handle and returns the
existing child pointer. It neither casts away parent constness nor creates an
alternate class-mutation channel. The older read-only report remains historical
evidence of the issue, not an assertion that it still exists.

This packet does not implement Mo, complete MmlFactory, TeX handlers, font
resolution, Common wrappers, shared scene, SVG or the Folia switch. The source
inventory remains 25 local acceptances out of 246; full G0–G7 remain closed.
