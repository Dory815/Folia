# M3d 径向内核逐分支 Source 审计（只读）

本文件只复述固定 Source 的几何准备和 raster stage；不冻结 API、不提供替代算法、不实施或验收。上份 `m3d-radial-source-audit.md` 初稿误取本机缓存的 tiny-skia-path 0.11.4，已获 root 授权纠正为 Cargo.lock:711–714 锁定的 **tiny-skia-path 0.12.0**。本审计下列阈值与变换定义均按 0.12.0。

## 源指纹与调用边界

路径以 Cargo registry 的 `index.crates.io-1949cf8c6b5b557f/` 为根。行号与 SHA256 对应本机锁定的完整文件：

| 文件 | SHA256 | 相关行 |
| --- | --- | --- |
| `usvg-0.47.0/src/parser/paint_server.rs` | `b268ffb743931c240962b1cd4296b332b12bef14283de1d2aee123072c9003ee` | 85–148, 277–404, 464–496, 948–1007 |
| `resvg-0.47.0/src/path.rs` | `cf49f94eba8a16cc3f730f544b49fbf1928b053dc4620c485677d1500dd897c7` | 22–108, 135–178 |
| `tiny-skia-0.12.0/src/shaders/radial_gradient.rs` | `3ae883526cead4d5361f8b6974a91dece8f62fe3740a1712be8ce6219efdd74c` | 21–80, 120–390 |
| `tiny-skia-0.12.0/src/shaders/gradient.rs` | `c8a87c87acb85764d5e13c95fd5f66531d24e2c145513e84adfa372d9a878847` | 17–256 |
| `tiny-skia-0.12.0/src/shaders/mod.rs` | `4f80ecb40d5a82985d97324748779cffd98252fca798c01c3f26969a1c27cbd3` | 27–102 |
| `tiny-skia-0.12.0/src/pipeline/highp.rs` | `81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416` | 256–270, 693–703, 894–978, 1020–1136 |
| `tiny-skia-0.12.0/src/pipeline/lowp.rs` | `3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88` | 118–145, 237–266, 558–729, 849–855 |
| `tiny-skia-0.12.0/src/pipeline/mod.rs` | `fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4` | 281–339, 352–455 |
| `tiny-skia-0.12.0/src/painter.rs` | `40f1d9523ab9875d83496cb7a1b4570aee2d958204dc91424fb6a5bd00c97105` | 220–313, 363–420 |
| `tiny-skia-path-0.12.0/src/scalar.rs` | `ffec71494c2c1c281300fab24efe4a33e900e770a836ea4c85aa4665e45b9e23` | 12, 25–70 |
| `tiny-skia-path-0.12.0/src/transform.rs` | `03f7e45c2708a416c35983ab6ad800211d5ceef92ba881b6025f2e8bc91a550f` | 183–196, 233–240, 296–399 |
| `tiny-skia-path-0.12.0/src/lib.rs` | `c5ccaf6aa5b793609bb10f3980a99f92b27a1e745617b9dc2498ae826e146de8` | 147–157 |
| `strict-num-0.1.1/src/lib.rs` | `03f39058c3ce3b67e65acedd5cef0eddaa61d732303b61cee9238cdf245ff854` | 265–281, 539–543 |

`resvg::convert_radial_gradient` 将 usvg 的 `(fx,fy), fr, (cx,cy), r, stops, spread, transform` 依序传给 `tiny_skia::RadialGradient::new`（`path.rs:135–153`）。下文记 `c0=(fx,fy), r0=fr, c1=(cx,cy), r1=r`；所有几何量在 tiny-skia 为 f32。以下谓词均是 Source 的有限/绝对误差判定，不是数学上的“恰等”：`E=DEGENERATE_THRESHOLD=1/32768`（`gradient.rs:18`），`N=SCALAR_NEARLY_ZERO=1/4096`（`tiny-skia-path/scalar.rs:12,55–69`）；`near0(x,t)=abs(x)<=t`、`nearEq(x,y,t)=abs(x-y)<=t`。

