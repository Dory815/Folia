# Current native MathJax status

The goal remains native TeX -> shared mathematical layout -> SVG, then verified
Folia integration replacing the math QuickJS worker. Folia still uses that
QuickJS path; complete renderer/application acceptance remains open.

Latest verified results are Gate268: complete Core passes 5,723 tests /
350,295 assertions; Host passes 484 / 3,477; output foundation passes
116 / 1,334. The unchanged pinned profile matches common layout 75/75,
full SVG bytes 150/150, paired Scene geometry 150/150 and all four
listener-position pairs. Font tables, instance extensions and provider
comparisons also pass. All 2,213 registered inputs remained unchanged.
See [the Gate268 execution record](evidence/shared-gate268.md).

Request-owner growth remains unresolved: owner-bearing records grow by
27,264 over 192 renders, down by 768 from Gate267 after document-owned
filter-record cleanup. MML projection growth is unchanged. The new
Source-constructor properties capture passes its ownership tests but is
not the native Renderer NodeFactory creation path. Further native-node,
filter-record and Lite serializer callback ownership work remains open.

The complete per-file/member review, real platform raster acceptance,
native bridge/worker acceptance and Folia math backend switch remain open.
Integration candidates are drafts, and Mermaid remains paused. Current
builds reuse ignored output and run at most four compile jobs.

## Earlier Gate255 checkpoint

Gate255 passes the complete Host suite (462 tests /
3,269 assertions), the filtered mfenced Core corpus (228 tests / 1,008
assertions), the font provider (40 modules / 80 requests / 24,646 glyphs), and
font instance/extension comparison (3/3). An additional 36 SVGs at em 12, 16,
and 32 match pinned Source byte-for-byte. Gate247's 75 Common layout records
exactly match an independently
regenerated pinned Source capture with the same register-only event host as
the SVG reference; the other 73 Source records remain unchanged. Gate251
passes the full four-source static font tables plus all synchronous dynamic
glyphs, options and delimiters. Each build uses at most four compile jobs;
2,208 registered inputs remained unchanged through the Gate255 observations.

The private Renderer Source allocation domain, prototype getter lifecycle and
canonical TeX error-formatter returns are committed and focused tests pass.
General lifetime acceptance remains open. Gate255 retires the canonical
Attributes and their three initial native layer projections; canonical
Attributes stay at zero. Live owner-bearing records still grow from 2,651 to
33,819 over 192 requests, despite the narrower MML/Scene/Lite counts returning
to baseline. Fresh Attributes return projections and the Common layout job's
Map are the next separately reviewed ownership paths.
See `evidence/context-record-inventory-g246.md`,
`evidence/private-source-domain-g247.md`,
`evidence/tex-format-error-context-g247.md` and
`evidence/common-font-gates247-251.md`,
`evidence/request-owner-cleanup-g255.md` and
`evidence/font-provider-and-svg-g255.md`.

Gate236's package comparisons pass 24/24, 23/23, 16/16 and 20/20; Gate233's
nested SVG corpus passes 12/12. These remain bounded corpus results. Private
Renderer Context lifetime, current complete SVG/Scene and broader core/platform
checks, the per-file/method acceptance ledger
and Folia integration are still required. The last complete 150-record SVG and
four event-position pass remains Gate239; it is not a current-tree acceptance.
The reviewed WinUI C ABI integration and actual-worker test packets are drafts.
Mermaid remains paused. Gate238 was rejected after a draft agent changed the
unregistered scope manifest; Root restored it and included it in Gate239's
snapshot. No scope acceptance was advanced by that edit.

## Historical records

Latest package comparisons: next-six passes 20/20 at Gate222; font packages
pass 16/16 at Gate223 against unchanged Source records. The observers now
separate compile/state capture phases explicitly, and the font observer seeds
the root Map's canonical identity before traversing shared aliases. A small
compiler experiment and C4866 diagnostics reproduce the old expression's
order limitation. These gates cover package observations only; see
`evidence/shared-gates222-223.md`.

Latest SVG evaluation (Gate221): all 61 targets build and 2,187 registered
input entries remain unchanged through evaluation. The tooltip candidate fails
its Scene nesting test and public Renderer geometry handling. The 150-record
profile matches 148 SVG hashes; both tooltip records now report a geometry
TypeError. The candidate is not accepted. Package diagnostics confirm correct
live state after compile but stale observation output; comparison results
remain 18/20 and 13/16. See `evidence/shared-g221.md`.

Earlier SVG diagnostic (Gate219): both 75-case modes produce 150 SVGs with no
thrown render errors; 148 byte hashes match the passive-host Source baseline.
Both differences are tooltip output. All six Maction event types and target
paths match in a separate diagnostic. The final input audit detects 25 pure
LF-to-CRLF changes, so this is not a passed frozen-source gate. See
`evidence/shared-g219.md`.

Latest completed focused evaluation (Gate220): all 61 targets compile with
2,185 unchanged registered input entries and at most four compile jobs. All
13 focused test/capture invocations exit 0. The ParseMethod Source receiver
regression passes 11 assertions, and single-/two-jax TextMacros state tests
pass. Package comparisons still reject next-six (18/20) and font packages
(13/16), now first differing in captured ParseOptions state. The actual
NativeRenderSession observation path remains under investigation. See
`evidence/shared-g220.md`. Complete renderer/lifetime acceptance and Folia
integration remain pending.

Earlier focused evaluation (Gate218): all 61 targets compile with
2,183 unchanged registered input entries. Every focused test/capture exits 0,
including 100 Maction host assertions, 55 TextMacros assertions, 36 mfenced
assertions and all 2,821 Host assertions. The package comparators still reject
next-six (18/20) and font packages (13/16): remaining differences include
packageData live-state identity and a ParseMethod receiver exception. These
test results do not establish complete layout/SVG or lifetime acceptance. See
`evidence/shared-g218.md`. Folia integration remains pending.

Latest completed focused evaluation (Gate217): all 61 targets compile with
2,181 unchanged registered input entries and at most four compile jobs. Full
Host passes 2,821 assertions; focused property-copy, TextMacros preprocessor,
ParseOptions.clear and Attributes constructor checks pass. Six mfenced tests
complete with six setter-call sequence failures, without reproducing the Core
crash. Package comparisons remain incomplete: next-six matches 18/20 and font
packages 13/16. No complete layout/SVG or lifetime acceptance was rerun. See
`evidence/shared-gates214-217.md`. Later callback/construction fixes await a new
build; Folia integration remains pending.

Latest completed full evaluation (Gate213): all 61 targets compile with 2,179
unchanged input entries through evaluation, using at most four compile jobs.
API matches 9/9 and BBoldx SVG/glyph paths match 6/6. Parser, Regexp and
LinkedList pass. Core still exits with an access violation; Host and Foundation
fail. Common layout, full SVG, paired Scene and both request-lifetime probes
exceed their existing time limits, so this gate establishes no complete layout
or SVG parity result. Package captures complete 24/25 and comparisons pass
15/20. See `evidence/shared-g213.md`. Source-confirmed fixture corrections,
fallback property-copy snapshots and timing diagnostics were applied afterward;
Gates216–217 provide the subsequent focused results above. Folia integration
remains pending.

Previous completed full evaluation (Gate212): all 61 targets compile with 2,179
unchanged inputs through evaluation. API matches 9/9, Common layout 75/75,
and public SVG/error records 150/150. This means 146 serialized SVG results
and four matching host errors, not 150 successful SVG renders. The two Maction
formulas still need an event-registration host capability. Paired Scene/SVG
routes agree on all 146 successful outputs; BBoldx SVG also passes.

Full acceptance still fails: Core exits with an access violation, Host and
Foundation have failures, mhchem and two request-lifetime observations exceed
their 60-second limits, and 16/20 package comparisons pass. Parser, Regexp and
LinkedList pass. Gate213 subsequently compiled the reviewed Tree null-property
access, partial property-copy Symbols and MML constructor-field corrections;
its full evaluation is recorded above. See
`evidence/shared-g212.md`.

The Core operator-trace null read was isolated to an inactive diagnostic
context and fixed in c6074e6. Its exact test passes. The subsequent small
commits add cancellation, shared Context property storage, per-Context
module owners, ordered Map storage and Unicode cancellation checkpoints.
Substantial Tree, MML, parser and output work remains uncommitted and must
be split by actual interface dependencies. Folia integration remains pending.

Earlier completed full evaluation (2026-10-16 local, Gate209): all 61 targets compile
with 2,160 unchanged inputs through evaluation. The native MML constructor
now converts its actual lexical factory, attributes and children to canonical
Context values. Rendering progresses past Gate207's conversion exception,
but still fails: complete SVG/errors match 0/150, Common layout 0/75, BBoldx
text SVG 0/6, and 4/20 checkpoint comparisons pass. Core still ends with
0xC0000005; Host, Parser, Foundation and all three lifetime drivers fail.
Regexp and LinkedList pass. Static tracing identifies a missing inherited
AbstractFactory prototype on MML factories: getNodeClass is undefined during
the shared initializer. Correcting that boundary remains pending. Gate208
failed to compile its new tests; the explicit Descriptor setter argument and
FactoryArgs overload corrections are verified by Gate209's build. See
`evidence/shared-gates208-209.md` and
`evidence/factory-context-base-contract.md`. Folia is not integrated.

Earlier completed evaluation (2026-10-16 local, Gate207): all 61 targets compile
with 2,157 unchanged inputs, but the shared MML work has runtime regressions.
Complete SVG/errors match 0/150, Common layout 0/75, BBoldx text SVG 0/6,
and 4/20 checkpoint comparisons pass. Core ends with 0xC0000005; Host,
Parser, Foundation and all three Renderer lifetime drivers fail. Regexp and
LinkedList pass. The MML argument-record wrapper mismatch from Gate206 is
corrected; rendering now reaches a missing native-to-Context argument
projection during initialization. The paired routes agree only on 150 error
pairs, with zero successful Scenes, so that agreement is not SVG acceptance.
See `evidence/shared-gates206-207.md`. Folia is not integrated.

Earlier compilation work (Gates199–205): the shared MML/Tree constructor and
operation-view packet is applied to the working tree, but has not compiled or
passed native acceptance. All seven builds ended with compiler errors and
unchanged frozen inputs; no old executables were used to claim results for
these gates. Root corrected template lookup, UTF-16/key conversions, Symbol
copy call sites, and MML container/owner interfaces. Gate205 still reports the
Common factory base-owner signature and MML test type errors. Empty-node
constructors and Array/Latex methods remain separate, unaccepted candidates.
See `evidence/shared-gates199-205.md`.

