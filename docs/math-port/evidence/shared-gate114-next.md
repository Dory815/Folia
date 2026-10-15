# Gate114 preparation and acceptance requirements

Root prepares one coherent shared build after the new canonical binding APIs
stabilize. Gate113 remains the latest successful compile and its recorded
runtime results remain unchanged. Nothing in this preparation accepts Folia
integration or the complete renderer.

## Changes awaiting the shared build

- Generic OrderedMapAccess and OrderedMapStorage share one owner with live
  Context Map views. Token maps and SubHandlers accept a context-backed typed
  view over the replacement's actual store, without copying entries.
- Canonical Token/TokenMap/Macro, Handler/PrioritizedList/FunctionList, and
  Attributes bindings must join the actual ParserRuntime graph. NodeFactory's
  absent native field declaration uses one descriptor-resolution/write path.
- Source raw delimiters reach fenced construction without converting Missing
  into the text `undefined`. Base EqnArray applies its raw spacing default.
- Scene append validates through a read-only Builder operation before the one
  actual child-array write. Moving a parented child uses the real remove path.
  Wrapper.node, MML attributes and the aligned transform slot await runtime
  verification in the same graph.
- Colortbl, Mhchem, TagFormat and SetOptions are source registration candidates;
  Gate113's actual registration is still 23/38. TagFormat's default callbacks
  and emitted prototype order receive an additional Source review.

The new Attributes observation executable and TokenMap binding tests are
enrolled alongside the existing complete Core, Host, Parser, Foundation,
package, full-renderer and public-interface targets. The byte-safe capture
runner preserves raw stdout/stderr and build/executable/input provenance.

## Findings that change the next action

The static module/Scene-boundary audit at 2026-10-15 09:28 UTC inspected 363
interfaces and found no missing or duplicate modules, import cycles, external
partition imports, or Scene-to-SVG imports. This is a structural check, not a
compile or behavior pass. Its report is
`build/math-port/scene-module-audit-pre-g114.json`.

An independent Bussproofs review initially inferred that empty proof content
should throw. A direct pinned Source rerun and complete MML inspection
disproved that inference: empty and whitespace-only AxiomC and UnaryInfC
succeed with empty mrows, matching native behavior. Both implementations
initialize the inner Stack eagerly. Remaining own-field reflection and
additional behavior observations need separate evidence; the original 31-case
Source output is unchanged.

The initial Extpfeil layout diagnosis was corrected after inspecting the
complete MML attributes. Both contain U+21A6, but Source selects Mathtools'
`xmapsto` parameters (10,10), while Gate113 native selects Extpfeil's (6,7).
Mathtools was not registered in that native session. The resulting widths
`+1.111em` and `+0.722em` explain mpadded widths 1.663 and 1.274 and the later
glyph-size selection. No CommonMpadded algorithm change is supported by this
evidence. Verify the complete package configuration before changing layout.

## Actual Gate114 result

The build terminated with exit code 1 after selecting 50 targets and capturing
1,932 inputs; the final audit found zero changed inputs. Its session is
terminal. No runtime acceptance is claimed and no stale executable is used.
Log: `build/math-font-package-gate-114.log`.

The failures identify integration work: Property-key/string overload ambiguity,
generic Map access/range adaptation, new binding code using an obsolete return
type or taking an address of a temporary, native observer storage inspection,
and several draft/test expression types. Owners are correcting their modules;
one Luna owns the consistent observer-storage adaptation across packages.

Root also replaced the stack-item registrar's class-name condition for static
error tables with explicit `ownsErrors` metadata. The three BaseItems classes
and Physics AutoOpen declare that metadata; other classes inherit their parent
table. A focused test checks own fields, inherited identity, prototype links,
and AutoOpen's Source stop-error tuple. These source changes compiled in
Gate114, but the tests and Physics 63-case comparison still require a complete
successful build and run.

## Acceptance after a successful compile

Capture the unchanged current graph with the tracked runner. Run complete Core
and focused Host/Parser/Foundation tests; compare all existing pinned package
records, maps and callable observations; compare all 75 complete renderer
records and nine public SVG/error records; inspect registration and warnings.
Run the focused canonical binding, identity/mutation/lifetime, moving-child,
texttip and aligned observations. Keep every remaining difference visible.
Only a complete accepted native path can replace Folia's existing math worker.

