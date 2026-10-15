# Gate158: runtime regressions closed; fresh complete SVG parity restored

All 55 targets compile. All 2,042 captured build input entries remain
unchanged through evaluation. Complete Core, Host, Parser, Regexp,
LinkedList and Foundation suites pass. The real NativeDocument probe passes,
including released-owner checks and independent lifetime of the copied Scene.
All evaluation processes terminate before the captured sources are released.

The complete public Renderer is freshly captured after the production
changes. The strict clean-runtime comparator matches 150/150: 75 cases in a
continuous renderer and 75 in separate renderer-owned runtimes. It checks
full SVG bytes, errors, Scene presence, actual runtime boundaries, input
inventory, pinned Source dependency hashes and native capture/build evidence.
The Source capture is the unchanged, clean-process Gate150 capture; the native
Gate158 capture and executable hashes are new. API comparison is 9/9 and
layout observations match 75/75 with all 38 actual registrations. The layout
profile includes two Source error observations; matching them is part of
the result, not a claim that every case produces SVG.

Production corrections since the failed list-owner integration include the
null native record check, shared canonical argument encoding using existing
registries, explicit Bboldx/TextMacros/Textcomp CONFIG argument decoding, and
end-sentinel comparison against an iterator's captured owner after array
replacement. There is no fallback to a prior native execution model.

Fixture corrections keep pinned Source expectations intact: FindTeX normal
scanning forwards the real findEnd algorithm, with accessor installation and
special capture overrides matching the Source witnesses; native fallback
receiver identity is checked separately from the four original Source
observations; deleting a seed list record leaves its detached fields intact;
raw boolean priority and valid read-only array length are accepted as Source
requires, with growth subsequently rejected. These are included in the
passing complete Core/Host suites.

All 25 package capture commands succeed. Seventeen of the twenty comparison
commands pass. The three remaining failures are texhtml-xml, tex-next-six and
tex-font-packages. Font observations remain 13/16 and next-six 18/20; the first
difference is now `jax.findTeX.end.$$[2]` own keys: Source exposes lastIndex
while the native observation exposes none. The responsible RegExp/observer
boundary is under investigation. Two standalone XML namespace differences
remain. Complete texttip SVG comparison passes.

Ignored build evidence includes gate-158 input audits, diagnostic exits,
package-captures-g158, checkpoint-comparisons-g158, the strict public SVG
comparison and native boundary annotation, API comparison, and unchanged
binary audits. The shared evaluation remains exit 1 because of the three
comparison failures; it does not signify complete renderer acceptance.
No executable or directory copies are created. Folia remains on math QuickJS;
Mermaid remains paused.