Earlier completed evaluation (2026-10-16 local, Gate198): all 60 targets compile
with 2,127 unchanged inputs through evaluation. Core, Host, Parser, Foundation,
Regexp and LinkedList all pass. Complete public SVG/errors match 150/150,
API 9/9 and Common layout 75/75. All 25 captures complete; 18/20 comparisons
pass, with next-six and font-package data still differing. All three real
Renderer lifetime drivers pass, including the mixed macro/label/error/Scene
sequence and retained outputs after destruction. The teardown regression is
resolved; Core's four remaining Gate197 failures are corrected and verified.
Context record slots remain cumulative, so this is not a whole-renderer memory
bound. Shared MML constructor/algorithm and six Stack-method candidates remain
under review. See `evidence/shared-gates197-198.md`. Folia is not integrated.

Earlier completed evaluation (2026-10-16 local, Gate196): all 60 targets compile
with 2,125 unchanged inputs, but runtime evaluation fails. Parser, Regexp and
LinkedList pass; Host, public SVG/API and Renderer lifetime drivers exit
abnormally. Core is explicitly terminated at its CRT runtime-error dialog.
The minimal real Renderer diagnostic completes five renders and fails during
destruction. New RegExp binding teardown accesses Context during its own
storage destruction; that lifetime boundary is being corrected. Gates194
and 195 preserve their test-compilation failures. No public parity or owner
acceptance is claimed for Gate196. See `evidence/shared-gates194-196.md`.
Folia is not integrated.

Earlier completed evaluation (2026-10-16 local, Gate193): all 60 targets compile
with 2,119 unchanged inputs. Complete public SVG/errors remain 150/150,
API 9/9 and Common layout 75/75; all 25 captures complete and 18/20 comparisons
pass. Host and Foundation now pass. Core retains three failing tests and
Parser one; a missing dormant-environment guard and two incorrect fixture
module lookups are corrected after evaluation, not yet rebuilt. TagInfo
retirement passes its focused tests and all 24 parser identity families stay
stable through 66 plain renders. The older lifetime drivers pass with their
180-second allowance; the stronger mixed driver still fails. Source review
identifies orphan Label placeholders, while the extra Lite identity belongs
to a pre-existing session element and needs named-record verification.
Shared MML algorithms and broader package work remain candidates. See
`evidence/shared-gate193.md`. Folia is not integrated.

Earlier completed evaluation (2026-10-16 local, Gate192): all 60 targets compile
with 2,114 unchanged inputs. Complete public SVG/errors remain 150/150,
API 9/9 and Common layout 75/75; all 25 captures complete and 18/20 comparisons
pass. Foundation now passes. Core's seven raw Stack failures reduce to two,
but Core, Host and Parser still fail, including two newly exposed constructor
and argument-phase regressions. Per-family diagnostics identify TagInfo as
the only registry growing during repeated plain renders (5 warm to 71 after
66 more); the other 23 families stay unchanged. A full-sequence diagnostic
locates the extra Lite identity at malformed input, and distinguishes durable
Label additions from transient TagInfo growth. The older lifetime driver is
terminated by an accidentally shortened 60-second harness limit, so it is
not a passing result or proof of a semantic regression. Gate191 failed only
on a missing forward declaration, corrected in Gate192. See
`evidence/shared-gate192.md`. Folia is not integrated.

Earlier completed evaluation (2026-10-16 local, Gate190): all 60 targets compile
with 2,108 unchanged inputs through evaluation. The canonical node-array
ownership correction restores complete public SVG/errors to 150/150 and API
to 9/9; Common layout remains 75/75. All 25 captures complete and 18/20
checkpoint comparisons pass. Host, Parser, Regexp and LinkedList pass; Core
retains seven raw Stack-field tests and Foundation retains MML bad-kind
ordering. The two earlier lifetime drivers pass, but the stronger mixed
64-iteration driver fails actual registry and Lite identity counts. Plain
renders accumulate one parser registry entry each; the mixed driver grows
from 123 to 322 entries and retains one extra Lite identity. Cancellation
also retains one extra parser entry. This is not accepted request cleanup.
The paired NativeRenderSession routes retain 146 equal successful Scenes
and four equal error pairs, with zero materialize/XML calls on Scene-only
routes. See `evidence/shared-gate190.md`. Folia is not integrated.

Earlier completed evaluation (2026-10-16 local, Gate189): all 60 targets compile
with 2,108 unchanged inputs. The new private Renderer request-owner cleanup
regresses public conversion: complete SVG/errors match 2/150 and the API
comparison fails. All three Renderer lifetime drivers exit 1 at a retired
Context record; a standalone diagnostic confirms the first warm render fails.
Parser, Regexp and LinkedList pass; Core, Host and Foundation fail. All 25
captures complete and 17/20 comparisons pass. Common layout remains 75/75;
the NativeRenderSession route audit remains 146 equal successful Scenes plus
four error pairs. Those paths do not exercise private Renderer cleanup.
The source shows nodeLists and nodeArrayIdentities both retire the same
canonical child-array handle. An owner-classification correction is applied
for Gate190, whose build is in progress; it is not yet verified. See
`evidence/shared-gate189.md`. Folia is not integrated.

Earlier completed evaluation (2026-10-16 local, Gate187): all 59 targets compile
and all 2,102 inputs remain unchanged through evaluation. Public SVG/errors
remain 150/150, API 9/9 and Common layout 75/75; paired routes retain 146
equal successful Scenes and four equal error pairs with zero Scene-only
materialization/XML calls. All 25 captures and 18/20 comparisons pass.
Host, Parser, Regexp and LinkedList pass. The two Scene-parent tests and the
original direct Stack-storage observation now pass. Core retains seven raw
Stack-field tests and a new dispatch-trace fixture difference; Foundation
retains only MML bad-kind ordering. Both extra lifetime drivers pass, while
broader owner counts still accumulate. Folia is not integrated. See
`evidence/shared-gate187.md`.

Earlier completed evaluation (2026-10-16 local, Gate185): all 59 targets compile
with all 2,100 inputs unchanged through evaluation. Public SVG/errors pass
150/150, API 9/9 and Common layout 75/75. Paired routes retain 146 equal
successful Scenes plus four equal error pairs; Scene-only materialization
and XML serialization counts remain zero. All 25 package captures succeed;
18/20 checkpoint comparisons pass. Host, Parser, Regexp and LinkedList pass.
Core retains seven raw Stack-field failures, two Scene-parent failures and
one direct Stack-storage failure; Foundation retains three failing tests.
Both Renderer lifetime drivers pass, but MML/parser/Lite owners still grow.
The direct Stack-storage correction is applied after evaluation and remains
unbuilt. Broader constructor, method-binding and owner-retirement packets
are still unaccepted. Folia is not integrated. See `evidence/shared-gate185.md`.

Earlier completed evaluation (2026-10-16 local, Gate183): all 59 targets compile
and all 2,098 captured inputs remain unchanged through evaluation. Host,
Regexp and LinkedList pass. Core, Parser and Foundation still fail. All 25
package captures succeed; 12/20 checkpoint comparisons pass. Public SVG/error
comparison is 132/150, API 8/9 and Common layout 66/75. All 18 full-SVG
differences report the same NodeFactory children-slot TypeError. Paired native
routes contain 128 equal successful Scenes and 22 equal error pairs, with zero
materialization/XML serialization on the Scene-only route; route equality is
not acceptance of these extra errors. Both additional Renderer lifetime
drivers pass, but actual MML/parser/Lite owner counts still accumulate. The
full Tree/MML constructor, six Stack method and broader owner-retirement
packets remain unaccepted candidates. Folia still uses math QuickJS. See
`evidence/shared-gate183.md`.

Earlier completed evaluation (2026-10-16 local, Gate180): all 59 targets compile
with all 2,096 captured inputs unchanged through evaluation. The generic
Factory, StackItem and Lite descriptor integration exposes a service-graph
initialization-order regression: public Renderer initialization fails before
API, full SVG and paired Scene captures can complete. Core, Host, Parser and
Foundation fail; Regexp and LinkedList pass. Five of 25 package captures and
one of 20 checkpoint comparisons pass. The two additional Renderer drivers
show runtime exception windows and are explicitly terminated; neither passes.
They now have top-level exception reporting pending the next build. The
initialization-order fix is being tested separately. Gate177 remains the last
passing full SVG comparison, not evidence that the current increment passes.
See `evidence/shared-gate180.md`. Folia is not yet integrated.

Earlier completed evaluation (2026-10-16 local, Gate177): all 59 targets compile
with all 2,087 captured inputs unchanged through evaluation. Public SVG/errors
pass 150/150, API 9/9, Common layout 75/75, paired Scenes 146 equal successful
results plus four equal error pairs, and Scene-only materialize/XML counts
remain zero. All 25 package captures pass; 18/20 checkpoint comparisons pass.
Host, Parser, Regexp and LinkedList pass; Core has five failing tests and
Foundation six. Private Renderer wrapper/LayoutJob retirement is integrated:
both actual owner counts stay zero through 64 repeated renders, with macro,
explicit tag/reference and saved-result checks passing. Other owners still
accumulate, and the broader lifetime diagnostic fails both its first run and
exact repeat. Post-closure instrumentation identifies pre-session UTF-8
cancellation in its in-flight fixture; preparing UTF-16 first passes a separate
run against the existing libraries. The corrected fixture and Lite descriptor
changes still need a shared build. Evaluation closes with exit 1; Folia is not yet integrated. See
`evidence/shared-gate177.md`.

Earlier completed evaluation (2026-10-16 local, Gate175): all 58 targets compile
and all 2,082 captured inputs remain unchanged. Public SVG/error comparison
passes 150/150, API 9/9, Common layout 75/75, and paired native routes retain
146 equal complete Scenes plus four equal error pairs with zero SVG
materialize/XML serialization calls on the Scene-only route. All 25 package
captures pass and 18/20 checkpoint comparisons pass. The corrected factory
fixtures now pass in the shared build; Host, Parser, Regexp and LinkedList
pass. Core and Foundation retain parent/callback and MML failures. Actual
owner counters expose accumulation across repeated calls in one Renderer;
the additional macro/tag continuity diagnostic exits 2 and needs Source
comparison before its expectations or implementation are corrected. No
automatic owner retirement or Folia integration has been accepted. See
`evidence/shared-gate175.md`.

