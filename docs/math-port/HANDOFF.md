# MathJax 原生移植交接入口

本文件是唯一当前 checkpoint。恢复先读本文件，再仅检查可能使这些结论失效的当前改动。架构与验收细节见 [native-raii-architecture.md](native-raii-architecture.md) 和 [native-raii-acceptance.md](native-raii-acceptance.md)。有实质进展后只更新本文件；历史由 Git、测试与 Source witness 保存。

## 当前状态与下一步

- merror已验收并小批提交：a95fcf3b（Scene Title/stylesheet/getters）、fdb42c27（writer）、0ee1f30c（原SVG owner/catalog接线及旧注册移除）、4fd31382（Source7/UT）。保护13未动。当前无活动build/测试；HTML批次Luna按下条分工，merror已freeze。
- 最新full `build/math-port/merror-full-math.log` exit0：790tests/212141assert/153suites，无skip。merror Source7 UT 1/30、default86 1/1021通过。canonical75 `merror-results.json`：72 SVG byte equal、1 HTML native exception、2 Source error/native exception、0 SVG mismatch。未达75/150 gate；两action Source错误仍为 `node.addEventListener is not a function`，不能算异常相同即通过。
- 当前HTML已在architecture末尾锁定接口/验收并授权六组Luna：common_radicals_emission=MML export/getter（已freeze）；scene_values_writer=owned HTML Scene/snapshot；common_rows_emission=Common XML及request measurement；common_scene_entry=SVG XML/catalog；common_base_emission=HTML serializer/writer；common_scene_source_tests=独立Source/UT及共享runtime可选XMLcapture。Root负责CMake、old placement/callback清退、canonical composition与统一build/tests/Git。只读proposal是候选，权威为最终Root lock。下一步review各owner交付，freeze后parallel4单build，再XML聚焦/旧Source fixtures no-op/default86/canonical75/full，尚未build本批。
- HTML迁移边界：Common原wrappers/xml_node pair保留font/HDW/measurement/bbox职责；SVG原xml_wrapper pair保留包装/发射职责；Scene原html_types/html_snapshot迁移成owned值，清除旧foundation Transform依赖，不复制HTML树。当前proposal声称可直接复用的serializer实际仍Property，不能导入active路径；需迁移对应算法到当前writer。XmlDomAdaptor已有typed opaque XmlRef，但没有DOM export/resolve；XMLNode也无adaptor getter。Root必须先定窄的原生request期接口，不补Context兼容桥或序列化再解析。HTML已按最终Root lock授权，遵守该接口，非旧proposal。
- merror Source fixture SHA256 `08ea0ed47413d39f5f43e40663342f52b89b932b3df45babb529341249a72647`；独立default/no-op与Root default已过。Source LiteParser.PCDATA包含title，特殊&<>原样、空Title自闭合；旧Group.tooltip不变。stylesheet保留class声明且不覆盖authored paint；完整CSS cascade仍单独gate。旧wrapper_factory_bindings不在CMake，后续清退，不补旧构造。
- 新UT曾访问异常：stack精准定位isCss，Boost.UT比较组合使空指针仍解引用；已改新UT显式空值分支，恢复DEBUG flags后Source7及full通过。日志 `merror-debug-stack.log`；自有PDB/诊断exe/obj已清理151.4MB。不能据该测试问题重开已验生产所有权。
- 已验composition修复见Git 2f8ed3be/0151394a/2b24e7e2/d7325313/c88df7d8：newcommand live-session marker、Font direct/overlay优先、Mo所有TextNode、连续TeX session+真实Lite测量、unknown text Source属性/逗号placement。禁止重复旧7mismatch审计。Source unknown width继承NodeMixin，非base零；复用typed LiteAdaptor，不重写分类/硬编码case bbox。
- Source TeX148 SHA256 `31761f077a57e92177b38c44dba0808b78e6616e99e1242f50058aafbe8de8ba`；table MathML8 `893e46e25dc8520157bb05c2918a4ad6b82a4c3ada71e169262955d5afb48e0f`；semantics/fenced11 `11d404fe4054172ee0ff87b355dee48f6e39dfe3e3a0cd9aade3959bc0e9984d`。default/no-op/retirement已过。fixtures constexpr flat，旧递归vector C1060不重试。initialAllowTexHTML/final/errors在common-source/common-scene-inputs.json，canonical连续session，不拿旧fresh候选作依据。每次compile仍新MmlDocument；default86 scheduling不变。
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
