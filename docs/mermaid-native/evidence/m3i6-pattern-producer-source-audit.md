# M3i6 Pattern producer：原 Source 只读依赖审计

本报告只确定下一包的依赖与原库观察接缝；不冻结 native API，也不将 M3i5 的 7056 次 raw-bit 比较和仍在运行的 mutants 视为正式验收。所引源码是 Cargo registry 中 `tiny-skia 0.12.0` / `tiny-skia-path 0.12.0` / `strict-num 0.1.1`，与 vendored renderer `Cargo.lock` 的版本相符。以下路径相对于各 crate 的 `src/`；SHA-256 为原文件整文件值。

| crate / 文件 | SHA-256 | 关键行 |
| --- | --- | --- |
| tiny-skia `shaders/pattern.rs` | `3a527adb4fad67fe147dec4d041233f35fce9fc985bdf085ab1afc4326323185` | 68–95, 97–186 |
| tiny-skia `shaders/mod.rs` | `4f80ecb40d5a82985d97324748779cffd98252fca798c01c3f26969a1c27cbd3` | 50–74, 79–110, 127–145 |
| tiny-skia `color.rs` | `3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d` | 480–502, 522–545 |
| tiny-skia `pipeline/mod.rs` | `fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4` | 70–148, 252–375, 395–480, 496–542 |
| tiny-skia `pipeline/blitter.rs` | `a3189191d6ad4e1e03e738d21f5a26f310b7c00c33bae0021f0961054ad08f06` | 19–35, 37–77, 79–210, 263–289, 334–383 |
| tiny-skia `pixmap.rs` | `484d49d1f4c3c3c8fb3fab3e1b1535292de828c0f258c3cab9bda02c7d9626fc` | 328–365 (`PixmapRef` dimensions/data) |
| tiny-skia-path `transform.rs` | `03f7e45c2708a416c35983ab6ad800211d5ceef92ba881b6025f2e8bc91a550f` | 107–115, 129–165, 296–367 |
| tiny-skia-path `scalar.rs` | `ffec71494c2c1c281300fab24efe4a33e900e770a836ea4c85aa4665e45b9e23` | 46–54 (`invert`, `bound`) |
| strict-num `lib.rs` | `03f39058c3ce3b67e65acedd5cef0eddaa61d732303b61cee9238cdf245ff854` | 472–513, 671–676 (`NormalizedF32`) |

## Producer 的完整顺序与分支

`Pattern::new(PixmapRef, SpreadMode, FilterQuality, f32, Transform) -> Shader`（`pattern.rs:81–95`）按字段保存 pixmap、spread、quality、transform；先以 `NormalizedF32::new_clamped(opacity)` 保存 opacity。该构造器对有限输入夹在 `[0,1]`，**NaN、正无穷及负无穷都变为 `0`**（`strict-num:503–513`），并非保留原始传入的 NaN 位型。`Shader::transform` 对 Pattern 做 `post_concat`（`shaders/mod.rs:95–110`）；`Shader::apply_opacity` 对 Pattern 计算 `p.opacity.get() * opacity.bound(0.0,1.0)` 后 `NormalizedF32::new(...).unwrap()`（同文件 `127–143`）。这里 `bound = max.min(self).max(min)`（`scalar.rs:50–54`），其 NaN/无穷处理与构造器的 `new_clamped` **不同**：按实际 `min/max` 运算，NaN 与正无穷得上界 1，负无穷得下界 0；因此 observer 应记录构造前原始值、构造后保存值和 apply 后保存值，不能把三者当作一个“opacity”。不要对零或乘法结果另行规范化。

`Shader::push_stages(cs, builder)`（`shaders/mod.rs:79–93`）只按 variant 分派；Pattern 走原 `Pattern::push_stages`，`false` 明确表示不绘制，无 shader fallback。其他 variant 的行为不属于此包：SolidColor 用 `cs.expand_color(...).premultiply()` 后 `push_uniform_color`；线性、径向、扫掠调用各自 producer。Pattern 的 `Shader::is_opaque()` 恒为 `false`（`63–74`），影响 blitter 的 SourceOver 强度削减/fast path，不应由像素 alpha 或 Pattern opacity 猜测替代。

`Pattern::push_stages` 的每次写入按 `pattern.rs:97–186` 排列：