Earlier completed evaluation (2026-10-16 local, Gate174): all 57 targets compile.
After preserving an interrupted API attempt and restoring one accidental
agent edit to the exact frozen bytes, the complete evaluation restarts with
all 2,081 inputs matching the build manifest and remains unchanged through
closure. Full SVG/error comparison passes 150/150, API 9/9, and Common layout
75/75. The Scene route retains 146 equal successful pairs and four equal error
pairs with zero SVG materialize/XML serialization calls. All 25 package
captures succeed and 18/20 checkpoint comparisons pass. Core, Host and
Foundation still fail: remaining owner-query/callback-fixture and Scene parent
operations need corrections. Parser, Regexp and LinkedList pass. Next-six/font
still expose MML factory and StackItem field differences. See
`evidence/shared-gate174.md`; this is not renderer acceptance.

Earlier completed evaluation (2026-10-16 local, Gate171): all 57 targets compile
with 2,078 unchanged inputs. Public Renderer full SVG/error comparison passes
150/150 and API passes 9/9. The Scene route has 146 equal successful frozen
layout pairs and four equal error pairs, with zero materialize/XML serializer
calls across all 150 Scene-only records. All 25 package captures succeed.
Common layout observations now pass 75/75 and the XML comparison passes;
18 of 20 checkpoint comparisons pass. Core, Host and Foundation retain
factory/parent and callback metadata failures. MML factory and StackItem own
fields remain different in the next-six/font observations. Evaluation closes
with exit 1 and zero changed inputs. See `evidence/shared-gate171.md`.

Gate170 failed compilation at RegExpBindings' use of a private Context field
API. Gate171 instead retains weak handles to the binder's own fields and
invalidates them directly during clear; the RegExp lifetime tests now pass.

Earlier completed evaluation (2026-10-16 local, Gate169): all 57 targets compile
with 2,075 unchanged inputs. Public Renderer full SVG/error comparison passes
150/150 against the pinned clean Source capture, and API passes 9/9. The paired
Scene route has 146 equal successful frozen layouts and four equal error pairs;
Scene-only materialize/XML serializer counts are zero across all 150 records.
All 25 package captures succeed. Core, Host and Foundation still fail; Parser,
Regexp and LinkedList pass. Layout observations are 74/75 (TeXHTML paint count),
and XML/callback metadata observations remain different. Evaluation closes
with exit 1 and zero changed inputs. See `evidence/shared-gate169.md`.

Earlier completed evaluation (2026-10-16 local, Gate168): all 57 targets compile
with 2,075 unchanged inputs. Public API is 9/9 and all 25 package captures
succeed. The current full SVG comparison is 148/150: only the enabled TeXHTML
case fails in both owner groups, with `unsupported Lite owner field`. Its
importer reads children on text/comment leaves; a correction is awaiting the
next build. A separate paired route capture has 144 identical successful
frozen Scene pairs and six equal error pairs; the Scene-only route records
zero materialize/XML serializer calls across all 150 records. These pairs
establish route equality, not independent Source geometry acceptance.
Core, Host and Foundation still fail; Parser, Regexp and LinkedList pass.
Sixteen of twenty checkpoint comparisons pass. All evaluation inputs remain
unchanged, and the failed evaluation is closed. See `evidence/shared-gate168.md`.

Earlier shared build (2026-10-16 local, Gate167): all 57 targets compile with
all 2,075 captured inputs unchanged. The native document probe passes and no
access violation has occurred in the completed probes. Public API, complete
SVG and layout capture now stop at TextMacros CONFIG argument decoding.
Core, Host, Parser and Foundation have current failures under investigation;
Regexp and LinkedList pass. All evaluation processes have finished and the
post-evaluation audit also records zero input changes; evaluation exits 1.
No current complete SVG or paired Scene comparison is claimed. Gates163-166
were compile failures; descriptor, callback signature and UT formatting
corrections close those errors. See `evidence/shared-gate167.md`.

Earlier shared builds (2026-10-16 local, Gates161-162): the Scene-only producer,
native text measurement, cancellation, live factory/filter fields, RegExp
lastIndex and Promise queue-owner changes are integrated for verification.
Both builds fail compilation with all 2,073 captured inputs unchanged. The
first build's dependent-name, numeric-assignment and test-header issues are
corrected; the second exposes remaining factory binding/test type updates.
No runtime or SVG comparison is claimed for these builds. The Gate160 Scene
clone regression has an explicit independent-owner correction, but it still
needs a working current build. Source-only witnesses pass; they do not replace
native acceptance. See `evidence/shared-gate162.md`.

Latest shared runtime evaluation (2026-10-16 local, Gate160): the Source list-operation
and live Scene-owner changes compile across all 55 targets with 2,050 captured
inputs unchanged, after fixing the Gate159 type errors. Public API, native
document, complete SVG capture and Core encounter an access violation; the
layout capture stops in the first plain conversion. Foundation has two failing
tests. Host, Parser, Regexp and LinkedList pass, and all 25 package captures
finish successfully. No current complete SVG comparison is claimed. The
evaluation is closed as failed; the Scene runtime regression is being diagnosed.
The last passing public SVG capture remains Gate158. See
`evidence/shared-gate160.md`. Folia is unchanged and Mermaid remains paused.

Earlier shared evaluation (2026-10-16 local, Gate158): all 55 targets compile
with 2,042 captured input entries unchanged. Complete Core, Host, Parser,
Regexp, LinkedList and Foundation suites pass again. A fresh native public
Renderer capture matches all 150 complete SVG/error records against the
locked clean-runtime Source capture; API is 9/9 and layout observations 75/75.
All 25 package captures succeed. Seventeen of the twenty comparisons pass;
fonts remain 13/16 and next-six 18/20, now differing at RegExp lastIndex own
keys exposed by the repaired FindTeX fields. The two XML namespace
differences remain. Source list operations, remaining live fields, Scene-only
output, cancellation and owner-lifetime candidates still need integration
and verification. See `evidence/shared-gate158.md`. Full renderer acceptance
is incomplete; Folia still uses math QuickJS and Mermaid remains paused.

Earlier shared evaluation (2026-10-16 local, Gate157): all 55 targets compile
with 2,041 captured input entries unchanged. The focused missing-slot,
canonical callback argument and FindTeX checks pass; the real native document
entry point also passes. Complete Core finishes without the earlier crash,
but Core, Host, Parser and Foundation still fail. The complete package profile
now stops at TextMacros CONFIG argument decoding; API, layout and full SVG
capture fail, so no current full SVG comparison is available. Five of the
25 package captures fail. Comparison commands are not run without a working
public renderer. Candidate fixes cover the remaining CONFIG codecs and list
iterator/test regressions. Folia remains unchanged; Mermaid remains paused.
See `evidence/shared-gate157.md`.

Earlier shared evaluation (2026-10-16 local, Gate155): all 55 targets compile
with 2,040 captured input entries unchanged, but runtime acceptance fails.
The new shared list owner exposes missing canonical callback argument
conversion during renderer initialization. API and complete SVG capture exit
with errors, so no complete SVG parity result exists for this code. Core also
encounters a null list-record dereference in an existing missing-slot test;
the focused reproduction confirms the same failure. Host and Parser fail;
Regexp and LinkedList pass. Failed captures cannot establish rendering
differences. The evaluation is closed as failed, and repairs are in progress.
See `evidence/shared-gate155.md`. Folia remains unchanged and Mermaid paused.

Last passing shared test suites (2026-10-16 local, Gate150): all 55 targets compile
with 2,031 captured input entries unchanged. Complete Core, Host, Parser,
Regexp and LinkedList pass. Full public SVG/error parity is 150/150 after
independently capturing fresh Source module runtimes; Gate149's reused-module
font-cache differences remain preserved as evidence. Thirty-one unchanged
native binary observations retain the Gate149 results, including failures:
Foundation passes, layout 75/75, API 9/9 and complete texttip SVG exact; three
package/XML comparisons still fail. Fonts remain 13/16 and next-six 18/20
at live FindTeX properties; two XML differences remain. See
`evidence/shared-gate150.md`. Full renderer acceptance and Folia integration
remain incomplete; Mermaid is paused. No runner or directory copies are made.

Earlier shared evaluation (2026-10-16 local, Gate149): all 55 targets compile
with 2,021 captured inputs unchanged. The new complete public SVG comparison
matches 146/150: continuous sessions 75/75, fresh sessions 71/75. Four fresh
differences remain under investigation. Layout remains 75/75 with all 38
registrations, API 9/9 and complete texttip SVG exact. All 25 captures pass;
the existing 20 comparisons still have three failures. Fonts 13/16 and
next-six 18/20 now differ at missing live jax.findTeX properties after the
initial BaseMappings generation fix. Two XML differences remain. Parser,
Regexp, LinkedList and Foundation tests pass; Core and Host stop at a new
empty-optional runtime assertion and are recorded as failures. See
`evidence/shared-gate149.md`. The renderer is not accepted, Folia unchanged,
Mermaid paused; no runner or directory copies are created.

Earlier shared evaluation (2026-10-16 local, Gate146): all 54 targets compile
with 2,002 captured inputs unchanged. All 25 captures pass, including
StackOwners; layout remains 75/75 with 38 registrations, API 9/9, complete
texttip and sequential public Renderer SVG 3/3 exact. Host, Parser,
Foundation, Regexp and LinkedList pass. Core has one new processor-test
fixture initialization exception; its actual-session replacement is ready.
Thirteen callback metadata mappings now match Source; fonts 13/16 and
next-six 18/20 advance to extra stretchy writes in registered token attributes.
Two XML differences remain. See `evidence/shared-gate146.md`. No runner
copies are created; the renderer is not accepted, Folia unchanged, Mermaid paused.

Earlier shared evaluation (2026-10-16 local, Gate145): all 54 targets compile
with 2,000 captured inputs unchanged. One actual public Renderer processes
proof/formula/proof sequentially; all three complete SVG strings match pinned
MathJax exactly, with completed Scenes and no errors. StackOwners now passes
the Source-correct MML Push check. The 33 other diagnostic/comparison binaries
retain their recorded hashes, so prior passing Core/Host/Parser/Foundation/
Regexp/LinkedList, layout 75/75 with 38 registrations, API 9/9 and complete
texttip SVG results are reused. Fonts remain 13/16, next-six 18/20, and two
XML differences remain. See `evidence/shared-gate145.md`. No runner copies
are created; the renderer is not accepted, Folia unchanged, Mermaid paused.

