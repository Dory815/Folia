# Mermaid C++ continuity

## Current checkpoint — 2026-10-21

- Active stage: real font measurement and Pie SVG. First batch adds a real
  browser probe (60 cases) and an explicit-error DirectWrite adapter/probe.
  Local Edge probe passed outside the sandbox after sandbox exit 0xc0000005.
  Native results are pending CI; advance is NOT yet accepted as SVG bounds.
  Next action: inspect same-host native/browser evidence, settle bounds, then
  implement Pie SVG. No live local process or agent. Candidate files are tracked
  under tools/mermaid-cpp and src/platform/text/mermaid_measurer.ixx.

- User scope: replace Folia's Mermaid dependency with a C++ port based on the
  original official Mermaid JS/TS. Do not port the Rust rewrite. Website and
  release tooling are excluded; this is unrelated to the MathJax migration.
- Upstream base: `water2004/Folia` master `72e2d7c7`. Fork: `zhao0349/Folia`.
  Current branch: `codex/mermaid-cpp-measurement`, based on the merged fork
  master. Completed branches `codex/mermaid-cpp-official` and
  `codex/mermaid-cpp-pie` are preserved. Remotes: origin=fork, upstream=author.
- Source pin: Mermaid 12.2.0, commit
  `1fb7869866dd619168d840291c9c54b4f129758b`, ignored checkout at
  `.reference/mermaid-js`. `source-lock.json` records file hashes.
- Text scanners `086e9b44` with fixes `d67f05f3` and `b851abd8` pass Windows
  Debug/Release CI `39543613360` (862 source observations).
- Number formatting `b3adc989` passes Windows Debug/Release CI `39543875561`
  with 9,695 actual JavaScript binary64 observations. All are pushed.
- Packet parser/layout `60691de3`, SVG `157e380b`, and complete official CSS
  fixture `535e5ab5` are pushed. Their Node oracles pass: 254 parser/layout
  cases, 55 SVG cases. Native CI `39544210209` passed Debug and Release.
  Earlier run `39544084249` failed on the missing CSS fixture header, now fixed.
- Fork stage PR: https://github.com/zhao0349/Folia/pull/1, head `535e5ab5`.
  PR-event CI `39544508181` passed. PR merged with preserved small commits at
  `396d814f2d6e03b3767560cab97f8cec794b3a0c`. Upstream has no PR yet.
- Shared lexer extraction is the first Pie-stage batch: common metadata,
  hidden tokens and string conversion move out of Packet without changing its
  public API. CMake discovers the new module; existing Packet fixtures are the
  required regression check. Commit `8ece2acf` passed Debug/Release regression
  CI `39544334238`.
- Pie raw parser/DB `09ad39f6`, D3 geometry `733a3295` and parser tests
  `3675ec45` are pushed; native CI `39544894952` passed Debug/Release.
- Geometry has 86 official-runtime cases / 618 sectors, exact paths and
  1e-12 centroid tolerance for platform libm. Parser oracle has 291 cases.
- Pie layout `337e41d4` adds explicit measurement inputs and 40 official
  renderer observations. Push CI `39545148134` and PR CI `39545380450` both
  passed Debug/Release, including Packet regression tests. This tests
  arithmetic only; real text measurement, final themed SVG output and host
  integration remain pending.
- Second fork PR https://github.com/zhao0349/Folia/pull/2 merged at
  `df9753da2a46aae851724397fce7acdbb98e7548`, preserving five small commits.
- All three prior agents have delivered their assigned files and are idle.
  No background local build is running. All active verification is in Actions.
- Next concrete action: implement the real text-measurement boundary and Pie
  SVG output on the measurement branch. First establish a browser/native
  measurement comparison using exact resolved fonts and UTF-16 display text.
  Do not claim a completed Pie renderer or application replacement yet.

## Next-stage measurement evidence