## 构造分支：按源码顺序

| 顺序及 Source | 条件 | 结果 |
| --- | --- | --- |
| `RadialGradient::new:129–139` | 任一半径 < 0；随后 stops 为空；随后只有一个 stop；随后 gradient transform 不可逆 | 分别为 `None`、`None`、`SolidColor(stop.color)`、`None`；单 stop 检查早于变换检查。 |
| `new:141–145` | `d=length(c0-c1)` 非有限 | `None`；Point::length 先算 f32 平方和，溢出后用 f64 重算再转 f32（`tiny-skia-path/lib.rs:147–157`）。 |
| `new:145–179` | `near0(d,E) && nearEq(r0,r1,E)` | 仅 `Pad && r1>E` 进入薄环特例：重建 stops `[(0,first),(1,first),(1,last)]`，调用 `new_radial_unchecked(c0,r1,...)`；其余 `None`。 |
| `new:181–189` | `near0(d,E) && !nearEq(r0,r1,E) && near0(r0,E)` | `new_radial_unchecked(c0,r1,原 stops,...)`。 |
| `new:192–200` → `create:328–341` | 尚未返回；`near0(d,N)`，且 `near0(max(r0,r1),N) || nearEq(r0,r1,N)` | `None`。注意 `E<d<=N` 仍可能进入本分支。 |
| `create:335–341` | 尚未返回，`near0(d,N)`，上述拒绝条件为假 | `Radial{radius1:r0,radius2:r1}`，`M=Translate(-c1).post_scale(1/max(r0,r1))`。 |
| `create:342–350` | `d>N`，但 `map_to_unit_x(c0,c1)` 不可构造 | `None`；其变换先把 c0→(0,0)、c1→(1,0)。 |
| `create:342–350` | `d>N && near0(r0-r1,N)` | `Strip{scaled_r0:r0/d}`，局部矩阵为 `map_to_unit_x`。 |
| `create:342–362` | `d>N && !near0(r0-r1,N)` | `Focal`；以 `r0/d,r1/d` 调 `FocalData::set`，其多边形映射不可构造时 `None`，否则保存修改后的矩阵。 |

`new_radial_unchecked:215–231` 的局部矩阵是 `Translate(-center).post_scale(1/radius)`，记录 `Radial{radius1:0,radius2:radius}`。这一步并未重新验证 `r0` 真等于零；入口只在阈值内判“近零”。`map_to_unit_x/ts_from_poly_to_poly/from_poly2:365–390` 用两点构造仿射矩阵，先反转源基底再与目的基底组合，不能将其缩写为未经核对的平移加旋转公式。

Focal 的准备次序（`radial_gradient.rs:29–79`）如下，是源码转写而非建议实现：

```text
u0 = r0/d; u1 = r1/d
f = u0 / (u0 - u1); swapped = false
if near0(f - 1, N):
    M = M.post_translate(-1,0).post_scale(-1,1)
    swap(u0, u1); f = 0; swapped = true
F = ts_from_poly_to_poly((f,0),(1,0), (0,0),(1,0)) or None
M = M.post_concat(F)
R = u1 / abs(1-f)
on_circle = near0(1-R,N)
if on_circle: M = M.post_scale(0.5,0.5)
else: M = M.post_scale(R/(R*R-1), 1/sqrt(abs(R*R-1)))
M = M.post_scale(abs(1-f),abs(1-f))
well_behaved = !on_circle && R>1
natively_focal = near0(f,N)
```

注意 `post_concat(other)=concat(other,self)`，`post_scale/post_translate` 也沿此方向（`tiny-skia-path/transform.rs:183–196,233–240,370–399`）；通用仿射 `concat` 的交叉项以 f64 乘加后转 f32，纯缩放/平移另有直接 f32 快路径。此运算次序和精度是 Source 的一部分。`new` 首先检查传入 transform 可逆；`Gradient::push_stages` 随后还检查结合外层 path transform 的矩阵能否反转，失败即不绘制（`gradient.rs:115–123`，`painter.rs:220–313,363–420`）。

