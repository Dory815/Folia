# Output resources and transform fidelity

Root's extension to the locked native architecture: full byte-compatible SVG
needs the original font outline spelling and mathematical transform sequence.
The typed outline and its composed affine matrix alone discard those details.
They must not be recovered by recreating an SVG DOM or a JS object model.

## Transform ownership

Scene stores one ordered native transform sequence. Its alternatives are typed
translation, scale, rotation and affine matrix operations. Common placement
produces the sequence once. A composed affine matrix is derived from it when a
consumer needs one; it is not a separately mutable layout result. SVG can spell
the same operations in Source order, and a future D2D consumer composes them.
Source-specific XML punctuation and number formatting remain in SVG.

The initial affine-only foundation is pending this refinement before common
family placement is ported. This is not a second Scene or layout hierarchy.

## Finite SVG font resources

The common immutable font repository owns metrics, resolved glyph selection,
typed outlines, stretch recipes and remap/inheritance data. Scene glyphs retain
immutable typed resources with Source variant cache identity and codepoint.
Normal and bold instances of the same codepoint must not collide in a cache.

`output/svg/font_data.ixx/.cpp` becomes an immutable SVG resource catalog,
module `folia.math.output.svg.font_data`, namespace `output::svg`, replacing
its withdrawn class/runtime projection. `SvgFontResources` maps a typed glyph
key `(variant cache ID, codepoint)` to the original pinned path data spelling.
That string is font data, not an SVG element or executable program. There are
no layout calculations, MML references, callbacks or mutable font loaders.

The development-time font generator emits both the common typed outline data
and this finite backend resource data from the same locked Source glyph records.
The SVG writer receives a const catalog through explicit serialization options;
it uses the preserved glyph path spelling. D2D consumes the common typed outline.
Neither backend repeats glyph selection, stretching or mathematical placement.
Generated sources stay in ignored build directories and compile in bounded
translation units with a font compile pool of two.

The existing typed-path writer tests remain useful as foundation observations;
they do not prove the Source corpus. All source metadata, full glyph records,
extension installation, remaps and exact output remain subject to the original
acceptance profile. Unknown font fields or missing resources must be reported
as incomplete support, not dropped or replaced by unrelated variants.
