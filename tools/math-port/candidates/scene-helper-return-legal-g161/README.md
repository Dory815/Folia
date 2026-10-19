# Legal TexHTML helper-return fixture (G161)

This candidate updates the existing `scene_attribute_parity_g145` test after
the G160 helper-return fixture correction. It keeps the test on the real
`MathItem::typeset` and SVG wrapper path, but places the XML-backed `HtmlNode`
inside an `mtext` token before typesetting. MathJax 4.1.3's pinned
`TexHtmlConfiguration.ts` constructs precisely this `mtext` wrapper around the
`HtmlNode`; the token node's `setTeXclass` does not recurse into that XML node.
The layout row therefore never calls `updateTeXclass` on the read-only
`AbstractMmlEmptyNode.prevLevel` field.

The test still intercepts the actual Source `svg` helper during typesetting,
returns the first `foreignObject`, creates a detached decoy, and appends the
geometry probe under the returned node. It then verifies the imported HTML
identity and live rect geometry as before.

The pinned CJS witness in
[`scene-helper-return-legal-source-witness-g161.cjs`](scene-helper-return-legal-source-witness-g161.cjs)
checks hashes for the TeXHTML constructor, MML token/XML node behavior, SVG
helper, and Lite adaptor. It compiles and typesets actual TeXHTML through
`MathDocument.convert`, then checks the `math -> inferredMrow -> mtext -> html`
tree, real `foreignObject` helper return, decoy separation, and child-parent
ownership. The additional Lite parent sequence exposes two Source details the
old fixture overclaimed: the helper's `foreignObject` construction sets the
`mjx-html` child parent, while `LiteAdaptor.clone` does not restore the parent
field of cloned comments. The C++ fixture expectations are adjusted to match
those pinned operations; no production setter or source behavior is changed.

## Candidate files

- [`scene-helper-return-legal-g161.patch`](scene-helper-return-legal-g161.patch):
  one test-only unified patch against the current tree after the G160 fixture
  correction has been integrated.
- [`scene_attribute_parity_g145.cpp`](scene_attribute_parity_g145.cpp):
  candidate test file for review.
- [`scene-helper-return-legal-source-witness-g161.cjs`](scene-helper-return-legal-source-witness-g161.cjs):
  executable pinned Source witness.

## Checks

The Source witness passed with Node 24.13.0. Run it with:

```powershell
node tools/math-port/candidates/scene-helper-return-legal-g161/scene-helper-return-legal-source-witness-g161.cjs
```

`git apply --check` is run against the current worktree before Root integrates
the patch. No C++ build or UT run is claimed here; the shared G162 build owns
the frozen IFC outputs. Once Root's build finishes, run the focused test via
the existing Foundation test executable using its positional test filter,
then run the containing math test target if the focused test passes.
