# M3d 径向渐变 Source 审计（只读；API 尚未冻结）

范围：固定的 usvg/resvg 0.47.0、tiny-skia 0.12.0、tiny-skia-path 0.12.0、strict-num 0.1.1 和 windows 0.61.3；本文件只提出共享类型/私有消费者的接缝，未实现、构建或验收。当前 16 种隔离混合的本地验收不覆盖径向渐变、完整文本或其他油漆资源。更正记录：初稿误引用本机另一个已缓存的 tiny-skia-path 0.11.4；已据 Cargo.lock:711–714 改为实际锁定的 0.12.0，并更新以下哈希与行号。

## 固定文件指纹

下表路径以 Cargo registry 的 `index.crates.io-1949cf8c6b5b557f/` 为根；SHA256 是审计时的完整文件哈希。

| 文件 | SHA256 | 本审计用到的行 |
| --- | --- | --- |
| `usvg-0.47.0/src/parser/paint_server.rs` | `b268ffb743931c240962b1cd4296b332b12bef14283de1d2aee123072c9003ee` | 24–48, 85–148, 225–251, 277–414, 464–496, 539–548, 551–670, 814–904, 948–1007 |
| `usvg-0.47.0/src/parser/units.rs` | `efd72982854393e807c83ae16aef0d71af2c9a66d5778d77ed9a35bacc476688` | 11–64 |
| `usvg-0.47.0/src/parser/mod.rs` | `39c69b3328a6a0a85d91bde911944b6d2fa205dfc6061faed128886827ea6057` | 178–188 |
| `usvg-0.47.0/src/tree/mod.rs` | `cb11738534ce989b8583827d55e3e01096831ad40075f7c8351365da7ece35b5` | 261–402 |
| `usvg-0.47.0/src/tree/geom.rs` | `6f9668d43d5b9c95446b6a5bef6ab0e13e639da3b2f3dda28ff47281ae050a59` | 27–44 |
| `usvg-0.47.0/src/text/colr.rs` | `e810acc49a7f7f72c7de73a5d8961eb843116c276c54493a035c5b75dfc1fe03` | 155–185 |
| `resvg-0.47.0/src/path.rs` | `cf49f94eba8a16cc3f730f544b49fbf1928b053dc4620c485677d1500dd897c7` | 22–108, 135–178 |
| `tiny-skia-0.12.0/src/shaders/radial_gradient.rs` | `3ae883526cead4d5361f8b6974a91dece8f62fe3740a1712be8ce6219efdd74c` | 18–80, 98–370 |
| `tiny-skia-0.12.0/src/shaders/gradient.rs` | `c8a87c87acb85764d5e13c95fd5f66531d24e2c145513e84adfa372d9a878847` | 14–256 |
| `tiny-skia-0.12.0/src/shaders/mod.rs` | `4f80ecb40d5a82985d97324748779cffd98252fca798c01c3f26969a1c27cbd3` | 27–102 |
| `tiny-skia-0.12.0/src/pipeline/highp.rs` | `81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416` | 256–270, 693–703, 894–965, 1022–1142 |
| `tiny-skia-0.12.0/src/pipeline/lowp.rs` | `3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88` | 118–145, 255–275, 558–661 |
| `tiny-skia-0.12.0/src/pipeline/mod.rs` | `fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4` | 352–455 |
| `tiny-skia-0.12.0/src/pipeline/blitter.rs` | `a3189191d6ad4e1e03e738d21f5a26f310b7c00c33bae0021f0961054ad08f06` | 29–45, 76–181 |
| `tiny-skia-0.12.0/src/painter.rs` | `40f1d9523ab9875d83496cb7a1b4570aee2d958204dc91424fb6a5bd00c97105` | 220–313, 363–420 |
| `tiny-skia-0.12.0/src/color.rs` | `3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d` | 255–261, 455–500 |
| `tiny-skia-path-0.12.0/src/scalar.rs` | `ffec71494c2c1c281300fab24efe4a33e900e770a836ea4c85aa4665e45b9e23` | 12, 55–69 |
| `strict-num-0.1.1/src/lib.rs` | `03f39058c3ce3b67e65acedd5cef0eddaa61d732303b61cee9238cdf245ff854` | 265–281, 539–543 |
| `windows-0.61.3/src/Windows/Win32/Graphics/Direct2D/mod.rs` | `c4d3309891f3bb465637e88b47811667becf6d9596b11c1e201d91d2ceccc7b4` | 1647–1655, 10891–10903, 11208–11215, 4865–4877 |

