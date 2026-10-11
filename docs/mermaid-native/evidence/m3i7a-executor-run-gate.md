# Root bounded complete-run control gate

The delivered highp/lowp start implementations pass this packet's bounded
whole-run gate. This is not full executor or Mermaid/Folia acceptance: the
gradient/radial/image whole-run witness and ownership faults required by the
contract are still pending.

Root reviewed both bodies against original start/control. The facade dispatches
compiled precision only. High/low register banks initialize once before row
iteration and persist across tiles/rows. The existing compiled full/tail lists
are read directly; Return and Mask Stop end one tile. Highp's three original A8
empty bodies end the tile without entering the separate Unsupported resource
service. Each numerical stage enters its existing provider. Resource and
Pattern views borrow the sole compiled context, source/mask and destination;
no context or pixel clone, additional builder, SVG parse or numerical formula
was introduced in these control files.

The independently reproduced original fixture retains SHA-256
`91234a25c7659d4c35df43a1d8387d136d4f811ab34279d6d3bc5b045ecbdabd`.
All 4,608 original runs / 45 compositions compare with zero mismatches, stubs
and error returns. This includes complete destination bytes and persistent
context before/after for full/tail/cross-row, mask/AA/A8 and blend compositions,
plus all 198 original panics and their partial writes. Native baseline log
`build/m3i7a-executor-baseline.log` SHA-256:
`6d47bba9d84cf058a4ad1ee0b34dae63ff3696c1cc02619541a50f72151f23dc`.

All 19 isolated control faults compile and fail assertions: register reset per
tile/per row for both precisions, skipped tails, tail execution through the full
program, ignored Mask Stop, full resource accesses at tails, SourceOver routed
to Plus, row cursor stuck at the first row, overlapping tiles, and highp A8
continuation. The runner byte-checks every working source and fixture unchanged;
none of these faults is accepted because of compile failure or process crash.
`build/mermaid-pipeline-executor-mutations/results.json` SHA-256:
`08828c4fc55e43e06d750a2b40e2ef39e4438381be925d410dd245bb8f8f9767`.

The final locked/offline integration matrix passes all 24 configured integration
targets / 67 tests, including 11 actual Direct2D WARP tests with fixed Arial.
Command: cargo test --locked --offline --manifest-path
third_party/mermaid-rs-renderer/Cargo.toml --no-default-features --features
direct2d --test '*', with FOLIA_SCENE_TEST_FONT=C:/Windows/Fonts/arial.ttf and
CARGO_TARGET_DIR under ignored build/. Log
`build/m3i7a-native-integration-matrix.log` SHA-256:
`dd01aa17d405628d1f18edccb3614ad4fdb4197084a1ce7a1c37aa7dd4cc62ea`.

Root's preceding broad --tests invocation was rejected: it also built upstream
library unit tests whose docs/comparison_sources/*.mmd are excluded from the
vendored package, causing 13 missing-file compile errors. No tests or expected
values were removed to repair that invocation. The corrected matrix covers all
configured project integration targets, not unavailable upstream unit fixtures.
Rejected log `build/m3i7a-final-native-matrix.log` SHA-256:
`3ca2aea8c61aadd7bcd91d3a76e8bb6772ec91bb4c2fbd1eb4ed480b54db6fd2`.

Remaining gates: original whole-run gradient/radial/image compositions and
persistent context mutation/ownership faults; three independently owned blitter
pipelines and AA/memset/new_mask paths; Shader constructors/variant dispatch;
complete shared scene paint/text and final SVG/Direct2D equality. Historical
rendering acceptance stays 34/70. Folia is not connected before those gates pass.
