# M3i6c generic Gradient builder producer 实施记录

依照 `m3i6c-gradient-builder-contract.md`，本包仅填充 `third_party/mermaid-rs-renderer/src/scene/prepare/gradient_program.rs` 中既有 `push_gradient_stages` 的函数体；未修改声明、现存 `GradientColor`/`GradientContext` helper、其他生产文件、测试、gold、原库 observer 或依赖。没有构建、运行测试或提交。Root 独立执行 20,644 条原库记录、变异及 WARP 验收。

原库基准为 pinned tiny-skia 0.12.0 `src/shaders/gradient.rs:106–259`。对应关系如下：

| Source 位置 | 函数体中的对应操作与部分状态 |
| --- | --- |
| 106–114 | 在任何求逆之前对传入的唯一 `PipelineBuilder` push `SeedShader`。已有 stage 前缀保留；满容量时由 `ArrayVec` 原位 panic。 |
| 115–123 | 原样调用 `gradient.transform.invert()`；失败发原文字 warning 并返回 `false`，已推 Seed 与既有 context 保留。成功后按原序 `post_concat(points_to_unit)`，再调用 builder 的有限/非 identity 守卫 `push_transform`。 |
| 125 | 仅将 `RadialStagePlan.before_color` 逐个显式映射到对应 `PipelineStage` 并 push；不读取 plan 的 `p0/p1/geometry_lowp_compatible`，不初始化或清零径向 context。 |
| 127–143 | Reflect、Repeat 各推对应 X1 阶段；Pad 仅在 `has_uniform_stops` 为真时推 `PadX1`。 |
| 145–163 | 恰有两个 stops 时保留 `debug_assert!(has_uniform_stops)`、原 ColorSpace expand 与逐通道 f32 减法；**先写** `builder.context.two_stop` 的 factor/bias，**再推** `EvenlySpaced2StopGradient`。容量 panic 因而保留新的 two-stop 值。 |
| 164–247 | 其他 stop 数量仅在局部建立一个 `GradientContext`：按原容量 reserve、端点同色排除、首个常色和 `t_values=ZERO`、严格 `t_l < t_r` 的系数/偏差算式及 t-value、末常色、len-before-padding、16 项补零，维持所有原 f32 运算和循环次序。局部对象不是另一个持久 context owner。 |
| 248–250 | 通用路径**先推** `Gradient`，再将局部 context 移入 `builder.context.gradient`；push 容量 panic 时旧 gradient context 保留。 |
| 252–258 | 颜色不全 opaque 时推 `Premultiply`，随后依原顺序推 `RadialStagePlan.after_color`；所有 push 完成后才返回 `true`。 |

前后 radial 阶段显式覆盖现有 `RadialStage` 的全部 13 个 variant，映射至同名 `PipelineStage`；只执行 Stage push，不改 generic producer 原本不负责的 `radial_p` 或 `radial_mask`。inactive context groups、force flag 与传入的 stage 前缀均由同一个 builder 保持。没有旧 `GradientProgram`、第二 stage 列表或第二持久 `PipelineContext` 执行路径。

静态 `git diff --check` 无错误，且函数体中没有原 stub；尚未执行 Source 对比、编译或运行测试。此交付不代表 generic producer、radial wrapper、完整执行器、三条 blitter、SVG/Direct2D 或 Folia 已获 Root 验收。
