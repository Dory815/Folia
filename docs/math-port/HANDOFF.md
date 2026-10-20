# MathJax 原生移植交接入口

本文件是唯一当前 checkpoint。恢复先读本文件，再仅检查可能使这些结论失效的当前改动。架构与验收细节见 [native-raii-architecture.md](native-raii-architecture.md) 和 [native-raii-acceptance.md](native-raii-acceptance.md)。有实质进展后只更新本文件；历史由 Git、测试与 Source witness 保存。

## 当前状态与下一步

- 最近代码提交 `0e91715c`；本批table/Scene/entry/menclose/Source验收已分7批提交（起点3d0f24f5）。最终build `common-table-final-build.log` exit0，parallel4；完整math `common-table-final-full-math.log` exit0：773tests/211281assert/150suites。无活动build/test。
- 独立Source112已由Root bytecheck（`table-source-root-bytecheck.log`）；fixture SHA256 `bbb73b2a93b5069dc9e7b20b0ca77bdcc760941a4d7fab138e8144daadb6c018`。native 48/48 byteequal（`table-source-final-focused.log` exit0）。Source48聚焦567assert；7个retirement cases/14assert也包含在上述完整测试中。
- 生产owner全部freeze。CommonMenclose沿用原pair/geometry，只开放line recipes与radical child；29notation SVG profile由composition传入standardNodes(span)，Common不import SVG，其余shape明确throw。Table首border path的Source width由Path.layout_width承载；solid用原anchor，broken用当前first-child，详见architecture末段。
- Luna全部freeze；`common_radicals_emission`已完善ignored `tools/math-port/drafts/native-raii/table-direct-mml/` 8-case Source/原生测试候选。另Source owner已freeze的3-case enclosure候选在 `tools/math-port/drafts/native-raii/enclosure-scene-cases/`。用 `rg --no-ignore` 找；均未native验收，不可把candidate消息当pass。
- 下一步：Root审查并提取direct-MML packet到现有table测试职责，比较完整SVG；再补menclose3条实际TeX对照。equalrows/columns/dotted未验收；Source mtr mathsize="150%" rscale仍1，mstyle仅content rscale1.5，不得混称row缩放。
## 现有验证边界（此前已完成）

- 此前full math766tests/211042assert/150suites，包含35Source cases与4个重复请求case；不是完整75/150渲染gate。
- 此前Source witness102 records的Root bytecheck通过：`padded-source-root.log`；默认生成工具不读取native输出。102不是最终75/150 gate。
- 最新viewport-review/report.json的59 Scene/writer/limits/scripts tests477assert全过；随后namespace/split补充改动复验viewport值9assert、writer13、split16，lifetime8均过，日志`assembly-*.log`。28Source全部匹配，不代表全profile。
- 实际full38/Newcm limits witness已重建并提交`ac8270e1`，`common-limits-source-current-root.log`通过；3个concrete Source native tests17assert通过。旧手工叠mixin零offset不是验收标准。


owner handles（未注明活动者均freeze，只有Root派followup）：
- `scene_values_writer`：Scene 5 files、SVG writer、两份原 UT。
- `common_base_emission`：Wrapper、TextNode、TeXAtom；`common_mo_emission`：Mo。
- `common_rows_emission`：Mrow、Mtext、Math、Mphantom、Mtr/Mtd；`common_radicals_emission`：Mfrac、Msqrt、Mroot、Mpadded、Mtable。
- `common_scene_entry`：OutputJax、WrapperFactory；`common_scene_source_tests`：Source tool、fixture、UT。
- `common_scripts_emission`：当前统一拥有scriptbase/msubsup/munderover/mmultiscripts及menclose emitter后续；旧limits/multiscripts owner不要并行改同文件。
- Root 拥有公共契约、CMake、build/tests、Git；不可假设上述 agent 仍在运行。
- Unicode/Entities/mhchem 原 owner 已完成组件验收；texhtml 的既有 Source/native 证据已接受。组件验收不代表 75/150 gate 或 Folia 集成。
- Source标准catalog为43个native NodeKind；menclose无profile时throw，有profile仅line/radical；XML/HTML仍throw。entry限定核对Source有效class metadata：Common字段均匹配commonDefaults；六组不同的SVG CSS归SVG原owner，不复制到Common，无scale字段。没有把该核对当作完整catalog渲染验收。
## 实现约束与未完成项

