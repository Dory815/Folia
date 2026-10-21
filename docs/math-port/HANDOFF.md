# MathJax 原生移植交接入口

恢复或压缩上下文后先实际读本文件，再检查可能使结论失效的新改动。唯一当前 checkpoint；职责见 [architecture](native-raii-architecture.md)，证据与限制见 [acceptance](native-raii-acceptance.md)，配置事实见 [files](native-raii-files.json)。不另建进度笔记，不重复已接受广审。

## 当前状态与下一步

- HEAD **57a153f3**。旧 Scene 50 源已分五批退出，只剩 builder/scene/types/html_types/html_snapshot 共8源；真实算法留在原 Common owner。8fe4445b 修正 CommonMrow inferred fast path；cad58dcd 将两条 border 数学断言保留到既有 CommonScene。提交与 Source 细节在 acceptance，不恢复旧重复层。
- 最新完整 math **96710 exit0：870 tests/217144 assertions/176 suites/0 skips**；支持的 WinUI Debug+真实 worker **64450 exit0**，stop join4599ms/total9.07s/empty stderr。日志：`build/math-port/retire-scene-{full-math,worker-verify}.log`、`retire-scene-full-results.json`。删除后 parallel4 native build exit0/no work。未验 Release/GUI 视觉。
- canonical 清单已刷新至 **57a153f3**、File API **index-2026-10-21T08-13-34-0100.json**：678文件/674代码；560配置源（含83生成字体cpp）、247接口与module map相等、active imports未解析0。清单仅配置事实，Source注释仅索引，不按文件名/空membership判删。工具b38c7627/a074b59f的55项自测与隔离cwd复验已接受。
- **下一动作**：先提交本轮记录/清单，再从 ignored `build/math-port/remaining-object-retirement-queue.md` 选择小消费者批次，Root实际读候选内容和direct API，保留mixed数学断言后逐批退图。剩余193个未配置cpp/ixx：MML64、TeX36、Core+tree/util58、font/common/svg25、Lite/html10；mapping_data.inc及license另计并保留。不得只凭未配置或agent结论删除。
- 只读材料在 `build/math-port/remaining-{object-graph,mml-owners,tex-owners,tree-util-owners,font-common-owners,lite-html-owners}.md`（graph另有json）。Root已读六报告；graph仅事实。树/util报告的active归类、TeX旧ledger缺口及MML保护名单有错误，正在精确纠正，未接受为删除依据。旧Scene报告仅历史before-removal证据。
- 当前无build/native运行。活跃Luna：remaining_object_graph（新index及小消费者队列）、remaining_tree_util_owners（配置/测试事实纠错）、remaining_tex_owners（实际owner与旧ledger交叉核对）、remaining_mml_owners（保护名单与小批队列）；只写ignored报告。不依据早期agent完成消息覆盖此checkpoint。

## 已接受决策

TeX → request-owned MML → 共享 Common 布局/placement → immutable Scene → SVG → Folia原SVG路径。保留 MathJax 原模块职责与算法；不模拟JS对象，不建平行 native/new/typed/placement 管线，不堆释放补丁。Scene无MML/DOM/请求指针和回调。未来D2D直接消费同一Scene，无D2D像素门槛；Mermaid暂停。

Renderer/C ABI仅9个C exports；worker线程拥有renderer，取消/结果RAII，保留队列/cache/generation；QuickJS数学调用/链接已退出。可复验入口 `tools/math-port/winui-math-worker-probe.ps1`。已接受 hosted75布局/150完整SVG、UTF8/16及C ABI、75两路Scene几何、字体14568049字段/24646glyph/40动态模块、192+768请求回基线。strictLite为73 equal+2 Source缺addEventListener而native成功，不宣称事件dispatch。详见acceptance，不重新广审。

## 执行约束与既有改动

Root独占build/native/Git，同一时间一个build、parallel≤4；console探针链接folia_console_test_runtime。异常退出即停广测、有界诊断。小批提交，不reset、不覆盖无关改动；生成物/日志留ignored。恢复找交付候选先 `rg --no-ignore`。不按名称删 active factory_types、js_number/js_numeric_input 等真实类型/数值算法。

以下11条原dirty仍保护，最新逐字节校验未变；hash在 `build/math-port/scene-batch-before.json`。媒体2条仅CRLF/stat状态，normalized blob同HEAD。确需处理其中旧bindings/tests时，Root先逐diff明确account、备份原字节与patch，再单独决定，不能顺手覆盖/暂存。

- src/app-winui/media/MathJaxSvgParser.cpp
- src/app-winui/media/SvgNormalizer.cpp
- src/math/adaptors/lite/lite_module.cpp
- src/math/adaptors/lite/lite_module.ixx
- src/math/core/mml_tree/mml_context_bindings.cpp
- src/math/core/mml_tree/mml_context_bindings.ixx
- src/math/core/mml_tree/mml_nodes/mtable_bindings.cpp
- src/math/core/mml_tree/mml_nodes/semantics_bindings.cpp
- tests/math/context_object_record_retirement.cpp
- tests/math/lite_array_view_request_retirement_g259.cpp
- tests/math/mml_private_renderer_allocation_domain_g227.cpp

inventory工具原dirty已保留resolver/自测并account入b38c7627；备份 `build/math-port/inventory-fix-candidate/{before-accounting.cjs,prior-worktree.patch}`，不再列保护dirty。旧869测试/8116 worker等状态均已被上述新验收取代，历史细节留acceptance。