## Gates115–116 and runtime findings

Gate115 selected 51 targets with 1,935 unchanged captured inputs and failed on
one remaining observer that treated a replaceable configuration-list owner as
a direct reference. Root changed that observer to retain the public shared
owner during iteration. Gate116 then built all 51 targets successfully with
the same 1,935 inputs unchanged.

Gate116 runtime acceptance did not pass. Regexp and LinkedList tests passed;
Core completed with one failure in the new ordered-map owner-release test.
The test tracked the first entry while deleting a different entry; its focused
correction now tracks the actual erased owner. Host encountered a fixture
callback that captured a hooks object after it had been moved. Parser tests
reported exceptions. Foundation, public SVG and most package runners exited
with Windows code 0xC0000005. The three new standalone binding constructors
used `ownedExecutionLease_` before its declaration-order initialization;
their member order is now corrected, awaiting a new coherent build.

The Attributes runner exited zero, but its observation JSON lacked a closing
record brace and stringified a numeric partial-write result. Those probe
serialization issues are corrected without changing Source expectations.

NodeFactory/ParseOptions projection comparison did pass against both complete
pinned Source records, including the native owner checks. Report:
`build/math-port/parser-projection-comparison-g116.json`. This bounded success
does not accept the renderer or the remaining runtime checks.

All Gate116 Root capture handles are terminal: Core session 74051, SVG/API
runners, and package session 99121. Their executable hashes, PIDs, timestamps,
raw output and actual exit codes remain under `build/math-port`.

## Actual Gate117 result

All 51 selected targets compiled successfully; the final audit found zero
changed inputs among 1,935 captured files. All runtime captures are terminal.
Complete Core, Host, Regexp and LinkedList pass. Parser still reports ten
exceptions. The declaration-order lease fix restored native document, SVG,
and package execution without the previous immediate process exits.

The strict full-renderer comparison matches 56/75 complete records, with
73 successful layouts and two errors. Actual registration is 27/38, with
11 warnings; neither configured package names nor successful layout alone
constitute acceptance. The public SVG/error API matches all nine records,
including the formerly different aligned SVG.

Current complete package comparisons pass Mathtools 42 cases / five maps,
Physics 63 / eleven, Colortbl 29 / one, original Bussproofs 31 / two, and
TexHtml/DOMStrings/XML 6+2+4. Attributes, both parser projection records,
MapHandler module sessions, Array.unshift and getBBox witnesses also pass.
Expanded Bussproofs matches 17/21: four empty-content records expose an actual
nested-child emptiness check missing from native padding; its metadata also
contains a Source hash transcription error. The original corpus did not
observe these cases or the unbound actual innerStack field.

Two package observers terminate while reflecting unregistered native
ParseMethod callable identities. Texttip reaches the shared String.trim call
and fails there. Tag/SetOptions matches 11/23: the earlier twelve cases see a
SetOptions map loaded too early by the render-session module bootstrap.
The separate default-methods executable exits zero but writes a file/status
line rather than JSON stdout, so its standard capture is invalid and no
current acceptance is claimed for that witness.

Foundation identifies three test setup issues: two Attributes assertions
expect global red despite defaults blue; one Scene fixture omits its canonical
property/Lite bindings; the XML lifetime fixture keeps a separate Context
owner alive beyond the intended scope. Owners are correcting those fixtures
against independently observed Source semantics, with production unchanged
for the Attributes and XML corrections.

Evidence includes `common-layout-comparison-g117.json`,
`math-api-comparison-g117.json`, `parser-projection-comparison-g117.json`,
`host-layout-comparison-g117.json` and all raw capture provenance under
`build/math-port`. Mhchem's capture exists, but its strict provenance comparer
was invoked after source edits resumed and correctly rejected changed build
inputs; do not claim a fresh Gate117 Mhchem acceptance from that invocation.
For the next gate, run every provenance comparer before releasing the freeze.
Folia integration remains pending full acceptance.