## highp 阶段和方程

`RadialGradient::push_stages:234–313` 先计算 `p0,p1`：Radial 且 `radius1==0` 为 `(1,0)`，否则 `((max(radius1,radius2)/(radius2-radius1)), -radius1/(radius2-radius1))`；Strip 为 `((scaled_r0)^2,0)`；Focal 为 `(1/R,f)`。初始化 mask=0 的 context，交给 `Gradient::push_stages`。它的完整次序是：`SeedShader`（像素中心 x+0.5,y+0.5）、有效时 `Transform`，下表几何 pre-stage，Spread stage，stop 颜色 stage，可选 `Premultiply`，下表 post-mask（`gradient.rs:106–258`，`highp.rs:256–270,693–703`）。矩阵 `T=gradient_transform.invert()?.post_concat(points_to_unit)`；即先按 Source 变换约定反向映射到梯度空间，再做局部双圆锥映射。path 非恒等 transform 与分片平移先被 `painter.rs` 追加到 shader transform。

| 类型/条件（`radial_gradient.rs:263–311`） | highp pre-stage 中 `t=p.r` 的 Source 方程（`highp.rs:1020–1136`） | post-stage |
| --- | --- | --- |
| Radial，`p0,p1==(1,0)` | `t=sqrt(x*x+y*y)`。 | 无 |
| Radial，其余 | `t=sqrt(x*x+y*y)`，随后 `t=t*p0+p1`。 | 无 |
| Strip | `t=x+sqrt(p0-y*y)`；若 `t!=t`，保存 mask=0 并把 `t` 置零；其他 lane mask=全 1。 | stop/透明度后按位 `rgba &= mask`。 |
| Focal，`on_circle` | `t=x+y*y/x`，优先于下列三个方程。 | 见下方公共 Focal 序列。 |
| Focal，`!on_circle && well_behaved` | `t=sqrt(x*x+y*y)-x*p0`。 | 同上。 |
| Focal，`!on_circle && !well_behaved && (swapped || 1-f<0)` | `t=-sqrt(x*x-y*y)-x*p0`。 | 同上。 |
| Focal，其余 | `t=sqrt(x*x-y*y)-x*p0`。 | 同上。 |

Focal 公共 pre-stage **严格在上表方程之后**：若 `!well_behaved`，`t<=0 || t!=t` 的 lane mask=0 且 `t=0`；若 `1-f<0`，`t=-t`；若 `!natively_focal`，`t=t+p1`；若 swapped，`t=1-t`。只有 `!well_behaved` 的 Focal 才在颜色与 premultiply **之后**按位 mask RGBA。这个先 mask、后取负/补偿/反交换、再 spread/lookup 的次序不可折叠成最终 t 的有效性检查。Strip 只 mask NaN，不 mask `t<=0`。`mask` 是针对双圆锥无解区域的透明处理，不是 `fx/fy` 的位置夹取。

Spread（`gradient.rs:127–143`，`highp.rs:894–914`）在几何和补偿后：Reflect `t=abs((t-1)-2*floor((t-1)*0.5)-1).normalize()`；Repeat `t=(t-floor(t)).normalize()`；Pad 仅在 `has_uniform_stops` 为真时 `t=t.normalize()`，否则不显式 clamp。两 stop 且恰有 0/1 的路径用 `factor=(c1-c0),bias=c0` 的 `mad(t,factor,bias)`；其他 stop 路径按 `t>=t_values[i]` 搜索分段，以 f32 `factor=(c_r-c_l)/(t_r-t_l)`、`bias=c_l-factor*t_l` 和 `mad(t,factor,bias)` 取色（`gradient.rs:145–250`，`highp.rs:916–978`）。若任何 stop 非不透明则在颜色后 `rgb*=alpha`，最后才按需应用双圆锥 mask（`highp.rs:211–218,1098–1106`）。

