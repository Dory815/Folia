# Stopped native Mermaid migration checkpoint — 2026-10-26

The user requested stopping implementation and saving all current work in a new PR targeting the source branch of upstream PR #1.
This commit preserves source and tests; it does **not** claim full Mermaid SVG acceptance.

The PR branch had 22 newer commits beyond the local starting snapshot. The checkpoint
keeps that branch history and its active source intact. `canonical.patch.gz` captures all
1,772 staged local source/test/document changes against the reconstructed local snapshot
of original contribution `011e0656`. It includes the persistent original SVG regression
fixtures, numerical source/vendor records, native diagram modules and test tools.

`luna01.patch` through `luna12.patch` preserve every differing private source, platform
and test file relative to the stopped canonical tree. These are independent candidates;
they must not all be blindly applied in order. Shared entry/platform edits can conflict.
The manifest records both sides' SHA-256 values and all saved comparison summaries.

To recover, reconstruct the local starting source at `011e0656`, decompress `canonical.patch.gz` and apply the patch
with Git's three-way conflict handling, then review the required worker patch. When
continuing directly on the PR head, merge the checkpoint against its 22 subsequent
commits instead of replacing the branch tree. Build products, package restores,
browser profiles and duplicated generated corpora are intentionally excluded.

Latest canonical verification: native build passed; core CTest failed; platform and
1,024-observation native-host regression passed. Later worker fixes are not a substitute
for another integrated run. The historical complete canonical byte gate remains
798 exact SVGs out of 8,812 originals, with 8,014 failures and 1,386 original errors.
Full acceptance remains false.

The private Railroad-only report records **212/212 exact original SVGs**, zero failures,
DLL SHA-256 `d5d708373d65a5cfadf301f2a2dd9460ff052398b88f8d697effca4b9af42693`.
Its implementation and shared integration are saved in `luna10.patch`; they had not
been integrated and independently accepted by root when the user stopped work.
Other families still have unsupported stages or byte mismatches.

The canonical archive decompresses to 45,641,089 bytes; the manifest records both compressed and original SHA-256 checksums.
