# Context storage and native MML checkpoint: gate 273

The build succeeds for 14 targets using at most four compile jobs. All 2,277
registered inputs remain unchanged through the terminal observations. Gate 272
was stopped after the user removed the additional D2D raster requirement; its
already issued compiler jobs finished, and its input audit was unchanged. The
G6-only source, project and fixture changes were reversed. Current integration
acceptance uses SVG equality and actual worker behavior through the existing
SVG path; it does not require an additional pixel comparison.

Gate 273 is **not accepted**. The complete Host run has one failed assertion:
the new native mtable test still has an extra property projection after cleanup.
The three earlier Source factory-getter assertions now pass. The default
attributes literal synthesized by `MmlFactory::create` is the next identified
birth site to verify; its ownership correction remains a draft.

The Context subset passes 26 tests / 208 assertions, including the eight sparse
storage cases / 78 assertions. Foundation passes 116 / 1,334. The C ABI probe
exits 0. Full Core and the complete Source profile were not rerun after the Host
failure; Gate 268 remains the latest complete-tree/profile acceptance.

The 192-render owner observation exits 0 but does not show bounded object
lifetime. After persistent state setup, live records grow from 5,886 to 93,182
(+87,296), and owner-bearing records from 2,359 to 21,175 (+18,816). These match
the Gate 270 logical counts. Actual resident records now equal live records:
49,744 retired identities retain no Record allocation at the post-loop
checkpoint, while historical issued identities remain 142,926. Reclaiming those
retired allocations does not solve the remaining live-record growth.

The same observation takes 526.857 seconds, compared with about 107 previously.
The sparse storage destructor repeatedly rescans the live table when releasing
owners; its replacement worklist is being reviewed. The unit checks establish
the tested identity/reentrancy behavior, not acceptable long-session cleanup
performance. Neither the storage revision nor the complete tree is accepted on
the strength of those unit checks alone.

Remaining live-owner work includes builtin iterators and their internal step
records, and exact fresh default-argument projections. Public/custom iterator
identities and Source-retained aliases must remain usable. Independent review
found an extra descriptor callback in the first iterator draft; that draft is
pending correction rather than promotion. The native Folia worker candidate
now omits G6 and includes separate actual-worker SVG tests; it remains a draft.
Folia still uses QuickJS for math, and Mermaid remains paused.

Ignored authoritative records: `build/math-port/final-evaluation-g273.json`,
`build/math-port/g273-*.provenance.json`, and the corresponding stdout/stderr
captures. The Source comparison and small commits remain required after the
ownership corrections pass.
