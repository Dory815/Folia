# MathJax 原生移植交接入口

本文件是唯一当前 checkpoint。恢复先读本文件，再仅检查可能使这些结论失效的当前改动。架构与验收细节见 [native-raii-architecture.md](native-raii-architecture.md) 和 [native-raii-acceptance.md](native-raii-acceptance.md)。有实质进展后只更新本文件；历史由 Git、测试与 Source witness 保存。

## 当前状态与下一步

- XML/HTML已验收并小批提交：421844e1、04dc48de、a9ef803e、0b347faf、7b950787；前批merror也已验收。maction架构先提交420c7112。保护13未动，Root负责build/native/Git，parallel4。
- maction静态无缓存批已验收提交：e389abf5 Common offsets/shared subtree Scene、e07b3b16 Scene值、0fe4fb0c writer、51ada53b 原SVG owner/旧桥清退、970cff98 独立EventTarget Source10/UT。最终format后`maction-final-build.log` exit0，Source UT1/92通过：7 none SVG byte-equal/owner退役后同字节，3 local数学tooltip明确Unsupported、未当parity。Common4/34+subtree1/7、Scene4/32、SVG owner5/43、writer3/8聚焦过。最新full `maction-full-math.log` exit0：818tests/212525assert/162suites，无skip。所有build/native已结束，所有Luna freeze，无live handles。
- 最新canonical75 `maction-results.json`/`maction-native.log`正常exit1：73 SVG byte equal、2 source_error_native_success（action-toggle/action-tooltip），无SVG mismatch或native exception；严格Lite Source仍 `node.addEventListener is not a function`，不能算完整75/150通过。前批XML full801/212330/157、XML11 1/46、merror7 1/30、default86 1/1021均已验，本次完整回归包含它们，不重开旧审计。
- 当前local cache未验收：Root已在architecture末尾锁resource_order（Builder生成顺序、Scene每scope唯一）与原font_cache owner的FontCachePlan（临时borrow、按scope/Glyph地址映射）；writer只消费已分配的完整prefix列表，不推测/自分配ID。四Luna均freeze：scene_values_writer=Scene/Builder/order UT，common_scene_entry=font_cache pair/plan UT，common_base_emission=writer+旧writer caller迁移，common_scene_source_tests=Source真实prefix trace/追加cases/SourceUT。Root已接CMake、清退inactive FontCache旧桥callers、review/format；parallel4 build live exec session76686，日志`cache-build.log`。完成后聚焦order/plan/writer/Source14→full/分批commit。Mermaid暂停，完整gate/Folia/旧层退出未完成。
- 新Source14 fixture SHA `e00941e648aae4ce257f227b89c6f9e76974a4d3ac49c83987ff6708ff5e4fa1`，尚未native验收。Root `cache-source-original.cjs`对Git175ecabf确认旧10条完整SVG字节未变，新增multi-segment local、empty fixed prefix、empty glyph/defs、nested none；所有prefix来自真实FontCache.useLocalID观察，不推测ID。Source owner default/write/no-op已过，Root独立default待运行。
- maction Source fixture SHA `2a7d669d7f723a5a713fd9302b9fde021181107f45162671db390ca7211b4cee`；Root独立`maction-source-root-final.log`通过10。raw `maction-scene-source.json`记录actual/requested display：MathML.compile按root display属性重写MathItem.display，toggle-line-segments实际false，不能拿convert参数true做native配置。Source SVG输入/字节没改。Common offset临时string ternary→string_view悬空已修为先显式view，Source relative-ex .431，2ex=.862。Title同级在rect前、无tip rect自闭合。
- nested cache证据`maction-nested-cache-source.json`/`maction-writer-design.md`：Source递归processMath替换活动defs，返回clearLocalID后外层后继glyph定义落nested defs且无prefix；fixed localID可实际重复id。local数学tip明确UnsupportedError，不能当SVG通过；多own segments不能简单按Group独立递归cache。Scene只owned数据/Action，没有JS callbacks/timers；SVG输入静态，无frontend dispatch实现声明。
- XML fixture最终SHA `037d34d9035c1441ea34ce350dbf7da7d1f2e73067cdc59b3ed39203253bc58d`，Root独立default `xml-source-root-final.log`、owner write/default/no-op及旧table/semantics/merror no-op均过。原生Src11数据和SVG未因修CPP更改。Source实际math.start=body、parent=html、metrics.family未设→Lite Times；output scale2时metrics.scale仍1。HTML geometry保留CSS px及em_per_pixel，Scene只owned树，native Common测真实nodeBBox/异常RAII清理，SVG不测量。font_scale有限非负，Source widthfit可0；em_per_pixel严格正。
- 本批必须记住的已定坑：Root wrapper不在nodeMap，取job.build返回或实际wrapper.parent；processIndent默认width是Source metrics.containerWidth（Wrapper.ts1245），不能用未设置的linebreak containerWidth。新UT曾因临时std::array转span导致AV，fixture已用命名namespace-scope constexpr observations；不要再生成悬空span。Boost.UT作用域内pointer/bounds guard必须显式分支，不能把dereference藏在组合比较的右边。无DEBUG flags/PDB遗留；旧诊断临时文件已清。
- 原inactive SVG output_jax仍Property，保留旧materialized/Serializer调用不代表active接口；最终旧层退出时清退，不补兼容constructor/Context桥。旧scene/xml_node pair、XmlSvgNodeCreator/caller及merror placement已清退；旧wrapper_factory_bindings未接CMake。XML annotation使用原xml_wrapper owner的SvgAnnotationXML（无独立布局），Common按Source普通wrapper发射。全部新接口在architecture末尾Root lock，不回到只读proposal。
- merror fixture SHA `08ea0ed47413d39f5f43e40663342f52b89b932b3df45babb529341249a72647`。LiteParser.PCDATA含title，特殊&<>原样、空Title自闭合；旧Group.tooltip不变。stylesheet与authored颜色分开，merror排除link/viewport/foreignObject；完整CSS cascade仍独立gate。newcommand live-session marker、Font direct/overlay优先、Mo所有TextNode、canonical连续TeX session/真实Lite测量、unknown text Source属性/逗号placement都已验收，不重审旧修复。
- Source TeX148 SHA `31761f077a57e92177b38c44dba0808b78e6616e99e1242f50058aafbe8de8ba`；table8 `893e46e25dc8520157bb05c2918a4ad6b82a4c3ada71e169262955d5afb48e0f`；semantics/fenced11 `11d404fe4054172ee0ff87b355dee48f6e39dfe3e3a0cd9aade3959bc0e9984d`。default/no-op/retirement已验。fixtures用constexpr flat，旧递归vector C1060不重试。initialAllowTexHTML/final/errors在common-source/common-scene-inputs.json；canonical连续session，每次compile新MmlDocument，default86 scheduling不变。Source unknown宽度继承NodeMixin，复用typed LiteAdaptor，不用base零或自造case bbox。
## 继续有效的 Source 验收边界

