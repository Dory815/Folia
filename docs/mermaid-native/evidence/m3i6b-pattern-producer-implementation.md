# M3i6b Pattern producer 实施记录

本包只填充 `pipeline_builder.rs` 的 `push` / `push_transform` 和 `pattern_program.rs` 的 `PreparedPattern::{new,transform,apply_opacity,push_stages}` 六个既有 stub body。未修改冻结的类型、声明、module、依赖、Source observer、gold 或测试；没有构建、运行测试或提交。Root 将依据原库 13,284 条完整记录、有效变异和 WARP 矩阵独立验收。

| 原 tiny-skia 0.12.0 Source | 本地映射及保留的顺序 |
| --- | --- |
| `src/pipeline/mod.rs:366–368` `RasterPipelineBuilder::push` | `PipelineBuilder::push` 对唯一 `ArrayVec<PipelineStage,32>` 调用原生 `push`；满容量按其原生行为 panic，没有截断、提前预检或第二列表。 |
| `src/pipeline/mod.rs:370–375` `push_transform` | 仅 `is_finite() && !is_identity()` 时先推 `Transform`、再写 `context.transform`。因此此步的容量 panic 不覆盖原矩阵；不合格矩阵不更改 stages/context。 |
| `src/shaders/pattern.rs:81–95` `Pattern::new` | `PreparedPattern::new` 保留原 `SourcePixmap` 借用和 spread、quality、transform，opacity 只经 `NormalizedF32::new_clamped` 保存；非有限构造输入服从该 provider 的零值语义。未复制像素。 |
| `src/shaders/mod.rs:95–110` `Shader::transform` 的 Pattern arm | `self.transform = self.transform.post_concat(transform)`，保留原有矩阵拼接方向及次序。 |
| `src/shaders/mod.rs:127–143` `Shader::apply_opacity` 的 Pattern arm | `NormalizedF32::new(self.opacity.get() * opacity.bound(0.0,1.0)).unwrap()`，直接使用 pinned `tiny_skia_path::Scalar::bound`；不复用构造器 `new_clamped`，不增加特殊数值分支。 |
| `src/shaders/mod.rs:79–93` Pattern 分派及 `src/shaders/pattern.rs:97–104` | native `push_stages` 首先求逆；失败发同一 warning 并返回 `false`，传入 builder 的 stages/context/force 原样保留。 |
| `src/shaders/pattern.rs:106–123` | 先推 `SeedShader`，再调用 builder `push_transform`；局部 quality 在逆矩阵 identity 或任意 translation 时降为 Nearest，之后仍保留 Source 被前一判断遮蔽的 Bilinear 整数平移判断。未改存储的 quality。 |
| `src/shaders/pattern.rs:127–146` | Nearest 先写 `limit_x`，再写 `limit_y`，每个均按 `dimension as f32`、`1.0 / dimension as f32` 构造；Pad 不推 tile stage，Repeat/Reflect 推对应 stage，然后推 Gather。 |
| `src/shaders/pattern.rs:147–166` | Bilinear 与 Bicubic 均先写 sampler 的 spread、宽倒数、高倒数，再推采样 stage；Bicubic 随后顺序推 Clamp0、ClampA。未选中的 tile/sampler 组不被重新初始化。 |
| `src/shaders/pattern.rs:169–178` | 存储 opacity 不等于 ONE 时，保留原 debug size 断言；先写 `context.current_coverage`，再推 `Scale1Float`，使容量 panic 可留下该部分写入。ONE 时不覆盖既有 coverage。 |
| `src/shaders/pattern.rs:180–186` 与 `src/color.rs:522–529` | 最后依据四种 ColorSpace 选择无 stage / GammaExpand2 / GammaExpand22 / GammaExpandSrgb，成功推完才返回 `true`。不提早 gamma 或触碰 destination/compress 阶段。 |

原 Source 的阶段容量与部分写入语义留给 `ArrayVec::push` 自然体现：Seed 或 Transform push 溢出时之后的字段不写；tile/sampler/coverage 则按原函数在相应 stage push **之前** 写入。每个 `PipelineBuilder` 仍仅持一个 `PipelineContext`，`PreparedPattern` 本身只持借用图像和原 Pattern 状态。

静态审核确认两个 production 文件没有剩余 stub，`git diff --check` 无错误。尚未执行编译或 Source 记录比较；本交付不代表 Pattern producer、完整 pipeline、三条 blitter、SVG/Direct2D 或 Folia 验收。
