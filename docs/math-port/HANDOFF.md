# MathJax 原生移植交接入口

当前：Lite/Xml/HTMLDomStrings/texhtml/Finder 已通过组件验收；最新 build exit 0（texhtml-final-build.log），四组 *-accepted-focus.log 全 exit 0。完整 texhtml-full.log exit 0：741 tests / 210529 assertions / 149 suites，无失败/跳过。DOM/Lite 已提交 4c379ef7，Xml bridge 为 c9d76a1f，CompileServices/Finder 为 ef04091d；正在提交 texhtml/验收。无活动 build/test；全部 Luna 冻结。额度已恢复。

下一步：提交 HTMLDomStrings/texhtml/验收，再冻结 Common→Scene 的原 family emission 接口并分 Luna。旧 texhtml methods/services 四文件与未注册 html_methods_identity UT 已删除，职责只在原 configuration pair。旧 Finder host 有 inactive parser_runtime/render_session 入边，随旧对象层退出显式清理；node_store/source_text_content 受保护 dirty Lite consumers 仍需，暂不删。

最近修正：LiteParser.checkDocument 的 replace(new,old) 参数顺序；测试 JSON/片段选择、parsed document 无 defaultView、有效 Element 不触发 span 包装、texhtml 空根、disabled TeX 的 U+2212。texhtml UT 空指针来自丢弃 compile 返回值再读 bad.root，已修并单例 exit 0；无未诊断异常退出。direct fixture 现从 pinned inputs 保存 initialAllowTexHTML，而非 jax.options 输出；22 direct + 1 recovery，Source JSON/期望未改。Source Ref/eqref 显式 display=false 已核对，无待修问题。

架构/职责：六个 Lite 原文件是同一 folia.math.adaptors.lite_adaptor primary partitions；core native DOM / Lite / immutable Xml bridge / HTMLDomStrings / texhtml 原 pair / FindMath 与 FindTeX 原 pair 均已交付冻结。Root 持有 CompileServices 借用/异常清理、ProtoItem、CMake 和共享调用点。旧 render_session 为 inactive JS 路径，不补兼容 overload。具体架构见下方链接。

