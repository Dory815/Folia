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