| Source 分支/位置 | 原操作、写入及 stage 顺序 |
| --- | --- |
| 98–104 | 首先 `self.transform.invert()`。`None` 只记 warning 并 `false`；**尚未 push SeedShader，也尚未写任何 context**。调用者传入的已有 stages/context 保持原样；新 builder 则保持空列表/default。`Transform::invert` 的 identity 快径、scale/translate 路径、一般路径和 determinant/finiteness 判断位于 `transform.rs:296–367`，不能以简单 `det == 0` 代替。 |
| 106–108 | 求逆成功后 push `SeedShader`（ordinal 6），再调用 `builder.push_transform(ts)`。后者只在 `ts.is_finite() && !ts.is_identity()` 时 push `Transform`（46）且写 `ctx.transform=ts`（`pipeline/mod.rs:370–375`）；identity 或非有限逆矩阵不写该 field/stage。注意求逆的 scale/translate 专门分支会直接构造逆矩阵，而一般路径另检查有限性。 |
| 110–123 | 复制 `self.quality`。逆矩阵 `is_identity() || is_translate()` 即改 `Nearest`。随后原样保留 `quality == Bilinear && ts.is_translate() && ts.tx == ts.tx.trunc() && ts.ty == ts.ty.trunc()` 时再改 `Nearest` 的条件。`is_translate()` 在 `transform.rs:143–145` 要求没有 scale/skew 且 **有** translate；前一个条件已经把这种矩阵降级，所以第二个整数平移判定对于正常执行路径被遮蔽，但 Source 仍有该分支，不可擅自把两段折叠成新的质量规则。单纯整数 translation 以及小数 translation 都在前一段降为 Nearest；identity 也降级。 |
| 127–146 Nearest | 先写 `ctx.limit_x={scale: width as f32, inv_scale: 1.0 / width as f32}`，再写同构的 `limit_y`，保持 f32 除法及赋值次序。Pad 不推 tile stage；Repeat push `Repeat`（48）；Reflect push `Reflect`（47）。最后 push `Gather`（11）。Pad 的 clamp 留给 gather。`ctx.sampler` 未写。 |
| 147–154 Bilinear | 写 `ctx.sampler={spread_mode, inv_width:1.0/width as f32, inv_height:1.0/height as f32}`，只 push `Bilinear`（49）。`limit_x/y` 未写；tile 操作由 sampler/stage 执行，不能额外推 Repeat/Reflect。 |
| 155–166 Bicubic | 同样写 sampler；依次 push `Bicubic`（50）、`Clamp0`（2）、`ClampA`（3）。Clamp 属 producer 序列的一部分。 |
| 169–178 opacity | 若已保存的 `NormalizedF32` **不等于** `ONE`，在 debug 下确认大小为 4 字节，写 `ctx.current_coverage=self.opacity.get()`，push `Scale1Float`（16）。若等于 ONE，既不 push 也不覆盖既有 coverage。这里不 premultiply pixmap 数据，不重新计算 opacity。 |
| 180–186 color space / return | 最后附加 `cs.expand_stage()` 的 `Some(stage)`，然后 `true`。`color.rs:522–529` 映射 Linear→无，Gamma2→`GammaExpand2`（70），SimpleSRGB→`GammaExpand22`（73），FullSRGBGamma→`GammaExpandSrgb`（76）。该 gamma stage **在图案采样、bicubic clamps、opacity Scale1Float 后**；Source 注释指出更理想的先于采样尚无中间 stage（181–182），不可自行提早。 |

`ColorSpace::expand_channel/expand_color`（`color.rs:480–502`）是 SolidColor 分派使用的另一路，Pattern producer 只使用 `expand_stage`；`expand_dest_stage`（530–537）与 `compress_stage`（538–545）由后续 blitter 添加，分别为 Gamma2 的 destination expand 71/compress 72、SimpleSRGB 的 74/75、FullSRGBGamma 的 77/78，Linear 无。故不能把 Pattern 的 source expand 与 destination expand、最后 compress 合成同一提前转换。

失败后的中间状态有两个层次：逆矩阵 `None` 是开头的原子失败，builder 完全不变；求逆成功后 Source 无其他 `false` 分支，但 `RasterPipelineBuilder::push` 受 `ArrayVec<Stage, MAX_STAGES=32>` 容量约束（`pipeline/mod.rs:347–368`），若传入已有足够多 stages，会按原顺序发生容量 panic，**可能留下已经推入的前缀和 context 写入**，不能把这归为 `false` 或回滚。独立 observer 应分别覆盖空 builder 与含 sentinel 前缀的 builder。

## 当前 native 拆分与唯一状态问题

当前 `scene/prepare/gradient_program_types.rs:27–31` 的 `GradientProgram.transform` 是已验收/冻结采样 `Transform` stage 的来源；`sampling/highp.rs:12` 的 `sample_high_stage` 读取它。`sampling/pattern/types.rs:4–24` 的 `SourcePixmap` 借用 premultiplied RGBA bytes，`PatternContext` 持 `source/limit_x/limit_y/sampler`，支撑已实现 M3i5 highp stages。`sampling/resources/types.rs:12–15` 的 `ResourceContext` 持 destination/mask/AA/uniform_color/`current_coverage`；高低 resource stage 使用该 coverage。这三块现能执行单 stage，但无法让一个 Pattern producer 只写一次原 Source 的 `Context.transform`、`limit_*`、`sampler`、`current_coverage` 后自动成为所有 consumer 的同一权威状态：直接向 `PatternContext` 写 tile/sampler、向 `GradientProgram` 写 transform、向 `ResourceContext` 写 coverage 会形成三份逻辑 context，且 `ResourceContext` 还掺有每次 run 的可变 destination 借用。