Earlier shared evaluation (2026-10-16 local, Gate144): all 53 targets compile
with 1,995 captured inputs unchanged. Both new test fixtures are corrected;
complete Core, Host, Parser, Regexp and LinkedList pass. The 29 binaries for
Gate143's package, layout, API and Foundation checks retain identical hashes,
so those observations are reused, including failures. Layout is 75/75 with
38/38 actual registrations; API is 9/9 and complete texttip SVG is exact.
StackOwners still fails its MML Push assertion, fonts stay 13/16, next-six
18/20, and two XML differences remain. A pinned Source witness for the MML
Push assertion is ready but its proposed fix is not enrolled in this gate.
See `evidence/shared-gate144.md`. No runner copies are created; the renderer
is not accepted, Folia is unchanged and Mermaid remains paused.

Earlier shared evaluation (2026-10-16 local, Gate143): all 53 targets compile
with 1,995 captured inputs unchanged through evaluation. The complete layout
profile now matches 75/75, all 38 packages are actually registered, API is 9/9
and complete texttip SVG is exact. Host, Parser, Foundation, Regexp and
LinkedList pass; Core has two new-test failures (ordinary x glyph expectation
and Bboldx fixture initialization). StackOwners advances past the repaired
MathItem root and fails its MML Push owner check. Fonts 13/16 and next-six
18/20 now expose a nested preprocessor callback arity. Two standalone XML
namespace differences remain. Factory raw fields, replaceable list owners,
live XML, reclamation and cancellation still need completion and verification.
No runner copies are created. See `evidence/shared-gate143.md`.
The renderer is not accepted; Folia is unchanged and Mermaid remains paused.

Earlier shared evaluation (2026-10-16 local, Gate141): all 53 targets compile
with 1,989 captured inputs unchanged. Complete Core, Host, Parser, Foundation,
Regexp and LinkedList pass again. The six Stack regressions are fixed;
ordinary object conversion and captured join length have Source witnesses.
The original Bussproofs comparisons pass, including expanded 21/21 and exact
session observations. ParserFields passes replacement and alias checks.
The new StackOwners fixture still fails its unset MathItem root; its repair
is candidate-only. Full rendering remains 74/75 and 37/38 actual registrations,
API is 9/9 and complete texttip SVG is exact. Fonts 13/16 and next-six 18/20
now expose callback name and configurations list projection respectively.
Two standalone XML differences remain. Gate139's accidental factory-test edit
was rejected and recovered byte-exactly. A later supplemental Source-witness
edit occurred after Gate141 evaluation ended and is recorded separately.
No runner executable copies are made. See `evidence/shared-gate141.md`.
The renderer is not accepted; Folia is unchanged and Mermaid remains paused.

Earlier shared evaluation (2026-10-15 local, Gate137): all 53 targets compile
with 1,982 captured inputs unchanged. Stack/parser owners are now in the shared
graph, but this checkpoint is not accepted: Core has six Stack regressions,
Host has two outdated boundary expectations needing Source verification, and
the independent Bussproofs/StackOwners/ParserFields checks fail. Parser,
Foundation, Regexp and LinkedList pass. Full rendering remains 74/75, its
native observation byte-identical to Gate134; API is 9/9 and texttip SVG exact.
The actual FunctionList configMethod identity is present; font/next-six now
expose a callback arity difference. Two standalone XML differences remain.
Test tooling verifies the frozen shared binaries directly and no longer
copies a roughly 2.2 GB runner set each gate. See `evidence/shared-gate137.md`.
Folia remains unchanged; Mermaid is paused.

Earlier shared evaluation (2026-10-15 local, Gate134): all 51 targets compile
with 1,968 captured inputs unchanged. Complete Core, Host, Parser, Foundation,
Regexp and LinkedList pass. Full rendering now matches 74/75: tooltip's entire
record matches through the shared layout-observation boundary. Bussproofs
remains unregistered. Public API stays 9/9 and the complete texttip SVG stays
exact. The standalone TeX fixtures now match Source construction; fonts 13/16
and next-six 18/20 expose a missing configMethod.items field next. TexHtml and
DOMStrings pass; two independent XML namespace differences remain. Stack,
parser, factory fields, XML creator, lifecycle and cancellation remain required.
See `evidence/shared-gate134.md`. Folia is unchanged; Mermaid remains paused.

Earlier shared evaluation (2026-10-15 local, Gate133): all 51 targets compile
with 1,965 captured inputs unchanged. Complete Core, Host, Parser, Regexp and
LinkedList pass. Foundation's XML font fallback test now passes; its one new
tooltip test has the wrong branch expectation and requires correction.
Rendering remains 73/75: tooltip now reaches the exact Source error, 23 calls
and 21 paints, but lacks its nested layout observer snapshot; Bussproofs remains
unregistered. Public API is 9/9 and the complete texttip SVG remains exact.
Standalone XML captures again complete, revealing two extra native-check
envelope fields as well as the two existing namespace differences. These are
not accepted. Fonts/next-six still expose their standalone fixture and pending
Stack/parser fields. Required owner, XML creator, lifetime and cancellation
packets remain under review. See `evidence/shared-gate133.md`. Folia is
unchanged and Mermaid remains paused.

Earlier shared evaluation (2026-10-15 local, Gate131): all 51 targets compile
with 1,963 captured inputs unchanged. Complete rendering now matches 73/75;
both TexHtml examples and valid Empheq match. Public API remains 9/9 and the
complete texttip SVG stays exact. Remaining renderer records are tooltip
(missing Context processMath method) and unregistered Bussproofs. This is not
an accepted checkpoint: Core has four failing new test fixtures/assertions,
Foundation has one failing new font-family test, and the XML capture exits
with an eager Lite-node argument check. Tags metadata matches its pinned CJS
witness; the fonts/next-six comparisons now expose a standalone Jax adaptor
setup difference and the pending parser/Stack alias. Stack/parser, XML creator,
factory field, lifetime and cancellation packets remain under review. See
`evidence/shared-gate131.md`. Folia is unchanged and Mermaid remains paused.

Earlier complete shared checkpoint (2026-10-15 local, Gate129): all 51 targets
compile with all 1,957 captured inputs unchanged. Complete Core, Host, Parser,
Foundation, Regexp and LinkedList pass. Public SVG remains 9/9 and texttip's
complete SVG remains byte-exact. Complete rendering matches 70/75, with 37/38
registered packages and one warning. MML fields/factory and Empheq now compile
and participate in the real session; the invalid Empheq option record matches.
Remaining renderer records are math tooltip, Bussproofs, valid Empheq and both
TexHtml examples. Next-six remains 18/20 and fonts 13/16, now exposing Tags
method metadata and a factory alias difference beyond the MML fields.
The independent TexHtml/XML comparison has two namespace failures; earlier
Gate126 reporting incorrectly said all twelve records passed. Its raw report
already contained those failures. Lifetime and cancellation implementation
packets are drafted, not accepted; Folia integration remains pending. See
`evidence/shared-gate129.md`. Mermaid remains paused.

Earlier complete shared checkpoint (2026-10-15 local, Gate126): all 51 targets
compile with all 1,948 captured inputs unchanged. Core, Host, Parser,
Foundation, Regexp and LinkedList pass; public SVG remains 9/9. Complete
rendering matches 69/75, with 36/38 registered packages and two warnings.
Cases, Centernot, Textcomp, HTML and TexHtml now join the actual session with
their canonical module owners. The independent XML comparison has two
namespace regressions (corrected reporting after Gate129). Gate127 changes
only the texttip witness options to match the pinned
profile: its complete 1,315-character SVG now matches Source exactly; that gate
does not claim another complete 75-record run. Remaining full-renderer cases
are math tooltip, Bussproofs, both Empheq examples and both TexHtml examples.
MML field/factory, Empheq and Stack packets are saved but remain unregistered
and uncompiled. Repeated-render lifetime acceptance still blocks Folia
integration. See `evidence/shared-gate126.md`. Mermaid remains paused.

Earlier shared checkpoint (2026-10-15 local, Gate123): all 51 selected targets
compile with all 1,944 captured inputs unchanged. Core, Host, Parser,
Foundation, Regexp and LinkedList pass; public SVG remains 9/9. Complete
rendering matches 63/75, with 31/38 registered packages and seven warnings.
Units and AMSCD now use canonical per-runtime method exports and participate
in the actual SVG session. All prior passing package comparisons remain
passing. Next-six and fonts still expose incomplete MML node field projections
(18/20 and 13/16). Texttip's collision is precisely the factory's `jax` field:
repeated canonical factory projection attempts to bind it twice. Its narrow
owner-reuse fix and regression await the next gate. See
`evidence/shared-gate123.md`. Folia and Mermaid remain unchanged.

Earlier shared checkpoint (2026-10-15 local, Gate121): all 51 selected targets
compile with all 1,941 captured inputs unchanged. Core, Host, Parser,
Foundation, Regexp and LinkedList pass; public SVG remains 9/9. Complete
rendering matches 60/75, with 29/38 registered packages and nine warnings.
Mhchem again passes all 220 cases and every alias/replacement/map/mutation
record; its native observation is byte-identical to Source. Existing passing
package comparisons remain passing. The handler-owner probe now passes its
prototype metadata assertions. Next-six remains 18/20 and fonts 13/16,
exposing MathItem fields and observer alias traversal respectively. Texttip
still fails with a native-field binding collision; the exact owner/key needs
diagnosis. Gate120 ended with a UTF-16 diagnostic streaming compile error;
Gate121 fixes it without changing assertions. No application or Mermaid code
changed; complete renderer and lifecycle acceptance are still required.

