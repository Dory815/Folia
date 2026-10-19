# Font outline decoding and Source path errors

The pinned font data contains four nonempty malformed path fragments. Do not
split concatenated positive numbers to guess intended coordinates, repair the
upstream data, omit the glyph, or abandon the whole immutable font profile.
SVG resources retain the exact Source path bytes, including these four rows.

The common font decoder follows the SVG path grammar and its
[error handling rules](https://www.w3.org/TR/SVG2/paths.html#PathDataErrorHandling):
keep complete segments preceding the first invalid segment, including complete
segments within a repeated command. Do not append the Source trailing close
command after a parse error. Missing coordinates are not zero. This preserves
the geometry represented by the Source SVG rather than inventing an outline.

`output/common/font_path` owns the decoder, not SVG serialization. Add a typed
`FontOutlineDecodeResult` holding an owned GlyphOutline and an optional
`FontOutlineError { byte_offset, command, reason }`; reason is a closed enum
(missing command, unsupported command, invalid number, incomplete segment).
Invalid units-per-em remains an invalid_argument. Only lexical/grammar errors
become this explicit result; allocation exceptions must unwind normally.
`decodeFontOutline` may remain the strict convenience service; generated font
ingestion calls the result-returning service and moves its outline into the
immutable GlyphResource. FontGlyph stores the optional typed diagnostic as
profile data. No render request or global mutable error registry is retained.

Normalize glyph coordinates to em in y-up axes once. For relative commands,
perform arithmetic in font units before normalization. Closepath restores the
current point to the start of its subpath. Leading plus signs and repeated
segments follow the grammar. The supported command vocabulary is explicit;
the pinned fragments contain M/L/C, and Q is a declared decoder service.
Unsupported future commands must produce a diagnostic, never be skipped.

Luna owns a draft-only packet covering font_path, FontGlyph's diagnostic field,
ingestion generator call sites, native dump diagnostics, Source comparator and
focused UTs. Root promotes after the current build is finished. All outlines
and diagnostics must be compared against independently normalized Source data;
check all four actual malformed rows and ordinary empty/absent paths, relative
coordinates, plus signs, subpaths, repeated commands and partial final segments.
Do not add codepoint-specific branches. The SVG catalog and writer preserve
their exact serialization contract and perform no parsing or placement.
