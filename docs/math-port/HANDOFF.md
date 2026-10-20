# MathJax 原生移植交接入口

当前：Unicode / Entities 已验收并分批提交，最近功能提交 `d348900a`。完整 math 测试 exit 0：706 tests、206757 assertions、136 suites，无失败/跳过。无活动 build 或测试进程。

下一步：移植 mhchem 原 configuration pair，复用并独立验证已有 `dependencies/mhchem_parser` 算法；先读架构中新冻结的 Mhchem 职责。只读候选说明 `build/math-port/mhchem-preparation.md` 不是验收结论，且其中“7 macros”计数有误：实际 9 commands（ce/pu + 7 arrows）、14 chars。

活动 Luna：`/root/mhchem_package` 只改原 configuration pair；`/root/mhchem_dependency` 负责原 parser 算法及 parser Source工具/fixture/UT；`/root/mhchem_package_cases` 只负责 mhchem Source工具/fixture/UT。Root 独占 CMake/build/Git。mhchem 未接入 CMake、未构建或验收。先收齐并核对 220 包级历史 inputs、63 replacements、14 mutations，以及依赖原 7960 条完整比较。replacement helper 取两个原 TokenMap 引用；不得用当前 registry map 代替 Source lexical maps。

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
