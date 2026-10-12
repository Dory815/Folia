# P54 Unicode NFD host freeze

This portable host implements the Source `String.normalize('NFD')` operation
needed by ParseUtil and French mathematical styles. The pinned original Node
reports Unicode 16.0 and ICU 77.1. The restored document-test ICU version is
not substituted for this host. Exact-version generated data avoids platform
APIs and a second normalization runtime.

Data comes from UnicodeData.txt for Unicode 16.0.0; its SHA256 and official URL
are in p54-unicode-data.json. The generator verifies the hash, parses all UCD
fields and excludes compatibility decompositions. It emits 2081 canonical
decompositions, 3127 decomposition values and 934 nonzero combining classes.
The Unicode license is retained alongside the generated module.

The official corpus is https://www.unicode.org/Public/16.0.0/ucd/NormalizationTest.txt.
Node/ICU versions in the manifest are observed metadata; the oracle enforces
Unicode 16.0, which fixes the relevant normalization data version.

Root froze the data, public interface, original Node oracle and generated
tests before implementation. The compiled stub throws Unimplemented.
`build/p54-nfd-stub-red.log` records successful compilation and failing NFD
assertions, exit 1. This establishes that the tests reach the missing body.

Required body, exclusively in util/unicode_normalization.cpp:

- Decode valid UTF-16 pairs; preserve isolated surrogate units and embedded NUL.
- Recursively expand canonical decompositions and algorithmic Hangul syllables.
- Order non-starters stably by canonical combining class without crossing a
  class-zero starter; preserve equal-class order.
- Encode exact UTF-16; retain compatibility characters and all unmapped scalars.
- Use the frozen data module. No ICU, Windows, JS engine or case-specific path.

The original oracle verifies all 19965 official NormalizationTest rows against
Node, giving 99825 direct native comparison inputs. Another 85 exact sequence
cases cover malformed UTF-16, NUL, ordering boundaries and deterministic mixed
strings. Seventeen full-plane FNV64 observations cover every 1114112 codepoint;
these are fingerprints, not direct per-scalar output comparisons.
Both generators support --check without changing their frozen output.

Root owns gold, declarations, data, build and acceptance. Luna may implement
only the native body. Completion requires baseline comparisons, independent
review, meaningful compiling semantic faults and full core verification.
This host does not accept French math-style logic or complete TeX rendering.
