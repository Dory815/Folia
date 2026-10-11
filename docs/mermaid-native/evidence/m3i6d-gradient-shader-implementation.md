# M3i6d Linear/Radial Gradient wrapper 实施记录

本包仅填充 `linear_program.rs::push_linear_gradient_stages` 与 `radial_program.rs::push_radial_gradient_stages` 两个既有函数体；参数名调整未改变签名或类型。没有修改其他生产文件、声明、测试、gold、Source observer、依赖或构建配置，也没有构建、运行测试或提交。Root 负责 53,862 条原库记录、变异和完整原生/WARP 验收。

| pinned tiny-skia 0.12.0 Source | native 对应及操作顺序 |
| --- | --- |
| `src/shaders/linear_gradient.rs:94–96` | `push_linear_gradient_stages` 直接调用已接受的 `push_gradient_stages(gradient, color_space, None, builder)` 并原样返回结果。不新建 builder，不读写 context、stage 前缀或 force 标志。 |
| `src/shaders/radial_gradient.rs:234–251` | `push_radial_gradient_stages` 仅调用已接受的 `prepare_radial_stages(kind)`，取得原 `p0/p1` 和 pre/post 阶段计划；不重算 radial 分支或规范化 f32 值。 |
| `src/shaders/radial_gradient.rs:253–258` | 在任何 generic Seed、求逆或 stage push 之前，把唯一 builder 的径向 mask 整组写为 `[0;8]`，参数写为 `[plan.p0,plan.p1]`。保留 `p0/p1` 的原始 signed-zero/非有限位；其余 context 组不触碰。 |
| `src/shaders/radial_gradient.rs:259–313` | 将 `Some(&plan)` 与同一个 builder 直接交给 `push_gradient_stages`，由已接受 generic producer 依原顺序推 radial pre、颜色与 radial post 阶段，并原样返回 bool 或传播容量 panic。没有备用 stage 列表、复制 context 或 SVG 路径。 |

径向 context 写入先于 generic producer，所以逆矩阵失败、Seed 容量 panic、后续任何容量 panic 均保留原 Source 已重置的 mask 和 `p0/p1`。Linear 不发生此重置。该包不包含 Linear/Radial constructor、SolidColor fallback、Shader variant dispatch、完整 executor、三个 blitter、SVG/Direct2D 或 Folia 接入。静态 `git diff --check` 是唯一检查；所有实际一致性仍待 Root 独立确认。
