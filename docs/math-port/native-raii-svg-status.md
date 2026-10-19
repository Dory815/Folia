# Typed Scene SVG consumer status

The SVG writer imports only the completed Scene, native decimal formatting and
the standard library. It contains no parser, mutable MML, CommonWrapper or Lite
DOM object model. Common layout must supply glyph selection, coordinates,
transforms, sizes and mathematical metadata before freezing Scene.

The current consumer implements ordered traversal, grouping, transforms,
glyphs/local cache, typed paths/rules/text, paint states, clipping, links,
tooltips and XML metadata escaping. Geometry is em-relative and y-up; a single
root reflection maps it to SVG. Arc rotation/sweep remain unchanged locally.
The renderer must provide local cache IDs as serialization options.

Focused native tests passed as part of the first foundation observation.
This does not establish Source SVG parity. Source glyph path/transform spelling,
all wrapper metadata and interaction elements, foreign HTML/annotation output,
and document-global caching still need their exact typed representation and
serialization rules. Unsupported foreign markup, annotations and global cache
requests currently raise `UnsupportedError`; they cannot silently produce a
successful approximation. Full TeX-to-Scene layout is not implemented yet.

Source behavior is pinned to MathJax 4.1.3 commit
`fb987178c2d279a99b0db5ea02e751691703955e`: `ts/output/svg.ts`,
`ts/output/svg/Wrapper.ts`, `ts/output/svg/FontCache.ts` and
`ts/adaptors/lite/Parser.ts`. The existing whole-output corpus and profile are
unchanged. All 150 byte comparisons remain pending for this new architecture.
