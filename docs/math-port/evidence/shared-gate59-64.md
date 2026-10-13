# Real SVG Jax construction and lifecycle: Gates59–64

The pinned MathJax source remains 4.1.3, commit
`fb987178c2d279a99b0db5ea02e751691703955e`. The required Folia input profile and
75-case output corpus have not been narrowed. These checkpoints accept bounded
construction/substrate behavior; they do not accept native TeX-to-SVG output.

| Gate | Actual build exit | Captured inputs | Changed inputs | Main remaining failure |
| --- | ---: | ---: | ---: | --- |
| 59 | 1 | 1,116 | 0 | Font patch types, shared property-binding API, Common qualification |
| 60 | 1 | 1,117 | 2 | Namespace/import/primitive-hint repairs; snapshot is not frozen |
| 61 | 1 | 1,121 | 3 | RenderActions iterable result and late import/test writes; snapshot is not frozen |
| 62 | 1 | 1,120 | 0 | LineBBox canonical resolution used an invalid non-polymorphic cast |
| 63 | 1 | 1,120 | 0 | First real Jax use exposes five missing Common/LayoutJob link symbols |
| 64 | 0 | 1,120 | 0 | All 13 selected targets build |

Root's freeze coordination allowed the Gate60 writes and captured a Source
oracle that had explicitly remained editable in Gate61. Neither checkpoint is
claimed as a frozen acceptance result. Gate62 onward captures the actual
compiler closure and retains every recorded hash through process termination.

The new closure includes real NativeRenderSession, the FontData Source surfaces,
canonical BBox bindings, Common menclose and RenderActions. Source script bases
are corrected to CommonMsub/CommonMsup/CommonMsubsup for munder/mover/munderover.
Font patch operands retain sparse PropertyArray handles. Shared Context
primitive conversion and binary plus are used by reduce/Lite textContent.
The emitted pinned BaseFontData constructor length is 1; its default options
argument is null for omitted and explicit undefined calls.

First real construction exposed declared LayoutJob methods without definitions
and redundant CommonOutputJax lifecycle overrides. Gate64 defines the actual
arena ownership and uses the legitimate inherited AbstractOutputJax methods
where pinned CommonOutputJax has no override. It does not add substitute bodies.

Gate64 Foundation runs PID 38724 from 2026-10-13 11:56:00.718 to 11:56:02.376 UTC,
exits 0, and has executable SHA256
`df4d9c507396c3e1ec4e4fb27ecb1e33af56a17e61d21de001d9d775e243e583`.

| Executed Foundation packet | Assertions | Tests |
| --- | ---: | ---: |
| Context Map | 69 | 10 |
| Actual native service graph / Context-only SVG Jax lifecycle | 25 | 2 |
| Lite projection and module lifetime | 28 | 4 |
| Immutable Scene, precision and serialization order | 16 | 2 |
| Canonical reduce, live textContent and shared plus | 45 | 10 |
| RenderActions isolated Source behavior | 13 | 6 |
| Actual XML DOM/provider ownership | 19 | 2 |
| Total | 215 | 36 |

The lifecycle tests construct the actual SVG output Jax, wrapper factory,
NewCM FontData, style sheet, linebreak service, Lite adaptor and MML factory.
Context-owned callbacks retain real providers without owning the Context.
Public handles retain the actual Context and graph after the session is
released; releasing the last handle releases both. The static constructor
fixture throws on unexpected dynamic font loads. It does not validate the
required dynamic font loader or mathematical rendering.

Root also executes all five freshly linked Gate63 core/diagnostic runners,
without filters, from byte-identical copied executables. This avoids executable
locks while Gate64 compiles. Every actual process exits 0.

| Gate63 runner | Assertions | Tests | Suites | Executable SHA256 |
| --- | ---: | ---: | ---: | --- |
| Full core | 345,620 | 5,320 | 171 | `0ddd690434a751ad75ac83c8d422b254a6df1bee47ecb3cfa873aa8f69ba3132` |
| Host | 458 | 196 | 10 | `d2aeb51f8f0f858b383aed3a9d96558aa98efe07569e0e478e7adcc4160b341c` |
| Parser | 864 | 56 | 7 | `eba2c70d11ca0e524e793d6daa368b2e974b22be5c69e23d5e3bbcaa6b70b973` |
| Regexp | 18,518 | 41 | 3 | `efb8372917f479fc35d08e6686f245e0934aa8f4dde6e18369009cbe3c30319f` |
| LinkedList | 27 | 7 | 1 | `348fe128b31a3b03d6807adccaa861aed31f6af7f1d993792230989e0d3ad12e` |

Full core PID 36520 runs from 11:53:02.840 to 11:56:05.383 UTC. Gate63's overall
build still fails on Foundation linking; the complete core execution is
reported separately from the Gate64 successful build. Gate64's full core has
not been executed.

The five Gate64 fresh parser dumps match all 150 complete Source observations:
Base37, Frontend37, Bold/ConfigMacros16, AMS24 and AMS Boundary36. The fresh
Unicode dump matches 37/37 complete records, including error messages and
RegExp source spelling. Ignored provenance records are
`foundation-64-provenance.json`, `diagnostic-exits-63-root.json`,
`parser-checkpoint-gate64-root/summary.json` and
`unicode-checkpoint-gate64-root/summary.json` under `build/math-port`.

Next acceptance requires the actual Source document/HTML item path, live
wrapper observation at the existing layout-job boundary, complete profile
package and dynamic-font setup, and strict whole-record comparison of all 75
Source cases. The Source corpus contains 73 SVG results and two legitimate Lite
adaptor event-handler errors. Scene must remain independent of Context/job
lifetime and later feed Direct2D directly. Folia integration remains pending.