Earlier shared checkpoint (2026-10-15 local, Gate119): all 51 selected targets
compile with all 1,937 captured inputs unchanged. Core, Host, Parser,
Foundation, Regexp and LinkedList pass; public SVG remains 9/9. The Maction
base-initialization fix removes the native process exit. Complete rendering
now matches 59/75, with 29/38 registered packages and nine warnings.
Mathtools, Physics, Colortbl, original/expanded Bussproofs, Tag/SetOptions and
its defaults retain parity. The new exact Bussproofs module-scope comparison
passes, including its cleared-document error. Mhchem matches all 220 cases
but its BaseMethods alias check regresses; font aliases also regress after
Mathtools loading reveals BaseMethods table reinitialization. Canonical table
guards and regressions are saved for Gate120. Next-six remains 18/20, now
exposing missing StackItemFactory fields. Texttip and action records expose
missing canonical wrapper/MML fields and methods; their owners are correcting
those shared boundaries. A complete Source comparison and the original scope
remain required before application integration. The independent Stack binder
and lifetime/integration reviews are unregistered; Folia and Mermaid are unchanged.

Earlier shared checkpoint (2026-10-15 local, Gate118): all 51 selected targets
compile with all 1,936 captured inputs unchanged. Complete Core passes
345,722 assertions / 5,337 tests / 173 suites; Host, Parser, Foundation,
Regexp and LinkedList all pass. The public SVG/error API matches 9/9.
Mathtools, Physics, Colortbl and original Bussproofs retain full package/map
parity; expanded Bussproofs now matches 21/21, Tag/SetOptions 23/23 plus its
default-method witness, and Mhchem 220/220 with every replacement/map/mutation
record. TexHtml/DOMStrings/XML, Attributes, parser projections and host/layout
witnesses pass. Next-six remains 18/20 and fonts 13/16, now exposing a repeated
handler identity difference after their missing callable identities were fixed.
The full renderer and texttip terminate during maction execution; there is no
complete Gate118 renderer record or accepted count. Gate117 below remains the
last complete 75-record comparison. A new handler-prototype probe also fails
on emitted method metadata; its correction awaits the next build. Physics is
registered in the renderer; Mathtools registration awaits its canonical module
export/counter owner. Folia and Mermaid remain unchanged.

Earlier shared checkpoint (2026-10-15 local, Gate117): all 51 selected targets
compile with all 1,935 captured inputs unchanged. Complete Core, Host, Regexp
and LinkedList pass. The public SVG/error API matches 9/9 complete Source
records. Full rendering matches 56/75, with 27/38 registered packages and 11
configuration warnings; it is not accepted. Mathtools matches 42/42 cases and
5/5 maps, Physics 63/63 and 11/11 maps, Colortbl 29/29 and its map, original
Bussproofs 31/31 and both maps. TexHtml/DOMStrings/XML matches all 12 records;
Attributes and both parser projection records match their Source witnesses.
Expanded Bussproofs exposes four empty-content differences (17/21 matched).
Parser exceptions, missing callable identities in two package probes, texttip's
missing shared String.trim intrinsic, module import visibility, and three
Foundation fixture corrections remain. See `evidence/shared-gate114-next.md`.
Folia still uses its existing math worker; Mermaid remains paused.

Earlier, Gate114 selected 50 targets and terminated with compilation errors; all 1,932
captured inputs remained unchanged. The new canonical binding interfaces and
Runtime wiring are now present. Remaining build repairs concern Map access and
iteration, string/Property overloads, binding temporaries, and observer/test
types. No Gate114 runtime result is claimed. See
`evidence/shared-gate114-next.md`.

Earlier shared checkpoint (2026-10-15 local, Gate113): all 46 selected targets
compile with 1,904 captured inputs unchanged. Core passes 345,659 assertions /
5,325 tests; Host, Regexp and LinkedList pass. Parser and the new TexHtml tests
still fail. Former Scene layering failures pass. Full renderer is 53/75,
public SVG 8/9, actual registration 23/38. Colortbl is 29/29, Physics 61/63,
next-six 16/20, fonts 13/16. Mhchem matches all 220 package cases and 7,960
dependency records; its intermittent Unicode difference is proven to come
from chunked stdout decoding, now corrected with a byte-boundary regression.
Root enrolls Colortbl/Mhchem for the next build; Folia is unchanged. Complete
handler graph bindings are assigned as explicit Luna packets. See
`evidence/shared-gate111-next.md` and `canonical-handler-graph-plan.md`.

Earlier shared checkpoint (Gate110): all 38 selected targets
compile/link with 1,836 captured inputs unchanged. Complete Core passes
345,659 assertions / 5,325 tests / 171 suites. Host, Regexp and LinkedList pass;
Parser and three typed Scene tests still fail. The full renderer matches
47/75 records (64 layouts, 11 errors); public SVG/error records match 8/9.
Registration remains 23/38. Mhchem dependency comparison passes 7,960/7,960;
its recorded package capture is 219/220, with one Unicode difference absent
in diagnostic reruns. Physics is 49/63, next-six 15/20, fonts 13/16. No new
package is accepted. See `evidence/shared-gate107-110.md`.

Gates111–112 enroll 46 targets, including the complete new package packets,
but have not passed compilation. Inputs remained unchanged in both builds;
remaining observer/test API corrections are assigned to their Luna owners.
The shared TextParser list-identity cause is identified and corrected in a
candidate awaiting the next build. See `evidence/shared-gate111-next.md`.

Current implementation work is primarily Luna. Root owns architecture and
acceptance. Per-session MapHandler and Bussproofs state, arbitrary Context
callable identity, HTMLDomStrings/document wiring, and the remaining package
and Scene differences are in progress. One focused Sol review covers only
the callable boundary. Folia integration and Mermaid remain unchanged.

Earlier shared checkpoint (2026-10-15 local, Gates102–106): Gate106 builds all
31 targets with 1,804 captured inputs unchanged. Runtime acceptance fails:
the live parser Array needs the missing generic unshift operation, all75
renderer records fail, and Core reports two SourceStringMap errors. Host,
Regexp and LinkedList pass. The seven Scene Context tests pass 56 assertions;
four typed layout-data tests remain blocked during parsing. Registration stays
23/38. Gate107 failed one ParseOptions compilation unit with all 1,804 captured
inputs unchanged; that owner correction and the runtime fixes await Gate108.
Neither earlier isolated passes nor the successful compile certify these edits. See
`evidence/shared-gate102-106.md`.

Current source candidates (2026-10-15 local):

- The public `folia.math` facade compiles against the completed Gate101
  library. Adding the current table separator correction gives 9/9 exact
  public SVG/error records against the pinned Source. All nine returned
  Scenes remain traversable after Renderer destruction (213 nodes).
- The generic Lite node/create/setAttributes dispatch correction passes an
  isolated Foundation run: 390 assertions in 59 tests, including the new
  live-method replacement and exception propagation regression.
- These are isolated diagnostics over the Gate101 dependency library; they
  do not certify current shared-module edits or complete package support.
  The latest fully passing Core checkpoint remains Gate101. Physics's initial isolated compile
  identifies import and type errors; its owner is correcting the complete
  package. Canonical parser projections and the shared SVG Scene Context
  projection remain in progress. Folia has not switched its math path.
  See `evidence/public-api-isolated-g101.md`.

Active complete shared-build checkpoint (2026-10-15 local, Gates99–101):

- Gate99 fails because an automatically collected draft test imports an
  unregistered module. After moving the draft, Gates100 and 101 build all
  28 targets with 1,761 captured inputs unchanged. Input capture now includes
  automatically discovered math tests and local quoted includes.
- Gate101 complete Core passes 345,659 assertions in 5,325 tests across 171
  suites. Host, Parser, Regexp, LinkedList and Foundation also pass. Gate100
  Font tests and the complete raw DynamicFile Source comparison pass.
- Complete75 improves to 53/75 exact records; registration remains 23/38
  with 15 warnings. The cancel-lines record now matches completely. Actual
  `x+y` SVG remains byte-exact and detached Scene checks pass.
- Table comparison remains 12/14. Its remaining aligned-label SVG separator
  correction awaits a new compile. Next-six remains 0/20 with all records
  emitted; Font3 remains 10/12. No partial package is newly registered.
- Luna owners continue package semantics, Physics, and Maction/TimerHost.
  Folia integration remains pending; Mermaid remains paused.
  See `evidence/shared-gate99-101.md`.

Earlier native execution checkpoint (2026-10-14 local, Gate98):

- All 28 targets build with 1,631 captured input hashes unchanged. Complete75
  remains 52/75 exact, with 23/38 packages registered and 15 warnings.
- Complete Core passes 345,656 assertions in 5,324 tests across 171 suites.
  Foundation, Host, Parser, Regexp, LinkedList and Font tests also pass.
  Three instance-extension records match exactly; actual `x+y` SVG and
  detached Scene checks remain green.
- Table comparison remains 12/14; label paint now agrees and the remaining
  aligned-label difference is full-width SVG behavior. Menclose Common boxes
  and placement calls for cancel-lines now agree, but paint/SVG and its
  separate 30-case method traces still require work.
- Next-six emits all 20 records but matches 0/20; Font3 matches 10/12. Raw
  DynamicFile observation stops at the unimplemented data-to-accessor
  descriptor change. These are explicit remaining acceptance failures.
- Follow-up fixes remain uncompiled. Folia integration remains pending;
  Mermaid remains paused. See `evidence/shared-gate98.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates95–97):

- Gate95 is cancelled before its build command starts; Gate96 fails compilation.
  Gate97 builds all 27 targets with 1,628 captured input hashes unchanged.
  Complete75 improves to 52/75 exact records; registration remains 23/38 with
  15 warnings. The renderer has not passed acceptance.
- Table/spacing improves to 12/14 exact records. Label-node creation order and
  colortbl parsing remain. Actual `x+y` SVG stays byte-exact and its detached
  Scene checks pass. Foundation passes 383 assertions in 58 tests.
- The remaining Core assertion still fails in Gate97; a corrected expected
  file key awaits a fresh build. Gate82 remains the last fully passing full
  Core run. Next-six's canonical token-map callbacks still need completion
  before its full 20-record comparison and production registration.
- Folia integration remains pending; Mermaid remains paused.
  See `evidence/shared-gate95-97.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates93–94):

- Gate94 builds and links all 27 targets with all 1,628 captured hashes
  unchanged. Complete75 finishes with 50/75 records exactly equal to Source.
  Actual registration remains 23/38 packages with 15 warnings; full renderer
  acceptance still fails.
- The table/spacing corpus passes 11/14 complete exact records. Remaining
  differences are column-rule arithmetic order, label placement and colortbl
  parsing. Actual `x+y` SVG remains byte-exact; detached Scene checks pass.
