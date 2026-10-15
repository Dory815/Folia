# Gate157: list crash fixed; full-profile callback decoding still fails

All 55 targets compile; all 2,041 captured build input entries remain
unchanged through evaluation. All evaluation processes have terminated.
The evaluation closes as failed. Twenty comparison commands were not run:
the complete public renderer did not initialize successfully.

Focused checks pass for the existing missing-null-next test, the new
canonical ParserConfiguration/TeX argument roundtrip, and FindTeX tests.
The corrected FindTeX fixtures match pinned Source behavior: construct before
installing normal accessors, keep the default findEnd algorithm for normal
scanning, and replace findEnd only in the two dedicated capture observations.
The setter failure fixture installs only the end accessor, matching Source.
Expected results and production scanner behavior were not changed for these
fixture corrections. Gate156's preceding compile failure was limited to five
unqualified Exception names in the new argument roundtrip test; Gate157
qualifies them as options::Exception.

The actual NativeDocument probe renders x+y, retains the document's real
Context/service graph as required, and confirms its copied Scene remains
readable after release. Complete Core now finishes with exit 1 rather than
an access violation. Core exposes the captured-owner end-sentinel comparison,
an obsolete detached-record expectation, and the fallback receiver check;
Host exposes three obsolete Configuration-list restrictions. These still
require correction and verification. Parser and Foundation also fail;
Regexp and LinkedList pass.

The production 38-package renderer now reaches TextMacros CONFIG and reports
`TextMacros CONFIG arguments are invalid`. Its adapter stores Context Values
in std::any while the native callback expects ParserConfigurationRef and
TeX*. The same issue exists in Textcomp CONFIG. Candidate adapters resolve
both through the existing canonical identity registries; they have not been
built in this checkpoint. API, layout and full public SVG captures exit 1.
There is no complete SVG parity result for Gate157. Gate150 remains the last
recorded 150/150 complete SVG comparison.

Twenty of the 25 package capture commands succeed. The five failures are
parser-fields, tex-next-six, tex-bboldx-font-packages, parser-projection and
texttip. Successful capture is not a comparison result. Build/evaluation
hashes, process exits, focused checks and full SVG binary audit remain in
the ignored build directory. The failed evaluation summary explicitly records
the complete comparison as unavailable and comparison commands as not run.
No executable or directory copies were created. Folia still uses QuickJS for
math; Mermaid remains paused.