本地现状指纹：`third_party/mermaid-rs-renderer/src/scene/paint.rs` 为 `d84c9ba2641b4e78927368021469c621409640510ebb6ba19247879279cc791c`；`src/backends/direct2d/paint.rs` 为 `02c21b2d5ae5d8456ed24ee019b18f9399f8cd4d1ac361b02d53ee2986aa9abf`。这些是工作树观察值，不是固定上游源。

## usvg 参数与正规化

`convert_radial`（`paint_server.rs:85–148`）先从本节点或 href 链找带 stop 的渐变并转换 stop；没有 stop 时 `None`，只有一个 stop 时变成该首个 stop 的纯色，至少两个才构造 `RadialGradient`。默认 `gradientUnits=objectBoundingBox`、`r=50%`、`fr=0`、`cx=cy=50%`、`fx=cx`、`fy=cy`，spread 默认 Pad。半径 `r` 须严格大于零且有限；零、负、非有限时返回末 stop 的颜色/不透明度而不建渐变（`tree/geom.rs:27–44`）。`fr` 通过 `PositiveF32::new(fr).unwrap_or(ZERO)`，允许零，负数、NaN 和无穷转为零；并未限制 `fr<=r`（`strict-num:265–281`）。`fx/fy` 没有被 usvg 裁剪至外圆内，resvg/tiny-skia 路径也没有这样的预裁剪。

`resolve_number` 通过 `resolve_attr` 与 `convert_length`（`paint_server.rs:407–496`，`units.rs:11–64`）。在 objectBoundingBox 中百分比除 100；在 userSpaceOnUse 中 `cx/fx` 百分比依 viewBox 宽，`cy/fy` 依高，而 `r/fr` 落在其他属性分支，依 `sqrt((width²+height²)/2)`。绝对单位经 DPI、字体大小换算，再落成 f32。href 解析允许径向坐标仅从径向渐变继承、units/spread/transform 从线性或径向继承；**观察到 `resolve_rg_attr:464–496` 列举了 `Cx/Cy/R/Fx/Fy`，没有列举 `Fr`**，因此当前 Source 的 href-only `fr` 会取默认零。此为 Source 行为记录，不用规范意图替代。

`convert_stops:277–404` 按文档顺序读取 stop；非 stop 子节点跳过。offset 缺失沿用前一个原始值；无单位数直接用、百分比除 100、不支持单位沿用前一个，再以 f32 限在 [0,1]。stop-color 的 `currentColor` 查找 color 属性；普通颜色解析失败退黑；颜色自身 alpha 与 stop-opacity 相乘。随后三步处理：连续三个 offset 在 4 ULP 内相等时移除中间项；相邻两个零 offset 时后项移为前项加 `f32::EPSILON`；逆序或近等 offset 时把前项移为 `offset1-EPSILON`，后项固定于 `offset1`，都经 clamped StopOffset。不能把这套处理简化成排序、去重或 D2D 自动正规化。`stops_to_color:539–548` 的少于两点降级取首 stop，`r` 无效的降级取末 stop，二者不同。