- Foundation passes 383 assertions/58 tests; Host passes 465 assertions/197
  tests. Parser, Regexp and LinkedList pass. Full Core retains one assertion
  failure in a dynamic-provider retry test; the earlier six provider failures
  have reduced to one. Gate82 remains the latest fully passing full Core run.
- Old-six and next-seven parser corpora pass 24/24 and 23/23. Font3 observation
  metadata and next-six's first-input exit still need fixes and fresh execution;
  their packages are not newly registered in production.
- Folia integration remains pending; Mermaid remains paused.
  See `evidence/shared-gate93-94.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates89–92):

- Gate92 builds and links all 27 targets with all 1,628 captured hashes
  unchanged. Actual `x+y` SVG remains byte-exact and its detached Scene
  lifetime checks pass.
- The 14-case table/spacing corpus completes: four mpadded records match
  exactly. Table construction order, labels, the box model and colortbl
  parsing still have differences under review.
- Old-six and next-seven parser corpora remain exact at 24/24 and 23/23.
  Font3 map callables and next-six initialization still require a fresh run
  after their current fixes; neither corpus is accepted.
- Foundation and Host each fail a new font/Context test. Full Core also has
  six dynamic-font-provider failures. Parser, Regexp and LinkedList pass.
  The iterator test remains unresolved after using the intrinsic Symbol.
- Complete75 exits during the 59th input, `verb-literal`; its partial output
  is retained only for diagnosis. Gate88 remains the latest completed75
  result (16/75), with 23/38 packages registered. Full acceptance and Folia
  integration remain open. See `evidence/shared-gate89-92.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates86–88):

- Gate88 builds all 26 targets with all 1,581 captured hashes unchanged. The
  whole driver emits 75 complete actual records: 16/75 match every Source field.
  It registers 23/38 packages and reports 15 missing-package warnings; complete
  renderer acceptance still fails.
- Actual `x+y` SVG is byte-exact (2,502 characters). Its shared Scene survives
  Context/graph/host release. Foundation passes 383 assertions/58 tests.
- The six-package and next-seven parser corpora pass 24/24 and 23/23. Font3
  remains 0/12 due to a canonical function length mismatch, corrected but not
  rerun. Instance extensions complete three actual rows but their comparer
  still rejects an empty-record schema difference.
- Gate88 Core/Host fail one newly added Symbol iterator test because its test
  input used a fresh Symbol instead of the intrinsic Symbol.iterator. The test
  source is corrected and awaits execution; Parser/Regexp/LinkedList pass.
  Gate82 remains the latest fully passing complete Core run.
- Remaining package and Scene placements, complete method/field fidelity,
  all75 parity and Folia integration remain open. Mermaid remains paused.
  See `evidence/shared-gate86-88.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates76–85):

- Gate85 builds all 26 targets with all 1,581 captured hashes unchanged.
  Actual `x+y` conversion now retains two SVG lines and their flow break in the
  completed Scene; Context/graph/host release and detached Scene reads pass.
  Its SVG still differs from Source, so output parity is not accepted.
- The six-package observer passes 24/24 complete Source records. The next-seven
  corpus remains 12/23; the font-package corpus remains 0/12 due to an observer
  reflection error. Other enabled packages still require implementation.
- Whole75 currently stops during actual instance font-extension initialization:
  the Jax font Context field is absent. No complete75 result is emitted.
  Gate70 remains the latest complete whole-profile run, with 0/75 full matches.
- Gate82 fresh full core passes 345,646 assertions/5,323 tests/171 suites; Host,
  Parser, Regexp and LinkedList also pass. Gate85 Foundation has one raw font
  array-field test failure. Common extension delegation and wrapper length
  conversion tests pass.
- Gate81 complete FontData tables and synchronous dynamic glyph/option/delimiter
  data pass strict comparison; arbitrary raw fields and actual Jax extension
  execution remain open. Menclose's actual method trace still differs.
  Folia integration remains pending and Mermaid remains paused.
  See `evidence/shared-gate76-85.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates73–75):

- Gate75 builds all 22 targets with all 1,531 captured hashes unchanged.
  Foundation passes 303 assertions/50 tests. The actual document probe completes
  an immutable Scene and SVG and verifies release of Context, graph and host;
  its `x+y` SVG remains unequal to Source.
- The full38 package request now reaches actual case conversion. Thirteen
  package handlers are implemented. The whole driver completes cases 1–27, then
  exits with a native access violation in case28, `braket-delimiters`. It emits
  no complete75 JSON result, so Gate70 remains the last complete whole-profile
  run (0/75 exact matches).
- Full FontData tables, extension merges and every synchronous dynamic glyph,
  option and delimiter now pass the strict Source comparer. The apparent empty
  merged arrays were an observer bug: it bypassed the live bound array accessors.
  Font tests pass 176 assertions/21 tests; lifecycle, methods, tuple and bundled
  provider observations also pass their exact comparers.
- The six-package parser observer aborts during token-map reflection with an
  unbound FunctionRef. Canonical function bindings and an isolated Braket parse
  mode are being prepared for the next build; they are not yet accepted.
- Live font Map replacement, SVG menclose instance dispatch, the remaining
  package handlers, complete75 output parity and Folia integration remain open.
  Mermaid remains paused. See `evidence/shared-gate73-75.md`.

Earlier native execution checkpoint (2026-10-14 local, Gates70–72):

- Gate71 builds all 21 targets with all 1,147 captured hashes unchanged.
  Actual `x+y` conversion now completes a native SVG and immutable Scene.
  The completed Scene survives release of the document, Context, graph and
  host. Its SVG still differs from Source and is not renderer acceptance.
- Gate70's whole run emits 75 actual records: 58 SVGs and 17 errors, with
  0/75 complete Source matches. Its base-only TeX selection means many SVGs
  are parser error output. Gate71 requests all 38 packages and records actual
  registrations/warnings separately; a missing live TeX.options binding
  prevents that run from reaching the first case.
- Fresh Gate71 core passes 345,635 assertions/5,322 tests/171 suites; Host,
  Parser, Regexp and LinkedList also pass. Foundation passes 295 assertions/
  49 tests. The rejected-provider fixture now distinguishes the actual NewCM
  placeholder's first fulfilled load from the subsequent failed-file rejection.
- Gate72 enrolls six Source-split TeX packages and five script placement pairs.
  Its 22-target build exits 1 with all 1,525 captured hashes unchanged. Script
  placements and the shared typed HTML/transform Scene interface compile;
  package module/import and syntax errors prevent linking and fresh execution.
- Complete FontData merges still fail the exact comparer. All 38 enabled
  packages, all 75 complete observations and Folia integration remain required.
  Mermaid remains paused. See `evidence/shared-gate70-72.md`.

Earlier native execution checkpoint (2026-10-13, Gate69):

- Gates68/69 build all 21 targets with all 1,147 captured hashes unchanged.
  Real document conversion now compiles actual TeX into MML and reaches Scene
  dispatch, where inferredMrow registration is missing. No SVG result is yet
  accepted. Both whole-profile runs emit 75 actual errors and match 0/75 full
  Source records. See `evidence/shared-gate67-69.md`.
- The actual NewCM module-provider trace matches 40 modules, 80 ordered events
  and 24,646 glyphs exactly. Full FontData option/extension behavior remains
  open. The Gate69 document probe verifies Context/graph/host release after an
  actual failed conversion; completed Scene lifetime remains unverified.
- Gate68's fresh full core exits 1 with one failed dynamic-font placeholder
  assertion/test; its other 170 suites pass. Host, Parser, Regexp and LinkedList
  pass. Foundation has two FontData option failures; its eight other packets
  pass, including the new shared Context numeric conversion witnesses.
- All 38 enabled profile packages, all 75 whole observations and Folia
  integration remain required. Mermaid remains paused.

Earlier document/font enrollment checkpoint (Gate66):

- Gates65/66 exit 1 with all 1,127/1,147 captured input hashes unchanged.
  Gate66 enrolls the real Core/HTML document and item owners, SVG assembly,
  dynamic NewCM module provider, actual conversion probe and whole-layout
  driver. Common/Scene observation compiles; document/tool boundary errors
  still prevent a fresh conversion run. See `evidence/shared-gate65-66.md`.
- The native session still configures only base. All 38 profile packages and
  all 75 complete Source output records remain required. No native TeX-to-SVG
  conversion or full layout parity has been accepted. Mermaid remains paused.

Latest complete passing output build and substrate checkpoint (Gate64):

- Gate64 builds all 13 targets with all 1,120 captured input hashes unchanged.
  Fresh Foundation passes 215 assertions/36 tests. The real SVG Jax now
  constructs through NativeRenderSession's actual Context-owned providers;
  lifecycle tests verify retained public handles and eventual service-graph
  destruction. Shared primitive/binary-plus coercion, RenderActions and Common
  menclose enter the compiled closure. Fresh parser corpora match 150/150 whole
  records and Unicode matches 37/37. See `evidence/shared-gate59-64.md`.
- This establishes actual construction and substrate behavior. The real
  MathDocument/HTMLMathItem conversion entry and whole-layout observer are
  still draft work; no complete native TeX-to-SVG conversion has been accepted.
  Complete Source own-fields/prototypes/methods, font extension/dynamic loading,
  XML/HTML and remaining SVG painting, the full 75-case comparison and Folia
  integration remain required. The enabled Folia profile remains unchanged.

Latest complete passing core/diagnostic execution (Gate63):

- The freshly linked Gate63 full core exits 0 with 345,620 assertions/5,320
  tests/171 suites. Host, Parser, Regexp and LinkedList also exit 0. Root runs
  byte-identical copied executables to avoid Windows linker locks during the
  next build. Gate63's overall build exits 1 because Foundation cannot yet link;
  Gate64 repairs that closure and executes the real Jax lifecycle tests. The
  full core has not been re-executed from Gate64, so its latest execution is
  explicitly Gate63. See `evidence/shared-gate59-64.md`.

Earlier complete passing build and core/diagnostic checkpoint (Gate49):

- Gate49 builds all 13 targets and keeps all 1,029 captured input hashes
  unchanged. Fresh full core passes 345,607 assertions/5,314 tests/170 suites;
  Host, Parser, Regexp and LinkedList tests also pass. All five parser corpora
  match 150/150 whole records, Unicode matches 37/37, and LinkedList matches
  13/13. Gate63 subsequently supplies the latest full core execution.