供 Root 冻结时考虑的职责划分（**仅候选文件责任，不是签名/API 决定**）：每次 producer 调用各有一个共享 shader/builder context，归属原 `Context` 的 transform、tile、sampler、coverage 等持久字段及 stage 列表；一个 Pattern producer 文件只执行上述 branch/写序并借用 `SourcePixmap`，不拷贝 pixel bytes；一个 consumer adapter 在调用现有 `sample_high_stage`、Pattern/resource stage 前借用该次调用唯一 context 的对应视图；一个 blitter 组装职责分别产生 anti-H、rect、mask 三份独立 builder/编译结果，运行时借用同一 source pixmap 和各自 destination/mask/AA。可用既有 frozen wrapper 作过渡视图，但必须明确每个 builder 的单一 owner 和写入时点，不能让该 builder 的 producer 和 adapter 各自保留可独立变化的 `transform`、`current_coverage` 等副本。尤其 Source 的 anti-H run 在 `blitter.rs:278` 把该 pipeline 的 `ctx.current_coverage` 改成 AA alpha/255；Pattern opacity producer 先写 coverage，后续 anti-H 运行可覆盖它，这个实际赋值次序要由原库证据决定，不能假设两个透明度自动相乘。

## 原库可执行 observer 的最小接缝

在单独复制的原 crate 里加 **append-only、同 crate 可见** observer；保持 `Pattern::push_stages`、`Shader::push_stages`、`RasterPipelineBuilder::push/compile` 与 `start` 原函数体不动。主观察路径由公开 `PixmapRef::from_bytes` + `Pattern::new` 构造真实 Shader，并可选原 `Shader::transform/apply_opacity`，随后调用真实 `Shader::push_stages(cs, &mut builder)`。在调用前后读取 builder 私有 `stages`（`pipeline/mod.rs:347–350`）及 `ctx`，记录：返回 bool；输入/保存 opacity 的 `f32::to_bits`；输入 transform 和逆/最终 context 的六个原始 float bits；原 quality/spread、尺寸；逐 stage ordinal 与先后；`limit_x/y` 四值、sampler 的 spread/两个倒数、coverage 的原始 bits；以及未触及字段的 sentinel 是否保持。只读快照 callback 可放在 wrapper 的调用前/后；无需在原 `push` 或 producer 体内插回调或重写决策。对特别的容量 panic，观察 `catch_unwind` 前后可访问状态的条件需由 Root 决定，避免被 move/unwind 规则误报；可先用 32/33 容量独立案例核实顺序。

再由原 `RasterPipelineBuilder::compile` 编译等价输入，记录 precision、实际 full/tail function pointer 列表及 compile 后 `ctx`。`compile(self)` 消耗 builder，故 stage/ctx 的编译前快照应先读取，再消费；不能以本地重算 stage 公式当 oracle。对三条 blitter 路径，应在复制的原 `pipeline/blitter.rs` 追加只读 accessor/wrapper：调用 **真实** `RasterPipelineBlitter::new`，读取其三条已编译 pipeline 的 context 与 full/tail 函数列表（指针解码可由同 crate `pipeline/mod.rs` 的只读 accessor 提供），并在运行时记录 anti-H coverage 的覆盖行为；producer 的精确 stage enum 序列仍由前述真实 `Shader::push_stages` wrapper 观察。这个安排不改三次调用本体，也不靠仿写 suffix 代替原 blitter；若要获得每条 builder 的编译前 enum 列表，则需要原方法内观察点，会越过 append-only 限制，当前不能宣称可得。

`RasterPipelineBlitter::new` 在 `blitter.rs:79–193` 连续构造 anti-H、rect、mask 三个 **新** builder，每个都独立 `set_force_hq_pipeline`、再次调用同一 `paint.shader.push_stages`，失败立即 `None`，并独立附加 mask/coverage、destination load/expand、blend、compress/store 等 suffix；不是一次 producer 结果被三个 pipeline 共享。Pattern `is_opaque=false` 也意味着 `blitter.rs:57–61` 的 opaque SourceOver→Source 削减不适用。source 借用 `PixmapRef<'a>` 从 Pattern 提取并存入 `RasterPipelineBlitter.pixmap_src`（19–27、196–205）；three pipeline contexts 按值归 blitter，源像素始终由调用方提供不可变借用，目标 `&'a mut SubPixmapMut<'b>` 独占借用，在 `run` 时分别传到原 `RasterPipeline::run`（`pipeline/mod.rs:502–539`; `blitter.rs:281–287, 363–369, 381–382`）。这也是 native 三次 producer 重入/借用生命周期必须验证的观察点。

此报告没有执行构建、测试或 Source observer；仍待 Root 决定共享 owner、观察数据模式和独立 red gate。它不证明完整 blitter、Direct2D、SVG、Folia 或 Mermaid 后端完成。
