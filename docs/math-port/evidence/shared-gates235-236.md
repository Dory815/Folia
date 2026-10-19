# Shared gates 235–236: standalone observers and full SVG comparison

Both 61-target builds completed with exit 0 and at most four compile jobs.
Gate 235 froze 2,196 inputs; gate 236 froze 2,197. All inputs were unchanged
through their corresponding evaluations, and all native processes terminated.

Standalone six-package and next-seven observers now construct the actual
Context-bound AbstractMathItem and a shared MmlFactory per TeX instance, matching
their pinned Source fixtures. The previous raw factory pointer skipped the
existing canonical identity binder. No Source expectations or comparison rules
were changed.

Gate 235 full SVG result: **150/150 exact matches**, covering 75 fixed inputs in
continuous and fresh-session modes. The native process exited 0 with 150 SVG
records and no runtime errors. Ten records contain expected MML error nodes;
their SVG bytes also match Source. The four toggle/tooltip event registration
and structural position comparisons pass.

- MathJax: 4.1.3, commit `fb987178c2d279a99b0db5ea02e751691703955e`.
- Source SVG capture SHA-256:
  `47ee5698ebcdaaf919cb7efe3c3aa6a87d0f19b51ff50d86361710e9f0f1224d`.
- Native SVG capture SHA-256:
  `5a256796b664f01bea52fac12e9a68ba470e0b93566ac4f9b7ac6fdc0061138c`.
- Native executable SHA-256, identical before and after execution:
  `e4f5d702ef5da68ff7dc108d9a9fa3c841d1c07a2bbde04b4d2cb9d4bf5be40e`.
- Source event-position capture SHA-256:
  `c893e1a61b5fb45329228156f27ea9498c2e61358fd6733df871d3bbecccbf2f`.

Before committing, the common standalone MathItem projection was moved out of
the ignored draft directory into tracked `tools/math-port/support/`. The bytes
are identical (SHA-256
`8f4ff1d4e258eaf0baac59ac216852c897df7efe282b0a24b489199651f575eb`).
All four consumers were rebuilt at gate 236. Actual strict comparisons:

| Observer | Matches |
| --- | ---: |
| Six packages | 24/24 |
| Next seven | 23/23 |
| Font packages | 16/16 |
| Next six | 20/20 |

Each observer exited normally with code 0. Ignored provenance and reports are
under `build/math-port/*g235*`, `*g236*`, `g235-*`, and `g236-*`.

This is SVG/profile acceptance only. The three host assertions documented at
gate 234, broader core tests, private Renderer allocation lifetime, and Folia
integration remain open. Folia still uses QuickJS for math; Mermaid is paused.
