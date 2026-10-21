# MathJax 原生移植：当前交接

恢复/压缩后先实际读本文。架构、职责和详细验收见 [architecture](native-raii-architecture.md)、[files](native-raii-files.json)、[acceptance](native-raii-acceptance.md)。只复查新变化；ignored候选用 `rg --no-ignore`。

## 当前 checkpoint：视口修复验收完成，转字体按需资源（2026-10-22，进行中）

- 最新用户澄清：字体按需单位为**一次请求/一批请求**，不是跨请求缓存；正文/轮廓/解码对象完成即释放。**只有一个批量接口，单个传1项**；批量去重、集中读取/解码以加速打开。数学线程及轻量解析器可持续驻留，字体正文不可跟着驻留。不使用SQLite；保留小索引和文件支持的只读源表，UI零同步IO。下一步在原font provider职责内明确request RAII与Common/SVG消费接口，再实现；尚未修改字体生产代码。用户再次强调滚走公式必须取消，不能因字体工作丢掉视口验收。
- 已修视口：删除layout命中隐式Read；GetSvg只查内存；绘制先裁剪，再按本帧可见fragment替换需求。移除离屏排队读、已开始读过期后丢弃/不回调；待渲染的离屏owner也取消，保留完成指标。**移除8MiB历史LRU**，只保留可见Document SVG，显式后台Prepare的临时读随调用释放。打开扫描16块一批等待worker，避免全文TeX队列；结束在worker销毁native引擎/字体，阅读缓存不重建引擎，编辑仍异步创建同一实现、Preview内存不写文件。
- 验收：Release自包含`math-viewport-release-build.log`与`math-viewport-release-msi.log`已成；最终worker `math-viewport-release-worker-final.log`3.43s PASS（100次视口替换取消107 Read，仅1实际Read+1当前nativeRender，离屏resident=0、Preview文件IO=0、读回逐字节/ID相同）；layout `math-viewport-release-layout.log`2.86s PASS（原普通编辑新增公式/4100数学与plain/稳定断行，另断言开头结束无nativeSession/SVG常驻）。生产`b8ded311`、测试`8b7a8028`已提交。无live build/agent。
- 内存调查：独立Release probe `build/math-port/native-font-memory-probe.cpp`（console runtime链接、`compile-native-font-memory.cmd`复现，先前CRT不匹配仅链接失败，已用MD runtime修正）显示字体单独构造private 1,970,176→118,718,464 bytes；整个Renderer 1,961,984→119,435,264，几乎全是字体。24,307 glyph/24,305资源，642,872段，PathSegment64 bytes、capacity49,486,912 bytes，SVG path capacity10,743,373。数据在`native-font-memory-{font,engine}.log`。最终worker释放引擎260,403,200→142,204,928；较早独立运行270,659,584→139,771,904。**这不是用户128→218MB同文档对照，不能声称已定位全部差额或回到128MB。** 用户明确基线是首次native运行时渲染（`2d58ded9`），并非后来的预计算版本；该基线也保留nativeRenderer，`src/math`/字体生成配置与HEAD无变更。此前以预计算生命周期解释用户差额不成立。已通过async工具询问实际文档路径及内存列，待回复。
- 用户已安装0.2.3，**固定版本为0.2.3，不能回退或擅自递增**；MSI已启用`AllowSameVersionUpgrades=yes`，解包也已核该项。新版同名MSI已覆盖旧本地文件：53,362,342 bytes，SHA256 `6E5475BEC3F45A47AD38659E9B3E52FCD5C84D03E1878604E68C105C51147D15`；148文件hash通过（`math-viewport-release-package-check.log`/`math-viewport-release-deployment.json`），含视口修复但尚不含字体按需。WiX仅ICE61同版本升级预期warning，未安装/GUI测试/发布/推送。Root独占Git/build，parallel≤4。下面旧checksum仅历史，勿混淆。

## 历史：0.2.3通用数学异步流程及MSI（被当前修复替代）

