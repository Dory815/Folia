# P21b frozen generic Unicode escape grammar experiment

This is an ignored successor of P20d, not an adopted production regexp engine.
Pinned regress 0.12.0 upstream commit:
771e6c8fa1e472ef86c45b5206b37fa4e4494e25. Immediate predecessor parse.rs SHA256:
3c2a911d8b65389d1a657f371784777393b2d374f7b4de4b0c3c5f2f31f2967e.
Root owns copy preparation, all host/boundary files, actual Node oracles,
unchanged previous corpora, replay tooling, patches, test/review and acceptance.
Luna tex_parser owns ONLY the new ignored copy
build/math-port-regexp-unicode-escape-source/src/parse.rs and
docs/math-port/evidence/p21b-unicode-escape-implementation.md.
No builds, tests, other files, resets, agents, commits or integration by Luna.

Scope: strict Unicode-escape grammar in try_escape_unicode_sequence and its
three callers. Freeze two separate private bool arguments: allow_braced and
combine_surrogate_pairs. Ordinary character escape passes Unicode mode for
both; named capture identifier start/continuation pass true for both, since
their Unicode escape grammar applies independently of regexp flags.
Braced code-point escapes require allow_braced. Legacy ordinary rejection
restores the cursor to after the consumed u, allowing the existing identity
escape and enclosing quantifier/class grammar. All collected digits in the
braced form, primary Hex4 and speculative low-surrogate Hex4 must be ASCII
HexDigit, excluding signs/space/prefixes. Preserve empty/overflow/range behavior,
P20d pre-prefix rollback, valid pair decoding and all existing failure paths.
No pattern spelling, concrete code-point or case-result special handling.
Public API, flags adapter, indexing, matching, Unicode/case tables and all
other parser functions remain outside this packet.

Root generated independent actual Node 24.13.0 / Unicode 16 / ICU 77.1
observations before dispatch: **31,488 cases**, including 27,648 ordinary
braced-mode/HexDigit, 3,456 Hex4/speculative-pair and 384 mode-independent
named-identifier cases. Match/capture/named ranges, constructor outcome and
numeric lastIndex are observed without candidate input. Source corpus SHA256:
cf090d64ed483a82a37740784a3c97b388c1a7c75335d9a6da8e82deced67bc6.
Data stays in build/math-port-regexp-unicode-escape/node-reference.json;
the summary and generator are root-owned.

Root replays all four corpora (119,218 original, 111,818 casefold,
4,032 lookahead and the new 31,488) against the P20d predecessor, retaining
both diagnostic and correct v-implies-Unicode modes. No regression is allowed
in the correct adapter. Run the complete upstream utf16 suite with locked
offline dependencies; compile failures, crashes and surviving differences
cannot count as equivalence. Whole-engine/native-host/TokenMap/TeX/Common/SVG
acceptance and Folia integration remain open even when this repair passes.
