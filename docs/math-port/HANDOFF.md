# MathJax 原生移植交接入口

本文件是唯一当前 checkpoint。恢复先读本文件，再仅检查可能使这些结论失效的当前改动。架构与验收细节见 [native-raii-architecture.md](native-raii-architecture.md) 和 [native-raii-acceptance.md](native-raii-acceptance.md)。有实质进展后只更新本文件；历史由 Git、测试与 Source witness 保存。

## 当前状态与下一步

- 本批代码已提交：d900333d（shared test helper/table）、5ef8c8ac（shared Source runtime/table）、3bdd8a18（Source11）、3cdaa2f4（semantics/mfenced原pair）、ef9ce49c（UT/CMake）；本批代码与工具已提交。此前enclosure已6批提交至e52d30b1。无活动build/test，所有Luna freeze。
- 最新full math semantics-fenced-full-math.log exit0：782tests/212129assert/152suites，无skip。Source semantics4/17assert、mfenced7/14（均含retirement）、empty-null semantics2tests/8、parent恢复2tests/15、原table8/184均通过。此前enclosure selected86/1107、retirement12/24、geometry56 exact也已验收。无活动build/test，owner均freeze。
- 独立Source：TeX148 default/no-op/原115 unchanged，SHA256 31761f077a57e92177b38c44dba0808b78e6616e99e1242f50058aafbe8de8ba；table MathML8 SHA256 893e46e25dc8520157bb05c2918a4ad6b82a4c3ada71e169262955d5afb48e0f，重构后仍byte/no-op unchanged；semantics/fenced MathML11 SHA256 11d404fe4054172ee0ff87b355dee48f6e39dfe3e3a0cd9aade3959bc0e9984d，default/no-op通过。Root复核脚本在ignored build/math-port/structural-mathml-source-verify.cjs。fixtures必须constexpr flat，旧递归vector曾C1060，不重试。
- 已验收table rows/cells/labels/solid/dashed/dotted/equalrows/equalcolumns及全部29 menclose notations+4边界、semantics/mfenced遍历。XML annotation payload仍未支持。MathML编译器是独立锁定Source输入，不算TeX profile覆盖；mtr mathsize150%仍rscale1，mstyle content1.5。旧ignored table/enclosure候选均被canonical替代，不恢复。
- 下一步：先扩展现有Common Scene对照到canonical75的真实执行矩阵，再按实际失败锁定剩余owner任务，勿盲目重写。Root只读coverage inventory在ignored build/math-port/canonical-scene-coverage.json：当前86 selected仅含canonical75中的17条，58条尚未纳入（不等于58条不支持）；11 direct-MathML与额外focused不能替代75/150 gate。新probe须保留Source expected errors、每case真实输入flags（尤其initialAllowTexHTML既定边界），不以fixture错误改期望。之后再决定maction/mglyph/foreign与CSS/C ABI/Folia顺序。上轮test接线问题已修：glyph writer需字体catalog；root wrapper用build返回，不在nodeMap；semantics inherit按arity补空row，直接空/null分支显式wrapper输入。

## 继续有效的 Source 验收边界

- 实际full38/Newcm limits witness已重建并提交ac8270e1，common-limits-source-current-root.log通过，3个concrete Source native tests17assert通过。旧手工叠mixin零offset不是验收标准，不重开该问题。

owner handles（未注明活动者均freeze，只有Root派followup）：
- `scene_values_writer`：Scene types/validation及Scene UT、共享MathML test helper/table复用。
- `common_base_emission`：SVG writer/UT与Wrapper、TextNode、TeXAtom；`common_mo_emission`：Mo。
- `common_rows_emission`：Notation pair/UT与Mrow、Mtext、Math、Mphantom、Mtr/Mtd、Mfenced pair/UT；`common_radicals_emission`：Mfrac、Msqrt、Mroot、Mpadded、Mtable。
- `common_scene_entry`：OutputJax、WrapperFactory、Semantics pair/UT；`common_scene_source_tests`：Source tool、fixture、UT。
- `common_scripts_emission`：当前统一拥有scriptbase/msubsup/munderover/mmultiscripts及menclose emitter后续；旧limits/multiscripts owner不要并行改同文件。
- layout_fixture_cleanup拥有canonical geometry comparer/selftest；Root 拥有公共契约、CMake、build/tests、Git；不可假设上述 agent 仍在运行。
- Unicode/Entities/mhchem 原 owner 已完成组件验收；texhtml 的既有 Source/native 证据已接受。组件验收不代表 75/150 gate 或 Folia 集成。
- Source标准catalog为43个native NodeKind；menclose无profile时throw，有profile支持全部29种notation；XML/HTML仍throw。entry限定核对Source有效class metadata：Common字段均匹配commonDefaults；六组不同的SVG CSS归SVG原owner，不复制到Common，无scale字段。没有把该核对当作完整catalog渲染验收。
## 实现约束与未完成项

Common→Scene Root contract 见架构文档 2026-10-21 的 Common-to-Scene 与 Script-family 段。保持统一 em/y-up；SceneContainers.nodes 是 own 输出，content 给 children；断行 Mrow 是单 outer 加多个 lineboxes。输出SceneNodes现为vector<optional<NodeId>>，content为valid SceneParents；默认place用child自身dx/id且跳缺失首段，显式target不额外叠加；Rule 不反转。

Inline fragments已验收：移动line entries到shell副本仅留一棵树；finalization优先cached lineBBox（非stack getLineBBox返回全row，不能替代cache），按scale/originalL/forcebreak处理。Translation逐operation保存分隔符。SVG local defs：single在rootg之前；inline只第一svg在rootg之后。suppressed Mo立即discard并留optional槽。getBaseCore用isKind含inferredMrow；Mo普通offset正u。

新viewport契约见架构Completed viewports段：Group末尾optional<Viewport>，bounds/view_box均em/y-up，aspect可选；Glyph末尾Transform；Scaling.explicit_axes仅序列化。Builder split必须保留viewport值；writer直接nested svg与path/use transform，禁止额外g。Mo一个viewport对应一个Source svg，不重复clip/scale节点。

仍待实现/验证：table完整profile覆盖；mglyph image、Xml/HTML foreignObject及其余专用families。`CommonXmlNode`仍是Property，不得导入active路径。完整75-case layout、150条SVG byte comparison、请求所有权、Folia集成、旧对象层退出均未完成。

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