- 用户要求的0.2.3 Release自包含MSI已完成：`build/installer/bin/Folia-0.2.3-x64.msi`，53,350,054 bytes，SHA256 `39645DCE4ABE67D2DEB16B857E48C28DBA786FA3CF467E2F2B623E3F8CE5B416`；旁存`.sha256`。版本提交`71dbd654`，数学生产/验收提交`1c98e559`/`ad96dae5`。无live agent/build/test；下一步用户安装及实际文档试用，不自行继续旧迁移。未安装/推送/发布Release。
- Release验收：含setup的`math-scoped-async-release-msi.log`成功；自包含worker `math-scoped-async-release-worker.log`3.19s、layout `math-scoped-async-release-layout.log`2.68s均PASS，覆盖异步内存预览、SVG后台读回、普通编辑新增公式及稳定断行。MSI解包148文件SHA256全部匹配，native DLL匹配core-release，排除probe EXE/旧JS且probe后payload不变；记录`math-scoped-async-release-deployment.json`及`math-scoped-async-release-package-check.log`。

- **用户边界**：普通内容维持原视口排版；打开只提前异步算数学公式，文档SVG存临时文件。滚动不在UI做缓存文件IO；实时编辑预览异步计算、只存内存。任何文档都可后来写公式，禁止HasMath预扫描/文档分类/开关；之前错误的全内存方向已撤回。旧迁移清理与Mermaid仍暂停。
- 接入：统一扫描已解析的render节点，遇到数学才请求工作；零数学自然零请求/线程/文件。删除全文PrepareAll/离屏renderer准备，结果只含math cache，不携带全文块几何。原viewport、普通文本/图片/Mermaid流程保留。CST/source/selection/history/core/math均未改；尺寸/断行仍属布局，不写CST。
- 缓存：Document常驻immutable指标+文件范围，ready SVG使用8MiB LRU，viewport cache hit/GetSvg只排后台读任务；Preview独立purpose key、immutable共享SVG，仅内存。编辑BeginBlock/EndBlock及RetainBlocks移除旧owner、取消过期任务；旧state完成不能写入新state。PDF在既有后台导出线程等完整数学/读取结果，UI不等待。
- 文件职责：`EditorSurfaceRenderer.Math.cpp`仅数学扫描/采用，`EditorMathPreparation.h`仅数学结果/context，`EditorContentGeometry.h`共用既有宽度计算（保持原浮点运算次序）。`MathJaxRenderer.cpp`为facade，`.State.h`仅内部声明，`.Cache.cpp`为请求/owner/取消/LRU引用，`.Worker.cpp`为native+normalize+文件读写/整理；所有文件IO与native会话都在worker。Normalizer共享Runtime lease，DLL懒加载；工作结束前保留lease，避免静态析构与detached任务重叠。零公式不创建数学worker/normalizer/store。
- **已通过**：WinUI Debug `math-scoped-async-debug-build-complete.log`（parallel≤4）；最终worker `math-scoped-async-worker-accepted.log`25.62s（mathbb/分式/Unicode/诊断/owner/连续预览替换/取消/Clear/跨线程转移/文件关闭）。8个实际SVG共8,968,637 bytes，resident 7,847,770 bytes；淘汰后后台读回逐字节相等、renderId/指标不变，IO线程不同于调用线程，GetSvg调用2us。Preview缓存读写均0且无文件，Clear后旧文件后台关闭。
- **布局已过**：`math-scoped-async-layout-accepted.log`16.95s；普通段落用正常InsertText现场输入公式→不完整→修复→离开，内存预览更新、随后Document写文件；长公式15行反复重建断点/基线/坐标稳定；4100数学段落预计算、重复公式native仅1次，4100普通段落0数学工作（扫描60ms，无全文排版）；宽窄各6数学表达式，取消/context验证及source/model身份保持。私有内存/peak日志来自同时保留多个synthetic session的Debug测试，**不是用户文档前后性能比较，不能据此宣称运行内存已降低**。
- 最终Debug缓存回收专项 `math-scoped-async-cache-accepted.log`0.14s已过（多实例/并发/残缺文件/junction保留）。文件symlink权限1314仍跳过；目录junction实际覆盖。Debug IndexOnly `math-scoped-async-index-accepted.log`29.01s已过，4100不同公式首尾ID/geometry保留。本次Release未重跑这两项。
- 限制：8MiB仅ready SVG缓存预算，metadata/可见绘制资源/preview另外持有；单个SVG大于8MiB时允许一次交付的临时超额。尚未安装或GUI目视试用，亦未对用户实际大文档做同输入前后性能比较；不能据probe宣称整体内存已降低。可运行`build/app-winui/bin/x64/Release/Folia.exe`。Root独占build/Git，一次一个build、parallel≤4。

