# MathJax 原生移植交接入口

恢复或压缩上下文后先实际读本文件，再检查可能使结论失效的新改动。唯一当前 checkpoint；职责见 [architecture](native-raii-architecture.md)，证据与限制见 [acceptance](native-raii-acceptance.md)，配置事实见 [files](native-raii-files.json)。不另建进度笔记，不重复已接受广审。

## 当前状态与下一步

- 记录提交402a7fab；旧Scene50源分五批已退，只保留8源，真实算法留原Common。8fe4445b修正Mrow inferred fast path，cad58dcd保留两条border断言。**176a81f9**又退SvgMath/TimerHost4源；**f30168ca**退3纯Context协议测试/探针。详细职责/验收在acceptance，不恢复重复层。
- 现再退出Common/SVG factory binding六源+最后的getBBox协议探针（及其ignore例外）。Root读Luna方法图并核对实际声明/回调/typed owner/全部tracked直接API；7路径闭包无external imports/target成员。证据 `scene-commonFactory-before-removal.json`；typed factory/catalog/Common测量/linebreak/styles均保留。
- 最新完整math **96710 exit0：870 tests/217144 assertions/176 suites/0 skips**；真实WinUI Debug worker **64450 exit0**，stop join4599ms/total9.07s/empty stderr。日志 `retire-scene-{full-math,worker-verify}.log`、`retire-scene-full-results.json`。未验Release/GUI视觉。最新小批后parallel4 build **69747 exit0**：两个unconfigured tests退出触发CMake再生成，MathTests.exe/DLL前后hash相同、保护11字节不变，`factory-retirement-build{.log,-results.json}`。未重复full/worker。
- factory binding已提交 **45f03d9a**。canonical清单现已刷新至该HEAD与 **index-2026-10-21T08-28-56-0446.json**，Root断言668文件/664代码、183未配置cpp/ixx（MML64、TeX36、Core+tree/util58、font/common/svg15、Lite/html10）、560配置源/247map exact、保护11不变。mapping_data.inc/license另计且保留。
- 剩余图/owner报告均在ignored `build/math-port/remaining-*.{md,json}`。Root已读六初稿、修订稿、consumer queue及Common factory/Lite/MML leaf/font-BBox小包。TeX两版误认未配置runtime/test为active，已纠正；不直接接受旧ledger为当前gap，不按空membership或agent结论判删。旧图有被上述提交移除的路径，仅作历史依赖证据。
- **下一动作**：两个Luna的既有UT镜像已由Root核对Source并晋升（未改生产算法）。固定3处显式owned-key转换后parallel4 build exit0；Source display probe六观察exit0/empty stderr，含math.ts锁与额外deps hashes。6聚焦 **6 tests/78 assertions**通过（mml-leaf-focused-results.json），不是新full run。先提交这两个测试与记录，再按mml-leaf-retirement-review退出reference_leaf旧混合测试；真实数学分支已保留，动态callback/无效raw tree/tuple身份不迁回typed owner。当前无build/native或Luna运行。

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
