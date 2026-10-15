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

The Extpfeil arrow difference is a shared layout issue, not a command mapping
issue. Source and native MML both contain U+21A6. Source label mpadded width
1.663 versus native 1.274 changes the requested stretch width and glyph-size
selection. The responsible Common layout boundary is being diagnosed; do not
replace the TeX mapping or glyph code point to match this formula.

## Acceptance after a successful compile

Capture the unchanged current graph with the tracked runner. Run complete Core
and focused Host/Parser/Foundation tests; compare all existing pinned package
records, maps and callable observations; compare all 75 complete renderer
records and nine public SVG/error records; inspect registration and warnings.
Run the focused canonical binding, identity/mutation/lifetime, moving-child,
texttip and aligned observations. Keep every remaining difference visible.
Only a complete accepted native path can replace Folia's existing math worker.