- 实际full38/Newcm limits witness已重建并提交ac8270e1，common-limits-source-current-root.log通过，3个concrete Source native tests17assert通过。旧手工叠mixin零offset不是验收标准，不重开该问题。

可复用的 frozen Luna handles（仅Root可派followup；不是仍在运行）：
- `common_radicals_emission`：MML XML export/getter；`scene_values_writer`：Scene值/Builder/HTML snapshot。
- `common_rows_emission`：Common XML/measurement/scale；`common_scene_entry`：SVG XML/merror/catalog。
- `common_base_emission`：SVG writer/HTML serializer；`common_scene_source_tests`：独立Source/fixture/UT。
- `common_scripts_emission`：scriptbase/msubsup/munderover/mmultiscripts/menclose既有emitter；旧limits/multiscripts handles不并行改同文件。Root拥有CMake/build/tests/Git。其它旧handles为历史，不据完成消息重开工作。
- Unicode/Entities/mhchem 原 owner 已完成组件验收；texhtml 的既有 Source/native 证据已接受。组件验收不代表 75/150 gate 或 Folia 集成。
- Source标准catalog为43个native NodeKind；menclose无profile时throw，有profile支持全部29种notation；XML/HTML由SVG catalog提供原生专用wrapper，Common默认catalog仍abstract。entry限定核对Source有效class metadata：Common字段均匹配commonDefaults；六组不同的SVG CSS归SVG原owner，不复制到Common，无scale字段。没有把该核对当作完整catalog渲染验收。
## 实现约束与未完成项

Common→Scene Root contract 见架构文档 2026-10-21 的 Common-to-Scene 与 Script-family 段。保持统一 em/y-up；SceneContainers.nodes 是 own 输出，content 给 children；断行 Mrow 是单 outer 加多个 lineboxes。输出SceneNodes现为vector<optional<NodeId>>，content为valid SceneParents；默认place用child自身dx/id且跳缺失首段，显式target不额外叠加；Rule 不反转。

Inline fragments已验收：移动line entries到shell副本仅留一棵树；finalization优先cached lineBBox（非stack getLineBBox返回全row，不能替代cache），按scale/originalL/forcebreak处理。Translation逐operation保存分隔符。SVG local defs：single在rootg之前；inline只第一svg在rootg之后。suppressed Mo立即discard并留optional槽。getBaseCore用isKind含inferredMrow；Mo普通offset正u。

新viewport契约见架构Completed viewports段：Group末尾optional<Viewport>，bounds/view_box均em/y-up，aspect可选；Glyph末尾Transform；Scaling.explicit_axes仅序列化。Builder split必须保留viewport值；writer直接nested svg与path/use transform，禁止额外g。Mo一个viewport对应一个Source svg，不重复clip/scale节点。

仍待实现/验证：table完整profile覆盖；mglyph image、maction及其余专用families。XML/HTML foreignObject已按本批Source11验收，CommonXmlNode已是typed native active owner，不重做。完整75-case layout、150条SVG byte comparison、请求所有权、Folia集成、旧对象层退出均未完成。

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