- Gate49's then-new Foundation runner exits 1: three Lite projection tests pass, but
  both Scene ownership tests throw during SVG serialization through an unbound
  LiteAdaptor. Subsequent fixes pass the focused Scene tests in Gates53 and 55.
  See `evidence/shared-gate48-49.md`. Complete native
  TeX-to-SVG and Folia integration are still unaccepted.

- Gates45 through 47 progress into actual Common per-kind bodies, output
  bindings and Scene/SVG painting. Gate47 exits 1 with all 1,029 input hashes
  unchanged. Its new actual Scene ownership tests compile but have not linked
  or executed; remaining errors are in six production files and the Lite
  projection test. Gate46 has one changed input during compilation and is not
  a clean frozen snapshot. See `evidence/shared-gate45-47.md`.
- Source constructor initialization required a separate semantic repair:
  C++ base construction prevented MathJax's derived virtual dispatch.
  Actual derived allocation, canonical identity and ordered initialization
  phases now compile and still need real layout verification. Full
  constructor/prototype/method coverage, FontData, remaining wrapper kinds and
  the production session lifetime remain open.

- The first current Common/Lite/Scene/SVG closure is explicitly enrolled in
  CMake. Gate42 stops on a missing Entities generation rule and has two changed
  mtr input hashes from Root's enrollment/snapshot communication race; it is not
  a valid frozen build. Gate43 fixes generation but stops on duplicate C++
  partition providers. Gate44 fixes those providers and reaches C++ compilation;
  it exits 1 on actual Lite declarations and CommonWrapper inheritance/imports.
  Gates43/44 each retain all 1,010 frozen hashes unchanged. Owners have repairs
  and are continuing the complete module packets. See
  `evidence/shared-gate42-44.md`; no native SVG path is accepted.

- Gate41 builds all 12 selected targets successfully with all 826 frozen
  input hashes unchanged. It fixes the Source-order BaseItems static error-table
  initialization that made Gate40's Core/Host/Parser abort before main. Fresh
  Host, Parser, Regexp and LinkedList runners all exit 0. All five parser corpora
  match 150/150 complete original records; LinkedList matches 13/13.
- Gate41's full core exits 1 with one failed Unicode assertion in
  `unicode_regexp_u_scans_code_points_and_keeps_utf16_indices`. The other 169
  suites pass 345,572 assertions/5,301 tests. The actual Unicode observer matches
  36/37 strict original records; the remaining tag-split record has identical
  parts and different RegExp `.source` slash spelling. The latest fully passing
  core remains Gate36. See `evidence/shared-gate39-41.md`.
- Root has enrolled the current Common/Lite/Scene/SVG module closure for its
  first full compile. Source-aligned draft bodies and module-closure checks
  establish neither native rendering parity nor Folia integration.

Previous checkpoints:

- Gate38 is the newest executed combined build and exits 1 with all 826
  input hashes unchanged. The fresh library and diagnostic executables link,
  but the core runner's Unicode test printer and LinkedList dump's cross-module
  MathLocation type ownership fail. The latest passing full core remains Gate36;
  repair edits require another build. See `evidence/shared-gate37-38.md`.
- Gate38's five fresh parser corpora match all 150 original records. HostTests
  passes 430 assertions/192 tests/10 suites. ParserTests now passes the new
  readonly/setter constructor-order cases, but fails four assertions in two
  tests preserving supplied object identities. RegexpTests fails two obsolete
  expectations that reject the newly implemented Unicode flag; they do not
  establish complete Unicode behavior. The actual production observer matches
  27/37 whole original Unicode records, including exact error text and patterns.
  An actual parser tag pattern fails to compile and remains a production gap.

- OutputJax and its FunctionList Context binding are registered and compiled.
  Gate27's original lifecycle comparison passes all 14 records and its tests
  pass 98 assertions in 21 tests. Complete common layout and SVG remain open.
- Gate34's combined build and ordinary core runner both exit 0. Core passes
  345,517 assertions/5,291 tests in 168 suites. Fresh Base, Frontend,
  Bold/ConfigMacros, AMS and AMS Boundary whole-record comparisons pass all
  150 observations. Equation's actual Source default display=true is restored.
  BoldTests passes 69 assertions/seven tests. ParserTests still fails two
  constructor/reflection tests; complete class acceptance remains open.
- Gate36 fixes Gate35's two compile errors; the complete target build exits 0.
  Registered RuntimeModules/LinkedList/MathList/MathLocation compile, the 13
  LinkedList original records match exactly, and lifetime tests pass 27/7.
  Native indexed-array provider/descriptor tests pass 17 assertions/four tests.
  All five parser corpora again match all 150 records. The full Gate36 core
  run passes 345,534 assertions/5,295 tests/169 suites, exit 0. Broader MathList
  coercion/getter acceptance is still open. See
  `evidence/shared-gate34-35.md` for exact scope and executable provenance.
- Three persistent Luna owners now implement Core DOM/Lite/Entities/font
  identities; CommonWrapper/factory/per-kind/linebreak algorithms; and
  CommonOutputJax and the completed scene/Source SVG placement/serialization.
  The two parser owners also resolve their
  remaining diagnostic regressions. New output packets are unregistered and
  unaccepted until their actual production graph compiles and compares.
- Root froze `common-layout-work-plan.md`: common measurement feeds one completed
  backend-independent scene; SVG serialization and future Direct2D consume that
  scene. The expanded original corpus has 75 cases, with 299 executable hashes
  locked, 31 observed wrapper kinds and four actual dynamic font requests.
  Whole results repeat exactly; tracing does not change the SVG/MML projection.
  Two actual Lite-host action exceptions remain in the Source records.
  A 37-row actual Unicode host corpus and corrected
  Entities source oracle also repeat exactly. See
  `evidence/common-layout-root-observations.md`; these are Source baselines,
  not native renderer acceptance.

Completed current-state evidence:

- MathJax-src 4.1.3 is cloned in .reference/MathJax-src, pinned to
  fb987178c2d279a99b0db5ea02e751691703955e. scope.json inventories 246 reachable
  upstream files for the enabled Folia profile.
- The C++23 folia_math target uses explicit module/source registration.
  Individual accepted packets have actual Source observations, Native checks
  and independent acceptance evidence. The last completed full core baseline
  was commit 2e58d8a. Gate23 worktree full core passes 345,517 assertions/
  5,291 tests; later uncompiled edits and failed parser diagnostics do not
  inherit that snapshot's acceptance.
- Utility, attribute, factory, tagged child-array/tree and parts of MML/TeX
  token/handler-map foundations exist. The architecture keeps semantic nodes,
  parsing, common layout and output responsibilities separate.
- P33/P34/P35 restores thirteen bounded MML consumer methods: error construction,
  spacing, row/identifier classes, style inheritance and subclass verification.
  The joint baseline passes 5,815 assertions in 238 executed tests and detects
  all 38 compiling semantic faults. The normal FoliaTests target builds and
  passes its new error filter plus constructor/base-verification regressions.
  See evidence/p33-p35-consumer-acceptance.md for precise scope and limitations.
- P36 restores fifteen table/multiscript/fraction/radical methods. Its independent
  baseline passes 19,609 assertions in 373 executed tests and detects all 39
  compiling semantic faults; the normal target's affected tests also pass.
  See evidence/p36-table-multiscript-acceptance.md.
- P37 restores thirteen operator/TeXAtom/under-over semantic methods, sharing the
  effective operator adjustment body. The independent baseline passes 32,828
  assertions in 162 executed tests and detects all 33 compiling semantic faults.
  The normal build and affected constructor/MML/script/operator filters pass.
  See evidence/p37-operator-script-acceptance.md for bounded scope and limitations.
- P38 restores nine action/MathChoice/menclose/mpadded methods. Its independent
  baseline passes 9,031 assertions in 1,452 executed tests and detects all 25
  compiling semantic faults. The normal build and affected filters also pass.
  See evidence/p38-action-choice-acceptance.md.
- P39 restores four mfenced methods. Its baseline passes 1,012 assertions in
  229 executed tests and detects all 21 compiling semantic faults. The complete
  normal core build/test now passes (build_test.bat, exit 0). No explicit MML
  Unimplemented stubs remain. See evidence/p39-mfenced-acceptance.md.
- P40 completes the 21 remaining NodeUtil methods and TexError's two methods.
  171 new utility observations and 1,981 TexError observations match the pinned
  original; all eight compiling semantic faults are detected. The complete core
  build/test passes, exit 0. See evidence/p40-parser-utilities-acceptance.md for
  the typed host boundaries and exact scope.
- P41 supplies all nine TeX NodeFactory methods, using the existing callable
  identity and MML graph with a narrow borrowed ParseOptions registration edge.
  All 78 original observations pass and all nine compiling semantic faults are
  detected. The full core suite passes, exit 0. See
  evidence/p41-node-factory-acceptance.md.
- P42A supplies MmlStack's eleven semantic methods over the same existing MML
  graph and node-array identity. All 76 original records pass; all twelve
  compiling semantic faults are detected. The full core suite passes, exit 0.
  This is one responsibility of StackItem.ts, not full stack/parser acceptance.
  See evidence/p42-mml-stack-acceptance.md.
- P42B completes BaseItem's seventeen methods and the StackItemFactory
  specialization over one actual shared StackItem identity, captured class
  metadata and Source constructor phases. All 85 original records pass; all
  seventeen compiling semantic faults are detected. Full core tests pass,
  exit 0. See evidence/p42b-base-item-acceptance.md. Stack.ts and concrete reducers
  are still required before actual TeX parsing.
- P43 supplies the actual parser Stack's nine responsibilities over existing
  BaseItem owners. All 72 original records pass; all sixteen compiling semantic
  faults are detected. Full core tests pass, exit 0. See
  evidence/p43-parser-stack-acceptance.md. Complete configured reducers,
  ParseOptions and TeXParser remain; the active next ownership boundary is P45.
- P44 moves the invocation context to its sole module above Stack and completes
  UnitMap/UnitUtil's nine responsibilities plus muReplace. All 245 original
  records pass; all sixteen compiling semantic faults are detected; full core
  tests pass, exit 0. See evidence/p44-unit-util-acceptance.md. P45 selects one
  parser-family module with Source-aligned partitions after a passing MSVC/CMake
  toolchain pilot. The actual parser-family classes are not implemented yet.