usvg 在整棵树的 bounding box 已知后运行 `update_paint_servers:551–670`。对于 objectBoundingBox，零宽/高 bbox 使填充或描边服务器失效；否则 `to_user_coordinates:948–1007` 把 bbox 变换 post-concat 到原 gradientTransform，坐标及 `fr` 保持原归一化数值，units 改为 UserSpaceOnUse。文本扁平化路径使用文本 bbox，context paint 有其上下文变换与 bbox（`process_context_paint:814–886`、`process_paint:887–924`）。换言之，后端看到的 usvg 渐变已吸收 objectBoundingBox，而源 Mermaid 的共享 scene 若保留 units，必须由共享准备层提供同等形状 bbox 与变换次序。

字体 COLR 径向 callback 也是可达输入：`usvg/src/text/colr.rs:155–185` 写出 `x0/y0/r0` 对应 `fx/fy/fr`、`x1/y1/r1` 对应 `cx/cy/r`。它属于待定的共享文本资源边界；这里不建议用该中间 SVG 作为原生 Mermaid 路径。

## resvg 与 tiny-skia 的实际绘制

`resvg/src/path.rs:22–108,135–178` 的填充/描边都将 `(fx,fy), fr, (cx,cy), r, stops, spread, transform` 原样按此顺序交给 `tiny_skia::RadialGradient::new`；填充/描边 opacity 先乘每个 stop 的 alpha，并经 `Opacity::to_u8` 执行 `(alpha*255+0.5) as u8`，再用 `Color::from_rgba8`。不是简单地把整支画刷 opacity 延后乘一次。tiny-skia 返回 `None` 时，`?` 使该 fill/stroke 返回 `None`，没有替代画刷。Path 的绘制次序、抗锯齿和 blend mode 随原路径执行。

路径的外层绘制 transform 还会被 tiny-skia 的 painter 追加到 shader transform：非恒等填充先变换 path、再 `paint.shader.transform(transform)`；大图分片的平移也同步作用于 path 与 shader。描边/细线及分片有对应变换路径（`painter.rs:220–313,363–420`，`shaders/mod.rs:96–102`）。因此底层反转的是已结合外层路径变换的 shader 矩阵；审计单独一个 gradientTransform 不足以说明最终像素采样。

`tiny-skia/shaders/radial_gradient.rs:120–229` 的检查顺序：负半径→`None`；零 stops→`None`，单 stop→纯色；不可逆 transform 或非有限中心距离→`None`。中心距离不超过 `DEGENERATE_THRESHOLD=1/32768` 时，半径也近等：仅 Pad 且末半径大于阈值才构造一条 0/1/1 的“无限薄环”三 stop 径向着色器（第一色、第一色、末色），其他 spread 返回 `None`；中心近等且起始半径近零，走简单径向优化。`create:316–364` 对同心非等半径构造带 `radius1/radius2` 的径向类型；非同心但两半径在普通 `SCALAR_NEARLY_ZERO=1/4096` 内相等则走 Strip；其余走 Focal 双圆锥。Focal `set:27–68` 的 `r0/(r0-r1)` 接近 1 时交换两个圆并记 `is_swapped`，建立局部映射、半径和额外缩放；后续可能有“焦点在圆上”、well-behaved、smaller、greater 分支。这里没有把焦点点位强行拉回外圆。默认/特定近似阈值见 `tiny-skia-path/scalar.rs:12,55–65`。因此正 `fr`、`fr>r`、焦点在外及相等半径都有可观察而不同的 Source 分支，不能替换成普通径向画刷。

