# P21c root data identity freeze — Unicode 16 candidate experiment

Production engine selection remains open. This successor keeps all P21b parser
and legacy canonicalization changes, but replaces generated Unicode 17 tables
with the Unicode 16 data used by pinned Node 24.13.0 / ICU 77.1. No pattern-name
filter or individual script/code-point exclusion is permitted.

Primary data is Unicode's final 16.0.0 UCD and Emoji 16.0, downloaded into
ignored build/math-port-unicode16. Pinned archive/input SHA256:

| Input URL | SHA256 |
|---|---|
| https://www.unicode.org/Public/zipped/16.0.0/UCD.zip | c86dd81f2b14a43b0cc064aa5f89aa7241386801e35c59c7984e579832634eb2 |
| https://www.unicode.org/Public/emoji/16.0/emoji-sequences.txt | 3fe3c77e72e8f26df302dc7d99b106c5d08fd808ef7246fb5d4502d659fe659c |
| https://www.unicode.org/Public/emoji/16.0/emoji-zwj-sequences.txt | 9423ec235474356f970a696506737e2d5d65453a67f45df66b8bbe920c3fab83 |

ReadMe.txt confirms the UCD version; emoji files identify version 16.0.
These data remain under Unicode's terms/license, not the candidate's license.
Final production adoption must retain the required data license separately.

Root owns preparation, original generator and data hash verification, path /
output / formatter adaptation, generation, all expected data and replay. No
agent is delegated a production engine/data decision. New ignored Source copy:
build/math-port-regexp-unicode16-source. Existing Source and all predecessor
copies must remain unchanged. Original gen-unicode algorithms remain exact;
adapt only the input path, output destinations and explicit standalone
rustfmt invocation. Generated upstream tests go to ignored evidence and cannot
replace actual-Node expectations or the unchanged upstream suite. Never run
a workspace-wide formatting command against the candidate tree.

Replay all four independently pinned corpora against P21b, record every fixed
and regressed case and retain remaining differences. Generate additional
property/case witnesses if old stimuli do not cover the data change. An output
table hash or reduction in failures cannot alone accept Unicode semantics,
the engine, the host protocol, TokenMap, complete TeX/Common/SVG or Folia.