- P45 supplies core MathItem's actual fields, constructor, state table/registration
  and complete AbstractMathItem lifecycle through typed borrowed engine edges.
  All 92 original records pass; all twenty compiling semantic faults are detected;
  full core tests pass, exit 0. See evidence/p45-math-item-acceptance.md. Parser
  declarations use P46's single-runtime Context/Value option-graph decision in
  evidence/p46-parser-options-ownership.md. The actual declaration graph and
  native option callback/regexp bindings are the next implementation boundary.
  The real core InputJax base must also precede the actual TeX declaration.
- P47 supplies live Context option-array input to the same FunctionList algorithm
  and a converter-only binding to the canonical native function body. All 19
  original records pass, all seven compiling faults are detected, and full core
  tests pass. A Source subclass-default witness first failed and now confirms
  omitted priorities reach the actual virtual callee. See
  evidence/p47-option-filter-acceptance.md. Actual core InputJax is next.
- P48 supplies the real generic core InputJax base above the same MathItem
  compile service, including actual class/options/filters and the shared mutable
  filter record. All 30 original records pass, all fifteen compiling faults are
  detected, and full core tests pass. See evidence/p48-input-jax-acceptance.md.
  The next gate is the real complete parser-family declaration graph and its
  native class/array/regexp/Unicode bindings; no TeX parser is claimed yet.

- P49 supplies canonical typed native identities in the same Context option
  graph, with retained actual MML factory lifetimes, and the actual core
  AbstractFindMath superclass. Nine original identity/Options/MML observations,
  26 native ownership assertions, and eleven original FindMath observations
  pass. All eight host faults and five FindMath faults are detected; the full
  core suite passes. See evidence/p49-prerequisites-acceptance.md. Actual FindTeX
  and its UTF-16 non-Unicode regexp binding are the next implementation boundary.

- P50 supplies the native UTF-16 non-Unicode RegExp host needed by actual
  FindTeX. It uses SRELL code-unit traits and preserves configured punctuation
  delimiters. All 34 original observations, four explicit host-scope assertions
  and seven compiling faults pass their gates; full core tests pass. See
  evidence/p50-regexp-host-acceptance.md. The complete actual FindTeX boundary
  and remaining array/regexp bindings are recorded in
  evidence/p51-find-tex-declaration-frontier.md; its class is not implemented yet.

- P51 supplies actual option-array forEach and canonical RegExp bindings in the
  same Context graph. All 22 array and eight identity/cursor original observations,
  ten lifetime assertions and thirteen compiling faults pass their gates. Full
  core tests pass. See evidence/p51-bindings-acceptance.md. The actual complete
  FindTeX class is next; its prerequisite host bindings are now available.

- P52 completes the actual FindTeX scanner class and all seven method/constructor
  responsibilities over its existing superclass and runtime bindings. All 77
  original observations and ten compiling semantic faults pass their gates;
  the complete core build/test passes, exit 0. Root review corrected constructor
  arity metadata and live tuple iterator reads before acceptance. See
  evidence/p52-find-tex-acceptance.md. This is formula discovery; actual
  parser/configuration/package declarations and bodies are the next main route.

- P53 establishes the actual parser-family declaration graph, aligned with the
  Source class/file responsibilities and one named-module owner for genuine
  cycles. Compile-time contracts check canonical identities, all 26 BaseItems
  and selected field/signature types. Independent Source audits corrected tuple,
  nullable-return and initialization details before acceptance. These are
  declarations; parser-family algorithms remain unimplemented. See
  evidence/p53-parser-declarations.md.
- P54 supplies the portable Unicode 16 NFD host needed by ParseOptions and
  ParseUtil. All 99825 official corpus outputs, 85 exact UTF-16 sequences and
  17 complete-plane output fingerprints pass. All nine compiling NFD faults
  and four string-key Map faults are detected; correct baselines pass again.
  Independent Luna review and complete core build/tests pass, exit 0. See
  evidence/p54-nfd-acceptance.md. The next main route is actual parser-family
  method bodies and their module-lifetime registrations.

- P55 supplies canonical Context configuration-record inputs to the actual
  SubHandlers and NodeFactory registries, sharing their existing reducers, plus
  complete Label/TagInfo constructors. All 27 pinned original observations and
  eleven compiling faults pass their gates. Independent Luna reviews and the
  full core build/tests pass, exit 0. Root repaired three Source-semantic errors
  before acceptance and removed a test's shared-registry pre-state dependency.
  See evidence/p55-parser-record-acceptance.md. P56 freezes Array.map and runtime
  initialization requirements for the next actual Configuration methods.

- P56 supplies the actual ordinary Context Array.map host needed by
  Configuration, with retained prototype callable, holes, live getters and
  callback/return identities. All 41 original observations and two native-scope
  assertions pass; all eighteen compiling faults are detected; independent
  Luna review and full core tests pass, exit 0. Custom species remain explicitly
  unsupported. See evidence/p56-array-map-acceptance.md. The sustained complete
  Configuration/runtime, Tags/ColumnParser and independent original verification
  assignments are recorded in parser-family-work-plan.md.

Incomplete requirements and current executed checkpoints:

- Current verified worktree checkpoint is Gate27: all targets build with 814
  unchanged input hashes, and ordinary core passes 345,517 assertions in 5,291
  tests/168 suites. OutputJax is registered and freshly linked: strict 14/14
  original observations and 98 assertions/21 focused tests pass. BaseMethods
  now matches all 37 tree/parser observations. This supersedes older
  unregistered/pending-build descriptions below; it is bounded parity, not
  complete host or renderer acceptance. See evidence/shared-gate27.md.
- TextMacros' actual configuration count is zero. The parser owner found and
  is correcting package registration into the canonical configuration list;
  the correction is after Gate27 and awaits fresh compilation. Begingroup's
  failing wrapper-text UT expectation is invalid in pinned original CJS and
  is being corrected without changing production NodeUtil semantics. Empty
  arrow node-list/retained-tree filtering and tag display differences remain.
- The OutputJax owner is proceeding with the separate complete LiteAdaptor
  packet, including Source Entities utility and canonical Unicode-RegExp
  dependencies. One module/runtime graph store supports shared node identity
  across adaptors. No native LiteAdaptor/scene/SVG result is accepted yet.

- Gate 21 completes all eighteen focused build targets, exit 0, with unchanged
  registered/generated input hashes. Gates 19 and 20 did not link FontTests;
  their other freshly linked observer programs remain valid evidence for their
  actual source snapshots. Gate 22 builds the official core runner but its execution fails 17 assertions
  in two regex policy tests. Root isolated the SRELL namespaces in 937f91b;
  the freshly linked gate23 ordinary runner passes all 345,517 assertions/
  5,291 tests. Its ParserTests target alone fails on one overload ambiguity;
  that diagnostic correction and later production edits remain uncompiled.
  Two diagnostic candidate files were edited during its combined snapshot and
  their new assertions require a later build. They are not FoliaTests inputs.
- Fresh gate-19 HostTests passes 420 assertions in 190 tests; RegexpTests passes
  18,518 assertions in 41 tests. Structured Array.prototype.concat comparisons
  match 19 of 25 original rows, with zero supported mismatches and six explicit
  boxed-primitive/species/conversion boundaries. The comparator returns partial
  status, not complete host acceptance.
- NewCM/common/SVG FontData and three extensions now use one canonical sparse
  PropertyArray tuple model. Root's fresh gate-20 full-table comparison passes
  all four static sources and every synchronous dynamic glyph/options record
  and delimiter, resolver samples and class merge state. Tuple presence and
  identity comparisons pass; nine lifecycle traces and supplemental FontMethods
  observations also pass. Fresh gate-21 FontTests passes 97 assertions/11 tests.
  Eight independent corrupted output candidates are all rejected; those are
  comparator sensitivity checks, not compiled production fault tests. Full
  FontData member/profile acceptance and wrapper consumption remain separate.
  See evidence/font-styles-gate21.md for provenance and limits.
- Styles/StyleJson native modules compile. Fresh gate-21 focused tests pass
  19 assertions in six tests and the strict whole-record comparison passes all
  37 pinned original observations. The capturing String.split caller hang is
  fixed without changing shared RegExp.exec. The modules expose the bounded
  typed interface; dynamic Context constructors and browser CSS are not accepted.
- BaseMethods' actual gate-19 comparison matches 33/37 original MML/parser cases.
  Array row controls, column types and two AMS table cases remain mismatched.
  Some separate BaseItems/ParseUtil tests had guessed flattened shapes; Root
  directly called the same-profile original and found the actual TeXAtom/mstyle/
  mrow wrappers agree with the native results. Their corrected diagnostic tests
  await fresh compilation; required Source wrappers must remain intact.
- TextMacros now uses the real configuration list, inner nodeFactory and shared
  ParseOptions filter payload; its Gate32 37-row frontend comparison passes.
  ConfigMacros and Bold rewriting also pass their full original corpus. AMS
  display/tag behavior and supplemental arrow verification remain open.
- Commit c7dc052 accepts the bounded canonical Symbol-valued Property marker
  bridge: 11 assertions in two executed tests plus a pinned original oracle.
  It does not accept Symbol keys, whole TokenMap reflection or Begingroup.
  Complete Source constructor/prototype/reflection coverage is still missing.
- Root froze the OutputJax/DOM-host responsibility and interface contract plus
  fourteen repeated original lifecycle/filter observations in commit 7f36040.
  A Luna now owns the complete OutputJax implementation, observer, tests and
  member ledger. The bounded OutputJax packet is registered/compiled and its
  original observations pass. Native LiteAdaptor, CommonOutputJax and all common
  wrappers remain required; no new glyph or math-item state is permitted.
- Three persistent Luna owners cover the DOM/common-layout responsibilities above
  while finishing their parser/font packets.
  Root owns architecture, registration, shared builds and independent acceptance.
  Configuration now allows twelve threads, but this active runtime exposes four total
  slots including Root and no tool to raise that injected session limit.
- The remaining enabled packages, common wrappers, completed shared scene,
  SVG renderer and public TeX -> SVG entry remain outstanding. Folia still links
  QuickJS and MathJax assets; native Folia integration waits for actual parity.
  Mermaid remains paused at its documented checkpoint.
The historical scope ledger marks 54/246 files implemented and 192 not started.
That is not a reliable 22% completion claim: several historical implementations
have tagged-tree consumers awaiting restoration, and source-file counts do not
measure coverage or usability. Module counts likewise measure compilation
structure, not completed upstream files. No complete-clean-port claim follows
from the existing build or local test counts.