## 历史：0.2.2安装包（已被当前源码替代）

- `e1b66948`启动回收、`230c1430`版本、`5942bade`验包记录；未推送/发布。0.2.2自包含MSI `build/installer/bin/Folia-0.2.2-x64.msi`，53,337,766 bytes，SHA256 `2134773981B611CE18FE471739B19B64E60C3BF03EAFCE3A2115A418CC3C30E7`。148文件解包hash已验，两个probe EXE与旧JS资产排除；证明`math-cache-cleanup-release-deployment.json`。不要用旧MSI验当前异步流程。
- 0.2.2的全文块预排版/额外离屏renderer已由当前源码移除，不重新恢复。更早0.2.1推送至origin/master `2d58ded9`；当前不擅自推送/打包。原生接入仍为TeX→request-owned MML→原Common共享布局→immutable Scene→SVG→Folia，整个旧移植清理尚未完成。
- MathSvgStore仍沿用TEMP/FoliaMath/{GUID}.svgstore、独占非继承/DELETE_ON_CLOSE；普通退出/进程结束关闭句柄删除，断电遗留下次后台回收，不复用半写内容。缓存不含源文档，不提供未保存编辑恢复。此前删除旧probe小目录被自动审批拦截，保留`build/math-port/worker-probe/Debug/cache-reclaim-9f3496cb771f4cc395a1bbd80711917d`，不要为清它绕过审批；不影响产物。

## 历史：旧迁移还差什么（暂停）

1. **旧测试直接消费者仍有35个cpp/ixx +4个inc**（按文件，不是断言；未包含所有间接依赖）。旧consumer退出必须保留全部Source期待和实际数学覆盖。现有窄图仍是 `8ba6b468`/36测试，恢复时仅刷新verification删除后的35，不重做宽审。
2. copy/copy_boundary两Cpp尚未移除。Root已实际通过完整Source profile与全文Cpp映射：sparse 299行中108项copy + boundary6项，原profiles不变。canonical owners为 `sparse-mml-oracle.cjs`/`mml-copy-boundary-oracle.cjs`。恢复后退两Cpp/两freeze mapper，shared `mml-inheritance-regressions.cjs` 仅减114断言/2suite（1449/13→1335/11），保留metadata32/1与profile。本机候选 `build/math-port/copy-source-migration/`；已成功proof `copy-source-executed-proof.json` 不重复跑。
3. 旧生产owner链仍需整体收尾：Fields → ContextBindings → ParserRuntime，values唯一调用仍fields.cpp:439。未配置不等于死代码；不能孤删/stub或新增双轨。`fe5eba6e` 按用户“全部提交”保存了原有4文件投影补丁（context_bindings.cpp/.ixx、mtable_bindings.cpp、semantics_bindings.cpp）；它们不在478活动源内，未重新验旧协议。**这是交接存档，不是继续扩展投影补丁的方向**，仍按RAII架构退出整条旧链。
4. 最终完成前还要对剩余源码/工具/依赖清理做对应验收及完整完成审查；D2D不是本轮接入前提。用户实际GUI测试尚待进行。不要把既有Source-only检查或编译成功当作新的native运行验收。

## 固定边界与本机资料

- 保留MathJax的TeX/MML/Common家族职责；SVG和未来D2D消费同一完成布局，不引入平行算法、Scene家族或JS解释/对象语义层。旧原型/getter/rebind矩阵仅留离线Source来源记录。
- Root独占Git/CMake/build/native，一次一个build、parallel≤4。Windows console probe链接 `folia_console_test_runtime`；异常退出后只做有界诊断。小批提交，不reset，不覆盖接手人的改动。
- 已完成staging/执行proof不重复运行；512/513及verification旧256/257是host预算，不新增MathJax深度规则，不重新抽取旧对象协议测试。
- 之前6个dirty路径已按用户授权全部处理：4个旧binding源码保存于fe5eba6e；MathJaxSvgParser.cpp/SvgNormalizer.cpp只有行尾/状态差异，暂存后无内容改动。原备份仍在 `build/math-port/context-dirty-backup-20261021/`；不再存在“永远保护这6个未提交文件”的限制。
- **build/日志/候选/安装包均ignored，不随Git推送。** 远端接手者应按tracked脚本重建，不能假设本机候选存在；canonical Source profiles与owners在仓库中。live句柄只以上方当前checkpoint为准。
