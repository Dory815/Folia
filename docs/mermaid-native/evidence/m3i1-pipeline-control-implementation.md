# M3i1 pipeline control implementation handoff

Status: implementation supplied for **Root's independent review and acceptance**. This packet projects tiny-skia 0.12.0's compile control only. It does not execute any stage, sample gradient pixels, or establish a complete Mermaid renderer.

Edited files are only `third_party/mermaid-rs-renderer/src/scene/prepare/pipeline_control.rs` and this report. No source oracle, fixtures, declarations, manifests, tests, backend, Folia integration, build or commit was changed or run by this agent.

## Original Source correspondence

The pinned Source hashes are in `docs/mermaid-native/m3i1-pipeline-control-contract.md`: `pipeline/mod.rs` SHA-256 `fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4`, `pipeline/lowp.rs` `3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88`, `pipeline/highp.rs` `81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416`.

| Original member/branch | Native projection |
| --- | --- |
| `Stage` ordinal declarations, `pipeline/mod.rs:68–150`; the 79 function-pointer slots of `highp::STAGES`, `pipeline/highp.rs:58–139`, and `lowp::STAGES`, `pipeline/lowp.rs:71–154` | Uses Root's frozen `PipelineStage` values. `pipeline_stage_is_lowp` is an exhaustive match over all 79 variants. It returns `false` exactly for the lowp `null_fn` slots and `true` for the executable lowp entries, including all non-gradient stages. The highp table has an entry at every ordinal. |
| `RasterPipelineBuilder::push`, `pipeline/mod.rs:366–368`, with `MAX_STAGES=32` and ArrayVec storage (`:347–350`); `compile` appending `just_return` (`:412–417`, `:449–454`) | `compile_pipeline` rejects slices longer than 32 as the earlier Source push-capacity failure; exactly 32 stages panic when Return needs a 33rd slot. An input slice cannot expose the original failure *time* during individual pushes, but it preserves the isolated call's panic outcome without truncation or a fabricated plan. Lengths 0–31 have capacity. |
| Empty-list early return, `pipeline/mod.rs:395–404` | Before eligibility or force-HQ, returns High, width 8, **both lists empty**, and `C::default()`; the supplied context is dropped. No Return is appended. |
| `is_lowp_compatible` tests the entire final stage list using lowp pointer equality to `null_fn`, `pipeline/mod.rs:406–409` | Calls `pipeline_stage_is_lowp` across the whole ordered input, not merely radial/gradient stages. Grouped exhaustive match maps all 36 `null_fn` positions: ordinals 2–3, 11, 32–33, 40–44, 47–50, 56, 58–69 and 70–78. The other 43 positions are lowp-compatible. `XYToRadius` at 57 is compatible; the other radial slots in that interval are not. |
| High branch on force-HQ or incompatibility, `pipeline/mod.rs:411–447`; Low otherwise, `:448–480` | Produces `PipelinePrecision::High`/`Low` for the whole program. Width follows original `highp::STAGE_WIDTH=8` (`pipeline/highp.rs:24`) and `lowp::STAGE_WIDTH=16` (`pipeline/lowp.rs:37`). No per-stage mixed precision. |
| Function mapping in input order, including repeats, then exactly one `just_return`, `pipeline/mod.rs:412–417,449–454` | `full` contains one `Stage` operation for each input occurrence followed by one `Return`. |
| Clone full list, substitute five tail function pointers in both precision branches, `pipeline/mod.rs:424–439,457–472` | `tail` starts as `full.clone()` and changes only `LoadDestination`, `Store`, `LoadDestinationU8`, `StoreU8`, and `SourceOverRgba` from `Stage(x)` to `Tail(x)`. All occurrences are handled; Return and all other operations remain in place. |
| Nonempty branches transfer `self.ctx` into the result, `pipeline/mod.rs:441–447,474–480` | Consumes `context: C` once and moves it unchanged into `CompiledPipeline<C>`. The only bound is `C: Default` for the empty branch; no Clone, numeric normalization or marker-bit rewrite is added. |

The eligibility table's non-gradient exclusions matter. For example `Clamp0`, `Gather`, `ColorBurn`, `SoftLight`, `Reflect`, `XYToUnitAngle` and every gamma stage force the **whole** final pipeline to High even if the gradient's own stages are lowp-capable. This is the Source table's compile decision, not a judgment about whether an individual lowp stage could someday be implemented.

## Boundaries and risks for Root review

The native signature accepts an already-collected slice, so the `>32` failure is observed when `compile_pipeline` is called rather than at the original `builder.push`; the `==32` Return overflow is separated explicitly. Panic *text* is not copied from ArrayVec. The accepted comparison should inspect the Source's success/panic result, pointer-labeled full/tail sequences, precision, lane width and context markers as specified by the frozen contract.

No executable `Stage` or `Tail` function is present in this plan. The Source's stage arithmetic, SIMD profile, gamma/blend output, and final pixels remain separate work. Build, unchanged-Source fixture comparison, mutation coverage and acceptance belong to Root; this handoff makes no parity or completion claim.