## 正规化的归属及足够的 typed 输入

不能把三个层次混为一次“梯度 normalize”：

1. **usvg 解析**（`paint_server.rs:85–148,277–404,464–496,948–1007`）：解析单位/default/href、`r<=0` 或非有限降为末 stop 纯色、`fr` 负/非有限归零；stop offset 限 [0,1]，剔除连续三近同点的中间点，移动零/重复/逆序点，处理 stop-color/currentColor 与 stop-opacity；objectBoundingBox 的非零 bbox 变换吸收到 gradient transform，零 bbox 使 paint 无效。当前 Source 的 href 解析列表没有 `Fr`。这是输入语义，不是 tiny-skia 内核。
2. **resvg paint 转换**（`path.rs:135–178`）：每个 stop 的原 alpha 再乘 fill/stroke opacity，`to_u8` 为 `(alpha*255+0.5) as u8`，构造 RGBA8 stop；`GradientStop::new` 再将位置限 [0,1]。它不会把 `fr` 清掉或改成焦点偏移。fill/stroke 通过 `?` 区分 tiny-skia 的 `None`（该笔不绘制）。
3. **tiny-skia shader**（`radial_gradient.rs:120–362`，`gradient.rs:51–256`）：处理两个圆的有效性/几何分支；向缺失的 0/1 插入同色端点、固定末端、以 `N` 判 uniform，创建段的颜色系数；颜色空间扩展后选低/高精阶段、spread、premultiply 和无解 mask。usvg 的 0/1 stop 调整与这里的 dummy stop/系数是两次独立处理。tiny-skia 的单 stop 纯色、空 stop `None` 是直接调用者可触发的保护；经正常 usvg 路径时少于两个 stop 已在前一层降级。

供 root 冻结时核对的数据充分性，不指定新接口：一种方式是提供已解析的 f32 `c0,r0,c1,r1`、经过 usvg stop 处理且 resvg alpha 量化后的有序 RGBA8/f32 stops、spread、精确的合成 gradient/path/bbox 变换、颜色空间/绘制 opacity 结果和明确的 `None/纯色/着色器` 状态，由消费者按上述 Source 分支再准备一次。若要让消费者**只消费已准备油漆**，则须额外保留 Radial/Strip/Focal 判别、局部 points-to-unit 矩阵、Focal `R/f/swapped` 与 `p0/p1`、`well_behaved/on_circle/natively_focal` 或等价布尔、有效性 mask 规则，以及经 `Gradient::new` 处理后的 stop 列表/分段因子偏置/`has_uniform_stops`。两个信息集都能避免 SVG 生成/解析；目前只是充分数据清单，不是冻结的 scene schema 或 GPU 协议。

**lowp 差异**：`pipeline/mod.rs:402–455` 只要任一 stage 在 `lowp::STAGES` 为 `null_fn`，或显式 force_hq，就用 highp；`lowp.rs:118–145` 将全部双圆锥 Strip/Focal 方程、mask、取负、补偿和一般同心 scale/bias 标为 `null_fn`，故这些分支必为 highp。零起始半径的简单 Radial 若其余绘制 stage 均受支持则可用 lowp：仍以 f32x16 像素中心、变换、`sqrt(x*x+y*y)`、铺展和查 stop（`lowp.rs:255–266,558–661`），但颜色由 `round_f32_to_u16` 乘 255 加 0.5 转 u16，premultiply 用 `div255(rgb*alpha)` 的整数近似（`lowp.rs:237–242,634–729,849–855`）。highp 则以 f32x8 保留颜色与 mask 运算直到存储（`highp.rs:211–218,256–270,916–978,1020–1136`）。因此“全部径向一律 highp”不是 Source 观察。