Source witnesses：Lite 41 records / 30 parser inputs，当前 C++ fixture SHA bb5c7ee4c20104b67cebb486d7e52f350b44dc54d0cad34e89baa9d0d768251c；texhtml 22 direct + recovery + 5真实pipeline，5份原 snapshot 全匹配，Source JSON SHA c13c8eb7ca5c9dea20e5a264a4b1437194949c01bd4f8c719dbf5a2959dfc4d5；Finder 22 inputs / 59 records。Root 从 C:/Windows 最新 bytechecks 三者全 exit 0（*-source-current-root.log）；canonical 自包含。生成物 tools/math-port/*-source.cjs、tests/math/fixtures/{lite,texhtml,find_tex}.inc。

所有 agent 已结束，无活动 build/test handles；需要语义修复时 followup 原 Luna owner（lite_parser、lite_native_contract、mhchem_package、html_dom_strings、mhchem_package_cases、mhchem_dependency、texhtml_tests）。common_scene_contract 的只读结论：现 Common 缺实际 Scene emitter，分式/根式 geometry 已可复用；不注册旧 Scene wrapper hierarchy，不重算数学位置。候选仍在 ignored build/math-port/{lite-source-candidate,texhtml-source-candidate,find-tex-source-candidate}/。Root 独占 build/test/Git/CMake/HANDOFF。保护的原 13 dirty 路径见末尾。

恢复时先读本文件，再检查相关当前改动。架构见 [native-raii-architecture.md](native-raii-architecture.md)，验收见 [native-raii-acceptance.md](native-raii-acceptance.md)。只更新这一个当前 checkpoint；历史交给 Git、测试和 Source witness，不追加阶段流水账。

## 目标与固定输入

完整目标仍是 MathJax 4.1.3、固定 38 个 TeX 包：TeX→MML→共享 Common 布局→不可变 Scene→SVG→Folia，替换 math QuickJS。整体未完成；Mermaid 暂停，没有 D2D 像素 gate。

- Source：`.reference/MathJax-src`，commit `fb987178c2d279a99b0db5ea02e751691703955e`
- CJS：`tools/mathjax-bundle/node_modules/@mathjax/src/cjs`
- Full38 compiled hash：`cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a`
- Profile/pins：`tools/math-port/reference-profile.cjs`、`docs/math-port/scope.json`

## 已接受状态与下一步

最近提交：`1f909e64` MML unicode marker，`c858e986` Entities 原 owner / 全量实体数据 / 测试，`d348900a` Unicode 原 owner / 测试。此前已接受包不重做。FunctionList 命名已恢复原 owner，不再重查。

- 最终 build：`build/math-port/unicode-entities-build.log`；full：`unicode-entities-full.log`。
- Root Source bytecheck：`entities-source-root.log`、`unicode-source-root.log`；完整 profile：`unicode-entities-profile-root.log`。以上日志均在 `build/math-port/`。
- Entities 已保留全部 2128 raw records（含 base/shard 重复项）；同步首次装表后重扫整个原输入，匹配 Source 完成 retry 的结果。
- Unicode 字体缓存由私有 CommandMap / MapHandler registry 拥有；同 registry 跨 TeX/reset 保留，不同 registry 隔离，析构释放。fixture 保留 NUL/孤立 surrogate；捕获别名依赖编译回调表。
- mhchem：220 package inputs（含全部 113 原用例）、63×6 replacement stages、14 Source mutations，其中 8 个是原生 typed 场景，6 个仅记录已撤销的 JS API 行为。依赖完整比较 7960 records（1679 parse、6279 pattern、14 FSM / 62 regex 的两个全表记录）。最终 build `mhchem-physical-parent-build.log`；包 focus `mhchem-package-focused-fixed.log`。parser UT 聚合检查全部 rows，当前为 9 assertions，不把 row 数当断言数。
- Mhchem 原 maps 单向 RAII 持有，无 JS module 对象；重复初始化保留状态。MathChoice 按 Source 用物理 `parent()` 替换节点；`Parent()` 跳过 inferred row 会留下未替换节点。此处已有真实包级 MML 对照覆盖。
- 两个 mhchem canonical Source 工具已自包含，Root 最终 bytechecks `mhchem-source-root-final.log`、`mhchem-parser-source-root-final.log` 通过，不依赖 ignored 草稿 oracle/corpus。replacement helper 使用原 lexical TokenMap 引用，不取当前 registry replacements。

顺序：mhchem 包与依赖 → texhtml → 实际 Common→Scene 发射 → SVG 全量对照 → Folia 集成 → 旧对象层退出。不可把包级测试当成完整 SVG/Folia 验收。

Common 测量辅助 MML 属于本次请求的 MmlDocument；同 document 重测可能增长至 document 析构。最终 renderer 每 request 新建 document，不把 document 留在 session 或 Scene。

## 工作约束与检查

保留 MathJax 原 owner/职责，复用已验算法；不建 native2/new/typed 平行实现，不模拟 JS 对象，不堆释放补丁。Source fixture 必须独立从 pinned Source 生成，不改期望迎合 C++。Node heap 512、cwd C:/Windows、每进程只 createReference 一次。

默认 Luna；必要时仅用 GPT-6.1 Sol。Root 控制 CMake、native build/test、Git；一次一个 build，总并发最多 4。Windows console tests/probes 必须链接 folia_console_test_runtime；手工构建包含 tools/test-support/windows_console_runtime.cpp。CRT/abort 到 stderr；异常进程退出后停止广泛测试，先有界聚焦定位。

标准 VS 18 开发环境构建：

```bat
call "%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake --build build/core --parallel 4 --target FoliaMathTests
```

聚焦测试使用 positional pattern（统计须扣除 skipped）：

```bat
build\core\FoliaMathTests.exe "*name*"
```

生成数据/日志只进 ignored build 目录。小批、明确路径提交；禁止 reset worktree 或改写历史。

## 保护的既有 dirty 路径

以下是既有工作，不得顺手编辑、暂存或提交：

- `src/app-winui/media/MathJaxSvgParser.cpp`
- `src/app-winui/media/SvgNormalizer.cpp`
- `src/math/adaptors/lite/lite_module.cpp`
- `src/math/adaptors/lite/lite_module.ixx`
- `src/math/core/mml_tree/mml_context_bindings.cpp`
- `src/math/core/mml_tree/mml_context_bindings.ixx`
- `src/math/core/mml_tree/mml_nodes/mtable_bindings.cpp`
- `src/math/core/mml_tree/mml_nodes/semantics_bindings.cpp`
- `src/math/math.cpp`
- `tests/math/context_object_record_retirement.cpp`
- `tests/math/lite_array_view_request_retirement_g259.cpp`
- `tests/math/mml_private_renderer_allocation_domain_g227.cpp`
- `tools/math-port/native-raii-inventory.cjs`
