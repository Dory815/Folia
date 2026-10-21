# MathJax 原生移植交接入口

本文件是唯一当前 checkpoint。恢复先读本文件，再仅检查可能使这些结论失效的当前改动。架构与验收细节见 [native-raii-architecture.md](native-raii-architecture.md) 和 [native-raii-acceptance.md](native-raii-acceptance.md)。有实质进展后只更新本文件；历史由 Git、测试与 Source witness 保存。

## 当前状态与下一步

- 最新提交：e4b0c1fa（canonical discovery）、d86c8455（hitbox顺序）、13439044（契约）、2f8ed3be（newcommand live session初始化）、0151394a（FontData direct/overlay glyph优先）、2b24e7e2（Mo所有TextNode发射）、d7325313（canonical连续TeX session/Lite测量接线）、c88df7d8（unknown text Source属性与位置）。后五批已验收。保护13路径未动。
- 最新已完成full unknown-text-full-math.log exit0：785tests/212089assert/152suites，无skip；前一轮canonical-composition-full-math.log为785/212087，也已通过。newcommand16tests/160、Font优先级1/6、Mo5/45、default86 1/1021均通过。新UT曾错误使用EnvironmentMap继承的MacroMap::sessionOwner（非实际environment owner），Root改验证contains/真实定义读写；literal delimiter用let alias "("，不再把langle command alias误当delimiter。生产无需为UT加accessor。
- 当前canonical75：67 SVG byte equal、6 native exceptions（5个merror，1个abstract CommonXmlNode/HTML）、2 Source error/native exception，0 SVG mismatch，仍不算75/150 gate通过。unknown-text-native.log正常exit1，矩阵unknown-text-results.json；上批64矩阵canonical-composition-results.json保留。最初57/7mismatch/9exception已被后两批修复推进，不重开旧7差异。
- 最新已提交c88df7d8仅writer.cpp/native_svg_writer.cpp：unknown_text按Source data-variant/transform/font-size/CSS family/style/weight输出，-explicitFont与已完成empty-family payload不发CSSfont；不重复UTF8/SMP判断。Root补未知text有origin时Source逗号translate；独立Source两例a漢/a😀b actual translate(500,0)证据unknown-text-offset-source.json。writer1test/4、default86 1/1021、canonical75矩阵均通过。build unknown-text-final-build.log exit0/parallel4；full已通过。无活动build/test，所有Luna freeze。
- 下一动作：Root先锁merror架构再分工，尚未授权merror/HTML实现。Source无Common Merror mixin；SvgMerror职责是standard own root背景rect（data-background,width,height,y，无x）→可选title→children。现有旧scene/wrappers/merror pair为Property projection，须在原owner迁移/退役；拟按原SVG class/factory职责注入Common catalog，禁止新增CommonMerror或CommonWrapper源字符串特例。当前Scene有序entries不含title值，Group.tooltip总在entries前；Paint不能同时保留CSS有效色与Source不发fill/stroke的表现。Root先决定最小值契约/样式处理，再让Luna实现，不能偷塞raw SVG或复制布局。HTML旧html_types/snapshot还依赖旧foundation Transform，不能与当前scene.types同名Transform直接import。
- Source独立证据：TeX148 Root bytecheck及此前default/no-op/原115 unchanged，SHA256 31761f077a57e92177b38c44dba0808b78e6616e99e1242f50058aafbe8de8ba；initialAllowTexHTML/final flags/errors在common-source/common-scene-inputs.json。table MathML8 SHA256 893e46e25dc8520157bb05c2918a4ad6b82a4c3ada71e169262955d5afb48e0f；semantics/fenced MathML11 SHA256 11d404fe4054172ee0ff87b355dee48f6e39dfe3e3a0cd9aade3959bc0e9984d。后两default/no-op/retirement已通过，Root verifier structural-mathml-source-verify.cjs。fixtures用constexpr flat，旧递归vector C1060不重试。
- 已定调查边界：Source LiteAdaptor继承NodeMixin，unknown宽度不是base零；复用已验typed LiteAdaptor，不重写分类或硬编码每case bbox。Source corpus连续session，历史candidates/profile75-full-svg-g147 fresh组不是canonical依据；units-bevelled须匹配连续session。TexHTML初值false且由输入启用/恢复，不拿最终options当初值。独立MathML/table/enclosure/semantics组件通过不代表75/150、所有权、Folia集成。

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