Common→Scene Root contract 见架构文档 2026-10-21 的 Common-to-Scene 与 Script-family 段。保持统一 em/y-up；SceneContainers.nodes 是 own 输出，content 给 children；断行 Mrow 是单 outer 加多个 lineboxes。输出SceneNodes现为vector<optional<NodeId>>，content为valid SceneParents；默认place用child自身dx/id且跳缺失首段，显式target不额外叠加；Rule 不反转。

Inline fragments已验收：移动line entries到shell副本仅留一棵树；finalization优先cached lineBBox（非stack getLineBBox返回全row，不能替代cache），按scale/originalL/forcebreak处理。Translation逐operation保存分隔符。SVG local defs：single在rootg之前；inline只第一svg在rootg之后。suppressed Mo立即discard并留optional槽。getBaseCore用isKind含inferredMrow；Mo普通offset正u。

新viewport契约见架构Completed viewports段：Group末尾optional<Viewport>，bounds/view_box均em/y-up，aspect可选；Glyph末尾Transform；Scaling.explicit_axes仅序列化。Builder split必须保留viewport值；writer直接nested svg与path/use transform，禁止额外g。Mo一个viewport对应一个Source svg，不重复clip/scale节点。

仍待实现/验证：table完整属性覆盖；mglyph image、Xml/HTML foreignObject及其余专用families。`CommonXmlNode`仍是Property，不得导入active路径。完整75-case layout、150条SVG byte comparison、请求所有权、Folia集成、旧对象层退出均未完成。

## 继续有效的既定边界

- Lite 六个原文件是同一 `lite_adaptor` primary 的 partitions。Xml payload 冻结为独立子树；CompileServices 只借用 output/adaptor，compile 正常或异常结束后都清理。
- Lite parsed `document.defaultView` 为空，`window.document` 才指向 window。HtmlNode 对有效 Element 缺 marker 时不包装 span。FindTeX Ref/eqref 显式 `display=false`。
- direct fixture 输入取 `initialAllowTexHTML`，不能把 `jax.options` 最终观测当初值。这些已有测试证据，不重开。
- Source profile 固定 MathJax 4.1.3、38 个 TeX packages；Source `.reference/MathJax-src` commit `fb987178c2d279a99b0db5ea02e751691703955e`；compiled hash `cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a`。profile 为 `tools/math-port/reference-profile.cjs`，scope 为 `docs/math-port/scope.json`。
- 总体目标是 TeX→MML→共享 Common 布局→不可变 Scene→SVG→Folia 替换 math QuickJS。Mermaid 暂停，无 D2D 像素 gate。Common 辅助 MML 归每次请求的 MmlDocument；同 document 重测可增长到析构，最终 renderer 每 request 新建 document，session/Scene 不保留 document。

## 工作与构建规则

保留 MathJax 原 owner 与已验算法；不建 `native2`/`new`/`typed` 平行实现，不模拟 JS 对象，不堆释放补丁。Source fixture 独立由 pinned Source 生成，不能改期望迎合 C++。Node heap 512、cwd `C:/Windows`、每个进程一次 `createReference`。默认 Luna，必要时仅 GPT-6.1 Sol。Root 管 CMake、build/tests、Git；一次一个 build，parallel 总数最多 4。

Windows console tests/probes 链接 `folia_console_test_runtime`；手工构建含 `tools/test-support/windows_console_runtime.cpp`。CRT/abort 写 stderr。异常进程退出后停止广泛测试，先有界聚焦定位。标准 VS 18 命令：

```bat
call "%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake --build build/core --parallel 4 --target FoliaMathTests
```

聚焦测试用 positional pattern：`build\core\FoliaMathTests.exe "*name*"`。日志与生成数据放 ignored `build/math-port`。小批、明确路径提交；不 reset worktree 或改写历史。

## 保护的既有 dirty 路径

不得顺手修改、暂存或提交以下 13 条路径：

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
