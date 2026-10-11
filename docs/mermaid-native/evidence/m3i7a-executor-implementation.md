# M3i7a 完整 pipeline 调度实施记录

本包在 `pipeline_executor/highp.rs`、`pipeline_executor/lowp.rs` 填入原 `start` 的控制流。`pipeline_executor.rs` 已有且仅有按编译后 `PipelinePrecision` 分派的薄入口，故保持原样。没有修改冻结类型、编译器、stage provider、数值算法、测试/gold/原库 observer、依赖、构建配置或 Folia；没有构建、运行测试、变异或提交。Root 负责实际原库对照与验收。

| pinned tiny-skia 0.12.0 Source | native 调度对应 |
| --- | --- |
| `pipeline/mod.rs:496–542` `RasterPipeline::run` | `run_pipeline` 使用编译结果自身的 precision 与唯一 `PipelineContext`；highp/lowp 各读取已编译 full/tail operation 列表，不重新判断资格或产生 stage。 |
| `pipeline/highp.rs:146–175` / `lowp.rs:165–192` | 各在进入行循环前初始化一次完整八通道寄存器；分别固定原高精度 8 lane、低精度 16 lane。SourcePixmap、mask 是借用值，destination 独占借用；pipeline context 无副本。 |
| highp `177–199` / lowp `194–216` | 每行从 `rect.x()` 到 `rect.right()`，每个整宽 tile 使用 full 列表；非零余宽使用 tail 列表。每 tile 从 operation index 0 开始，使用原 `dx/dy/tail`，跨 tile/跨行保留同一寄存器组。 |
| `Pipeline::next_stage`：highp `49–55`、lowp `61–67` | 本地 operation loop 每次读取已编译的下一项；`Return` 停止当前 tile。空列表在非空矩形的第一次读取处自然 panic，未提供成功 fallback。 |
| highp `292–305,333–335` | `LoadDestinationU8`、`StoreU8`、`LoadMaskU8` 在原高精度函数体为空且没有 continuation，native 对其直接结束当前 tile；不调用孤立 resource provider 的 Unsupported 分支。 |
| highp `337–350`、lowp `364` 起的 `MaskU8` | 通过既有 resource stage；其 `StageControl::Stop` 跳过本 tile 剩余 operations，再继续下一个 tile/行。full/tail 光栅访问由已编译 `Stage`/`Tail` operation 传给 `ResourceCursor.program`，没有自行替换 tail stage。 |

每个阶段显式进入已有 provider：highp common 35、pattern 5、pure blend 27、resource 9，外加三个原库 highp A8 空体；lowp common 11、pure blend 20、resource 12，其余对不适用的 stage 返回冻结的 Unsupported。common 阶段短暂独占借用 pipeline context；pattern 阶段构造只读 `PatternContext { source, pipeline }`；resource 阶段构造临时 `ResourceContext`，同时借用同一 context 和可变 destination。借用视图用完即释放，没有 context/pixels clone、第二 stage owner 或执行器内数值公式。

`MaskU8` Stop、`Return` 和 highp A8 空体各有独立分支。其余 provider 的 panic 原样传播，既不捕获也不清理目标 bytes/context；这保留了原库 debug u16 算术溢出和已发生的 destination 写入等部分效果。Unsupported/Error 仅沿冻结 `PipelineRunError` 返回，不被吞掉或替换为其他 stage。静态 `git diff --check` 无错误；**未执行**这 4,608 条 witness 的 native/Source 比较。

当前 4,608 条 Source witness 尚未覆盖完整 gradient、radial 和借用图案 whole-run，亦未覆盖三份 blitter pipeline、Shader constructors/dispatch、SVG/Direct2D 或 Folia。实施完成不等于 full executor 或最终 Mermaid 后端验收；Root 将按合同扩充原库 witness、运行 mutants 与完整 WARP 矩阵。
