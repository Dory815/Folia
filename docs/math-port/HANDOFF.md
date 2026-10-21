# MathJax 原生移植：当前交接

恢复/压缩后先实际读本文。架构、职责和详细验收见 [architecture](native-raii-architecture.md)、[files](native-raii-files.json)、[acceptance](native-raii-acceptance.md)。只复查能推翻现有结论的新变化；本机 ignored 候选用 `rg --no-ignore`。

## 当前 checkpoint（2026-10-21）

- **广义迁移仍暂停，goal为paused，等用户后续安排；不自动继续清理。** 本轮仅按用户要求更新Folia、构建Release MSI，并提交推送全部tracked改动；不创建tag、不发布GitHub Release。无live agent/build/native。
- 最新功能代码已接 WinUI 默认路径：TeX → request-owned MML → 原Common共享布局 → immutable Scene → SVG → Folia；数学不再走QuickJS。旧对象模拟源码/consumer清理未完，不能宣布全部移植完成；Mermaid暂停。
- 可测试安装包：`build/installer/bin/Folia-0.2.1-x64.msi`（自包含Release x64，53,329,574 bytes；同目录`.sha256`）。包SHA256 `6D044C437C52FC211866D9747738944AF06FE929D8EA252EA5D62DAEC3335541`。没有安装/GUI目视验收。
- setup通过；Debug实际worker20610通过且部署DLL已与core一致（CFDA6103…）。Release MSI最终build17523通过；包内EXE/DLL已实际解包核hash，Release DLL与core-release一致（B852D050…）。Release实际worker通过（mathbb/分式/Unicode/cache/错误恢复/Clear/重启；501ms停止，1.19s总耗时）。证据见acceptance及本机 `folia-latest-*.log`/`folia-latest-release-deployment.json`。
- 已修MSI资源清单，排除旧数学/字体JS及worker测试exe。复现：`powershell -ExecutionPolicy Bypass -File .\build_msi.ps1`；检查现成MSI构建产物用 `tools/math-port/winui-math-worker-probe.ps1 -Configuration Release -SkipBuild -SelfContained`，别重建成另一种部署配置。
- 最新旧测试清理：verification Source批 `46940bb0` 保留p32主3176+flags576全部期待、profile hash不变；删除批 `073e6abe` 已提交，build6566及最终核对通过（478活动/verified9/retained31/17targets/248接口/未解0）。历史SVG1047项断言、完整math/font/RAII等验收见acceptance，不重复广测。

## 还差什么 / 恢复后的具体起点

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