- Pie's official renderer measures both every legend and the title using
  `getBoundingClientRect().width`; title width can make viewBox X negative.
- `src/platform/text/dwrite_measurer.ixx` currently returns end hit-test
  advance and estimates with character counts on failures. Do not reuse that
  fallback for Mermaid. Keep Windows/DirectWrite code in the platform layer
  and return an explicit error if measurement fails.
- `pie_legend_text()` supplies the exact strings to measure after first-label
  deduplication, including showData suffixes. `PieMeasurements` requires one
  width per populated section and a title width, in SVG user units.
- Local browser executables verified on 2026-10-21: Edge at
  `C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe` (157.0.4341.10),
  Chrome at `C:/Program Files/Google/Chrome/Application/chrome.exe`
  (155.0.8059.81). No browser measurement probe has been run yet. Use an
  isolated ignored test profile; do not touch the user's browser profile.
- Existing oracle packages include a browser-loadable Mermaid distribution
  at `tools/mermaid-cpp/oracle/node_modules/mermaid/dist/mermaid.esm.mjs`.
- There are no uncommitted implementation candidates or live local build
  processes at this handoff. Prior agents have completed their assignments.

## User's incremental persistence instruction — 2026-10-21

Persist every completed small part as a commit pushed to the user's fork.
For each completed stage, open a PR into `zhao0349/Folia` and merge when
verification and review support it. Preserve the small commits with a merge
commit. Only after the entire dependency replacement is complete should a PR
be submitted to `water2004/Folia`. The user explicitly authorized this workflow
to avoid losing work when usage quota interrupts a session.

## Acceptance and remaining scope

No upstream PR has been created. The application still uses the Rust Mermaid
dependency; current C++ work is foundational and must not be described as a
completed replacement. The user has authorized implementation, GitHub Actions,
fork/branch pushes and a final upstream PR; no repeated approval is needed.

Folia's current caller has no diagram-family whitelist. The existing dependency
inventory is 23 families (listed in README). Packet is the first native family,
not an agreed reduction of scope. Shared configuration/preprocessing/themes,
the other families, text measurement, app integration and full validation remain.

Keep the original family layout algorithms. Do not substitute a generic layout,
character-count text measurement, an embedded JS runtime, or a Rust fallback and
claim the requested C++ replacement is complete. Ordinary SVG normalization is
a separate Rust dependency and does not disappear just because Mermaid moves.

## Build environment

Use PowerShell 7. Local MSVC/CMake/Ninja are unavailable; the fork's
`.github/workflows/mermaid-cpp.yml` builds the isolated module with VS, CMake
4.3.5, Ninja, the forked Boost.UT module and the console runtime. CI uploads
test executables. Set `FOLIA_MERMAID_FIXTURES` to the local absolute fixture
directory before running a downloaded executable.

Local Node and pnpm are in the Codex runtime dependency directory. Node 24 can
import the source-text oracle's TypeScript directly. Packet tools use the exact
packages in `tools/mermaid-cpp/oracle/pnpm-lock.yaml`. Only node_modules and
build outputs are ignored; fixtures and source provenance are committed.

UT expression formatting cannot stream UTF-16 characters. Wrap UTF-16 string
comparisons in `bool(...)`, or compare their hexadecimal snapshots. A source
oracle passing is distinct from native C++ tests passing.

## Verified boundaries

- Detection comments and preprocessing comments intentionally differ. Do not
  replace the current released TS scanners with legacy regex examples.
- Raw Packet grammar differs from full MermaidAPI preprocessing, entity
  encoding and DOM sanitization. Keep these validation stages distinct.
- Official Packet silently truncates fields beyond 10,000 rows and permits
  parsed Infinity; native rejects these cases with explicit errors.
- Packet default renderer always emits a title text node, even when empty.
  Negative coordinates in an empty diagram can be legitimate.
- Number formatting must use JavaScript shortest significant digits; exact
  fixed integer formatting differs for values above the safe integer range.
