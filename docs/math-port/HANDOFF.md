# MathJax 原生移植：当前交接

恢复/压缩后先实际读本文。架构、职责和详细验收见 [architecture](native-raii-architecture.md)、[files](native-raii-files.json)、[acceptance](native-raii-acceptance.md)。只复查能推翻现有结论的新变化；本机 ignored 候选用 `rg --no-ignore`。

## 当前 checkpoint：打开前完成数学准备（2026-10-21）

- **已接入并通过下述检查，待用户GUI试用。** 公式固定启用；设置UI、AppSettings字段/JSON键、数学优先队列及两阶段pending依赖均删除。旧迁移清理goal与Mermaid仍暂停，不自动恢复。
- 用户选择“先准备完整数学布局，再显示正文；期间显示进度”。LoadDocumentAsync后台准备独立session，完整测量后匹配当前宽度/主题再发布；进度可取消，新加载/Detach会停止旧准备。失败不发布半成品。图片/Mermaid仍有各自异步流程；本轮消除的是数学完成引发的重排。
- 职责：`NativeMathSession`封装同线程ABI/取消；`MathSvgStore`仅一个文档文件的追加/读取/RAII；`MathJaxRenderer`同步完成native+规范化并按块拥有指标/文件范围，旧块/旧公式回收，失效SVG累计后整理文件。原生会话在宿主线程复用，缓存移动只移动文档数据，原宿主销毁ABI句柄。SVG规范化共用串行字体数据库。
- `EditorSurfaceRenderer.Preparation.cpp`负责后台准备/采用边界；`EditorDocumentPreparationPass::PrepareAll`复用既有block preparer、DirectWrite行内对象与虚拟化批次，转移前释放设备/可视资源，仅保留块几何及完整数学索引。**CST/source/selection/history未改；尺寸/基线/断行属于布局，不写入CST。** MathJax提供可断行SVG片段，DirectWrite决定段落断行；宽窄窗口片段数可以相同（本例均81），不能期待片段数就是行数。
- 文件位于系统临时目录`FoliaMath/{GUID}.svgstore`，非继承/独占/CREATE_NEW，TEMPORARY + DELETE_ON_CLOSE；普通关闭/进程结束由句柄关闭删除，断电不保证清理。缓存不存源文档。渲染层补了同步异常的EndDraw/错误显示边界。
- **验证通过**：setup；WinUI Debug `math-preparation-app-build-final.log`（parallel≤4）；同步native/store `math-preparation-probe-final.log`（mathbb/分式/Unicode/公式诊断/块归属回收/跨线程缓存移动/16,781,312字节4097范围/析构及子进程结束删除，11.07s）；真实4100不同公式索引 `math-preparation-index-probe-final.log`（首尾ID和指标不淘汰，28.31s）。所有日志在ignored `build/math-port/`。
- **布局验证通过**：`math-preparation-layout-probe-final-v2.log`（15.1s）：7块富文档宽窄1266.01/671.563 DIP；长公式15行，反复视口内/外重建时逐行断点/基线/高度及公式坐标完全相同；4101虚拟化块中4100数学段落均测量（另1尾空块）；提前/中途取消不返回部分结果；匹配context采用、宽度失配拒绝；源码/模型标识保持。无GUI目视验收、无新Release/MSI；旧安装包不含本轮更改。
- 可运行 `build/app-winui/bin/x64/Debug/Folia.exe`；复验脚本 `tools/math-port/winui-math-worker-probe.ps1 -SkipBuild`（另`-IndexOnly`）、`winui-math-layout-probe.ps1`。布局脚本复用当前app工具链/PCH/IFC/CRT，禁止与app编译并行。console runtime已链接，CRT失败无桌面弹窗。
- 本轮准备分两批本地提交：接入/同步缓存与对应运行检查；完整布局检查及打包排除测试exe。无live agent/build/native。**下一步是用户实际GUI试用**（打开进度、取消/连续打开、窗口变化、滚动/编辑），按用户安排再决定Release MSI；不擅自恢复35个旧测试清理。

## 历史：上一版MSI与推送（2026-10-21）

- 上一轮已将全部tracked改动推至origin/master `2d58ded9`，无tag/Release。旧自包含Release MSI `build/installer/bin/Folia-0.2.1-x64.msi`，53,329,574 bytes，SHA256 `6D044C437C52FC211866D9747738944AF06FE929D8EA252EA5D62DAEC3335541`；不含当前数学准备改动。包内解包核hash及Debug/Release真实worker已过，详细证明见acceptance。需要新包时运行 `build_msi.ps1`，复验现成自包含产物用probe的`-Configuration Release -SkipBuild -SelfContained`，别改部署配置。
- 活动路径仍为TeX → request-owned MML → 原Common共享布局 → immutable Scene → SVG → Folia；旧对象模拟链/consumer尚未全部退出，不能宣布整个移植已完成。verification Source/删除批46940bb0/073e6abe已验；保留下面暂停工作的精确起点。

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
- **build/日志/候选/安装包均ignored，不随Git推送。** 远端接手者应按tracked脚本重建，不能假设本机候选存在；canonical Source profiles与owners在仓库中。没有live进程需要恢复。
