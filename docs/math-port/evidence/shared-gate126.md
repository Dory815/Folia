# Gates124–127 actual results and next work

Gate124 ended with a truncated HTML UT: the draft patch's new-file hunk count
omitted the final three closing statements. Gate125 restores them and builds
all 51 targets with 1,948 unchanged captured inputs. It renders 67/75 complete
records, public API 9/9, actual packages 36/38. Its complete Core run fails one
new Centernot assertion: Configuration normalizes a postprocessor to
`[filterCenterOver, 5]`, while the UT expected a bare function. This was verified
against pinned CJS before correcting the test. Host, Parser, Foundation,
Regexp and LinkedList pass.

Gate126 builds all 51 targets with all 1,948 inputs unchanged. Complete Core,
Host, Parser, Foundation, Regexp and LinkedList pass. All runtime handles are
terminal, and the complete comparison runner audits inputs before and after.
Public API remains 9/9; complete rendering is 69/75 with 36/38 packages and
two warnings. Most previously passing package/identity comparisons stay passing;
the independent XML comparer has two foreignObject namespace failures. The
initial checkpoint summary overlooked its nonzero exit; Gate129 rechecked the
saved Gate126 stdout and corrected this report.
Next-six remains 18/20 and fonts 13/16, exposing the shared missing MML fields.

The factory-repeat binding correction retains the original canonical identity,
constructor/prototype and mutable jax owner. CommonOutputJax now projects its
actual factory as a live Context field. Scene creation preserves Source rect
geometry attributes and inferred-row group transform at the original paint
observation point, alongside typed geometry/transforms. Removing the extra XML
foreignObject namespace matches the full profile's creation attributes, but
breaks two standalone XML records; namespace serialization remains unresolved.
Complete action
toggle and AMSCD records now match. No Lite event-handler substitute was added.

Texttip reaches completion in Gate126, including the original strict Context
factory identity assertions and typed Scene geometry checks. Its byte comparer
correctly rejects the missing profile option: native `overflow="overflow"`
versus Source `overflow="linebreak"`. Gate127 adds the pinned exFactor,
displayOverflow and inline linebreak options to that witness; all 51 targets
build with the same 1,948 inputs unchanged, and a fresh provenance-checked
texttip capture matches all 1,315 Source SVG characters. Gate127 runs this
focused comparison only, without relabeling Gate126's complete results.

The remaining six renderer records are math tooltip, Bussproofs, two Empheq
and two TexHtml cases. TexHtml's enabled SVG now differs at font-family
(`undefined` versus `Times`); disabled mode has four additional calls. They
remain failures. The independent twelve TexHtml/XML records also include the
two namespace failures described above.
Empheq's actual export/receiver packet, MML raw fields and captured factory
creation packets, and the Stack array-owner packet are unregistered drafts.
Root must review and build them before any acceptance claim. The lifetime
review also identifies unreleased per-conversion Context/MML/wrapper/DOM
owners; resetting the whole renderer would lose persistent Source state and
is not an accepted solution. Folia and Mermaid are unchanged.
