# MathJax 原生移植交接入口

当前：Lite/Xml/HTMLDomStrings/texhtml/Finder 组件已验收并分批提交：4c379ef7（DOM/Lite）、c9d76a1f（Xml bridge）、ef04091d（CompileServices/Finder）、f0f4aaef（HTMLDomStrings/texhtml，删除旧 methods/services 四文件及旧 UT）。最后 build texhtml-final-build.log exit 0；完整 texhtml-full.log exit 0：741 tests / 210529 assertions / 149 suites，无失败/跳过。工作区仅剩末尾原13个保护 dirty 路径。无活动 agent/build/test handles；额度已恢复。

下一步：Root 先在现 architecture 文件锁定 Common→Scene emission 的实际 C++ 接口/文件职责/验收，再改声明并分 Luna。需解决：CommonWrapper 多行 segments/后置 place 与 Builder transform/wrap/prepend 能力；Scene 缺 group paint/opacity、unknown TextRun 字体语义；CommonMo assembly 只有 bbox，尚无 glyph piece placements。这些是新出口缺口，不重写已验的 BBox/字体/分式根式算法。现 CommonMfrac/Msqrt/Mroot::layoutGeometry 可直接复用。SVG writer 只做拼写、路径/cache、数值格式，不选字形、不计算数学位置。

固定边界：六个 Lite 原文件是同一 lite_adaptor primary 的 partitions。Xml payload 冻结独立子树；CompileServices 只借用 output/adaptor，本次 compile 后正常/异常均清理。Lite parsed document.defaultView 为空，window.document 才指 window；HtmlNode 对有效 Element 缺 marker 不包装 span；FindTeX Ref/eqref 显式 display=false。direct fixture 输入取 initialAllowTexHTML（不能拿 jax.options 最终观测当初值）。这些已测试，不重开。

证据：Lite Source 41 records / 30 parser inputs，native 11 tests/95 asserts；Xml 5/48；Finder Source 22 inputs/59 ProtoItems，native 3/1330；texhtml 22 direct + recovery + 5真实pipeline，native 9/182，五份原 snapshot 精确匹配。Root 从 C:/Windows 的三个 *-source-current-root.log 全 exit 0；canonical tools/fixtures 自包含。完整 Source JSON/fixture hashes 在工具和验收文档，不重复抄历史日志。

本轮只读 Luna common_scene_contract、scene_paint_contract、glyph_emission_contract 已结束：旧 scene/wrapper、scene/wrappers/*、glyph_paint 都不在 active CMake，不能注册回来。CommonTextNode 只有 remappedText() 方法，不是存好的 glyph 输出；composition/unknown buffering 与实际 stretch piece 数学须回原 CommonWrapper/TextNode/Mo owner。Source place 区分默认 dom[0]+dx/id 调整与显式 segment；多行 parents[] 不可简化成单 origin。Scene metadata 可保留 SVG 拼写，但不能替代后端共用的 paint/font 语义。

需要续用时 followup 原 Luna owners（lite_parser、lite_native_contract、mhchem_package、html_dom_strings、mhchem_package_cases、mhchem_dependency、texhtml_tests）；新阶段写任务需 Root 先锁接口。旧 Finder hosts 有 inactive parser_runtime/render_session 入边，随旧对象层退出清理；node_store/source_text_content 仍被保护 dirty Lite consumers 引用，暂不删。旧 render_session 不补兼容 overload。

候选在 ignored build/math-port/{lite-source-candidate,texhtml-source-candidate,find-tex-source-candidate}/，当前 canonical 工具不依赖它们。Root 独占 build/test/Git/CMake/HANDOFF；无未诊断异常退出。上次 texhtml UT 的空 bad.root 已改为读取 compile 返回值并经聚焦/完整验证。

恢复时先读本文件，再检查相关当前改动。架构见 [native-raii-architecture.md](native-raii-architecture.md)，验收见 [native-raii-acceptance.md](native-raii-acceptance.md)。只更新这一个当前 checkpoint；历史交给 Git、测试和 Source witness，不追加阶段流水账。

## 目标与固定输入

完整目标仍是 MathJax 4.1.3、固定 38 个 TeX 包：TeX→MML→共享 Common 布局→不可变 Scene→SVG→Folia，替换 math QuickJS。整体未完成；Mermaid 暂停，没有 D2D 像素 gate。

- Source：`.reference/MathJax-src`，commit `fb987178c2d279a99b0db5ea02e751691703955e`
- CJS：`tools/mathjax-bundle/node_modules/@mathjax/src/cjs`
- Full38 compiled hash：`cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a`
- Profile/pins：`tools/math-port/reference-profile.cjs`、`docs/math-port/scope.json`

## 已接受组件与剩余边界

Unicode/Entities/mhchem 原 owner 已完成 Source 和 native 组件验收，记录见 native-raii-acceptance.md 与 Git；不重复全域审查、FunctionList 命名或已移除的 JS API 兼容测试。

顺序：已完成 texhtml → Common→Scene 实际发射 → SVG 全量对照 → Folia 集成 → 旧对象层退出。不可把包级测试当成完整 SVG/Folia 验收。

Common 测量辅助 MML 属于本次请求 MmlDocument；同 document 重测可能增长至 document 析构。最终 renderer 每 request 新建 document，session/Scene 不保留 document。

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