`Gradient::new`（`shaders/gradient.rs:51–103`）在端点不是 0/1 时增补相同颜色的端点 stop，再固定最后为 1、保证单调并判定均匀间隔。每个 shader 在 `push_stages:106–256` 以像素中心 `x+0.5,y+0.5` 起始，反转 gradientTransform 后与局部 points-to-unit 矩阵拼接；反转失败即无绘制。几何阶段为半径及可选同心 scale/bias，或 Strip/Focal 的平方根、NaN/非正退化 mask、补偿及交换（`radial_gradient.rs:234–313`，`pipeline/highp.rs:1022–1142`）。之后 Reflect 为三角波，Repeat 为小数部分；Pad 只在均匀 stop 时显式 clamp，非均匀 stop 保留未 clamp 参数交给通用 stop 搜索，以维护端点硬停。两 stop 0/1 走线性因子/偏置，其余搜区间、预计算每段 factor/bias；含透明 stop 时 premultiply，退化 mask 在颜色求值后施加。高精阶段实现见 `highp.rs:256–270,693–703,894–965,1022–1142`。`pipeline/mod.rs:402–455` 按所有 stage 是否有 lowp 实现选择 lowp/highp；`lowp.rs:118–145` 对双圆锥、一般同心 bias、mask 等为 null，因此这些路径走 highp；零起始半径的简单径向路径才可能只使用 lowp 支持的 f32 坐标/查找及较低精度颜色阶段（`lowp.rs:558–661`），还要看其余绘制 stage。`ColorSpace::Linear` 是 tiny-skia 默认且在本实现中不做 gamma 展开（`color.rs:455–500`，`resvg/path.rs:35–59`）。

## Direct2D 能力边界

固定 Windows 绑定只提供 `D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES { center, gradientOriginOffset, radiusX, radiusY }` 与 `CreateRadialGradientBrush`（`Direct2D/mod.rs:1647–1655,11208–11215`）；设备上下文的 `CreateGradientStopCollection` 另有颜色空间、精度、extend 与插值参数（4865–4877）。可表达椭圆外边界、一个梯度原点与 Pad/Reflect/Repeat 扩展，且 brush transform 可承接仿射变换；结构中没有起始圆半径、两个任意半径、双圆锥退化 mask 或 tiny-skia 的 stop 搜索规则。微软的 [ID2D1RadialGradientBrush 说明](https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nn-d2d1-id2d1radialgradientbrush) 明言 stop 0 在 gradient origin、stop 1 在椭圆边界，原点在椭圆外时虽可工作但渐变不明确定义；[属性结构说明](https://learn.microsoft.com/en-us/windows/win32/api/d2d1/ns-d2d1-d2d1_radial_gradient_brush_properties) 仅列上述四个字段。因此不能把 Source 的外部焦点“夹回去”，也不能称任一 D2D 内置画刷为完整双圆锥等价物。即使 `fr=0` 且焦点在内，停点/透明度、坐标、铺展和抗锯齿的像素同等性仍需独立 WARP oracle，不能由参数形状推出。

现有共享 scene（`src/scene/paint.rs:16–45`）只有 `None/Solid/LinearGradient` 与线性资源；现有 D2D 画刷消费（`src/backends/direct2d/paint.rs:99–154`）也只处理这三者。缺口同时在便携类型与原生消费者，而非只缺一个 Direct2D API 调用。

供 root 冻结时考虑的最小接缝：共享 scene 添加径向 paint 引用及一个有序的径向资源，保留两个中心、两个半径（尤其 `fr`）、全部经 Source 规则准备的 stop/alpha、spread、坐标系与仿射变换，以及“无绘制/首色/末色/双圆锥”的准备结果或能无损重建这些结果的输入。共享准备层负责源语义下的长度换算、href、bbox、stop 正规化与退化判定；SVG 和 D2D 两消费者读取同一 typed 结果。D2D 私有消费者可被约束为：仅在 root 证明等价的子集使用内置径向画刷，其他分支用能表达双圆锥方程、mask、铺展和 stop 查找的原生 GPU 效果；任何分支不得静默降为“最近”的线性/普通径向、透明或 SVG 往返。这是接缝建议，**不是冻结 API 或验收结论**。

独立验收观察点应至少覆盖 `fr=0/fr>0/fr>r`、同心/非同心、等半径 Strip、焦点在圆上/外部、不可逆变换、零或负 `r`、零/负 `fr`、0/1/2/重复/逆序 stop、首末 offset 未覆盖 0/1、三种 spread、objectBoundingBox 零/非零 bbox、userSpace 百分比、fill/stroke opacity 与 WARP 多 DPI 像素；同时记录构造是 `None`、纯色还是着色器，避免把所有不绘制情形混为一类。

