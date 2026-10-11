# M3i6a shared PipelineContext ownership implementation

按 `m3i6a-context-consolidation-contract.md` 的机械迁移范围，修改了指定的十个 production files。此报告仅陈述实现；Root 仍负责原库观察、编译、回归、mutants 和验收。

| 文件 | 原字段/调用 | 当前唯一 owner / 借用 |
| --- | --- | --- |
| `gradient_program_types.rs` | `GradientProgram` 内平铺的 `transform/two_stop/gradient/radial_p/radial_mask` | 删除五组字段；结果只含 `returned/stages/context: PipelineContext`。旧值类型仍从 Root 的 `pipeline_context_types.rs` 按同一 type identity 重导出。 |
| `gradient_program.rs` | 五组平铺字段及逐字段 default | 一次创建 `PipelineContext::default()`，原径向参数、求逆成功后的 transform、双停止点或通用 gradient 按原控制流程直接写 `program.context`。求逆失败的 Seed 前缀、warning、return 时机和部分状态不变。 |
| `sampling/highp.rs` | common stage 接 `&mut GradientProgram` 并读写其平铺字段 | 接 `&mut PipelineContext`；Transform、gradient、two-stop、径向 `p` 与 mask 直接访问同一 context，`Mask2PtConical*` 的 mask 写入可供随后的 `ApplyVectorMask` 读取。寄存器/数值 body、阶段集合和不支持阶段先行拒绝不变。 |
| `sampling/lowp.rs` | 同一旧 program 参数 | 接 `&mut PipelineContext`；Transform、gradient、two-stop 直接读统一 owner，原 lowp packing/数值顺序不变。 |
| `sampling/pattern/types.rs` | `PatternContext` 自存 tile X/Y 与 sampler | 两生命周期 `PatternContext<'source,'context>` 只持 `SourcePixmap<'source>` 和 `&'context PipelineContext`；像素始终借用，tile/sampler 是 Root 类型的同一 identity，没有重复字段。 |
| `sampling/pattern/highp.rs` | stage 内部 view 读平铺 tile/sampler | 通过 `view.pipeline.limit_x/limit_y/sampler` 只读访问；临时 `Pipeline` 只持 view 借用、source borrow 和 SIMD 寄存器。Gather/tile/bilinear/bicubic 数值体保持原样。 |
| `sampling/pattern/mod.rs` | lowp 拒绝入口的一生命周期签名 | 改为两生命周期 `&PatternContext<'_, '_>`，仍对全部 lowp pattern stage 原样拒绝且不修改状态。 |
| `sampling/resources/types.rs` | `ResourceContext` 自存 uniform color 和 current coverage | 三生命周期 `ResourceContext<'pipeline,'dst,'mask>` 持独占 `&'pipeline mut PipelineContext`；destination/mask/AA 留在 run view。删除复制的 color/coverage 字段；原 `UniformColor` re-export 保持类型 identity。 |
| `sampling/resources/highp.rs` | `context.uniform_color/current_coverage` | 通过 `context.pipeline` 读取原始值；其他 destination/mask/AA、full/tail、Stop 时机和读写顺序不变。 |
| `sampling/resources/lowp.rs` | 同上 | 通过 `context.pipeline` 读取统一 owner；原 u16 转换、遮罩、融合和 store body 不变。 |

每个 producer/编译结果应有自己的 `PipelineContext`；Pattern 只读视图和资源独占视图作用于该次调用的同一个 owner，不通过 clone、字段拷贝、`Deref` 或旧签名 fallback 形成第二权威状态。三个 blitter builder、Pattern producer、完整 pipeline executor 仍是后续边界。本包未编辑 Root 的 `pipeline_context_types.rs`、module/defaults、test/fixture/gold、manifest、provider 或 backend，也没有构建或测试。`git diff --check` 静态检查无 diff 错误；数值与 Source parity 仍待 Root 独立验证。
