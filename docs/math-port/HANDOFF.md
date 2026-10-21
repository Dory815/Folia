# MathJax 原生移植交接入口

唯一当前 checkpoint。恢复或压缩上下文后先实际读本文件，再检查可能使结论失效的新改动。架构与文件职责见 [native-raii-architecture.md](native-raii-architecture.md)，验收及 Source fixture 见 [native-raii-acceptance.md](native-raii-acceptance.md)。只更新本文件，不另建进度笔记。

## 当前状态与下一步

- 无 live agent、build 或 test handles。用户已确认额度问题解除；旧 errored agents 属于历史任务，不重启旧审计。
- 工具修正 **b38c7627**、快照版本/生成器hash **a074b59f** 已提交：保留既有生成字体 resolver/13项自测，新增隔离 File API fixtures，总55 assertions通过；Root与独立Luna复验，包括从 C:\Windows 运行。缺失/坏配置、缺生成文件、stale inputs、module map差异不会被当成权威配置；默认输出 ignored，canonical manifest须显式指定。Root最终manifest断言通过，snapshotRevision=a074b59f且generator hash匹配当前工具；HEAD标记不等于宣称dirty工作树与HEAD相同。
- 清单已用该工具刷新，随本 checkpoint 提交。当前 `src/math` 728文件/724代码源；最新 File API index **2026-10-21T06-50-21-0412**：folia_math **560=313cpp+247ixx**，含83生成字体cpp；247模块声明与map完全一致，活跃未解析imports=0；bridge另属FoliaNativeMath；配置数学测试源157。File API只证明配置成员，mtime gate不证明全部源码未变。清单不再按文件名自动 remove/rewrite；Source注释仅索引，不是正确性或完整映射验收。
- 下一动作：核实旧Scene首批 **placements.{ixx,cpp}+13组wrappers配对，共28文件** 的职责/断言与tracked消费者后，小批退出。旧Scene50文件可按 importer-first 分为28/10/3/2/7，不能先删foundation留下悬空imports。保留8个配置Scene源及全部Source fixtures。Core旧Context层另有消费者；`core/tree/factory_types.ixx`是活跃typed服务，只import std，不能当Context绑定删除。
- ignored报告：`build/math-port/{inventory-reader-review,core-binding-retirement-review,old-scene-retirement-review,old-scene-menclose-review}.md`。Root已读。Menclose全部内置分支有Common实现与专门Source证据；动态Map/回调/对象投影语义已撤销。其他报告仅审阅材料，不等于已核实每个方法或可以自动删除。

## 已接受的实现和验收

管线：TeX → request-owned MML → 共享Common布局/placement → immutable Scene → SVG → Folia原SVG路径。保留MathJax原owner与数学算法；不模拟JS对象，不新建placement目录或平行native/new/typed管线，不堆请求释放补丁。Scene不保留MML/DOM/请求指针或回调；未来D2D消费同一Scene，无D2D像素门槛。Mermaid暂停。

- Renderer/C ABI及worker已接入，QuickJS数学调用/链接退出。DLL仅9个C exports。worker接入ba9ee591，可复验入口 `tools/math-port/winui-math-worker-probe.ps1`（7f38e4d9）。不得因清单旧数据重做已接受实现。
- 最近完整native **43545 exit0：869 tests/217108 assertions/175 suites，无skip**，`build/math-port/retire-svg-full-math.log` / `retire-svg-full-results.json`。旧61693是180s驱动timeout，不是通过；保留retire-svg-full-timeout180.*。后按原600s上限完整通过。
- 删除旧SVG18源（fab831cc）后parallel4 native build exit0/no work；支持的WinUI Debug+真实worker **8116 exit0**，`retire-svg-worker-verify.log`，stop join4423ms/total8.72s。未宣称Release或GUI视觉验收。更早平台80/3429、core455/40840通过；本轮仅改工具/清单，无新C++构建或native测试。
- hosted75布局/150完整SVG独立Source对照、UTF8/16与C ABI、75两路Scene几何、包/family专用witness、192+768请求MML/Common/DOM/CRT回基线均已接受。strictLite为73 equal+2 Source缺addEventListener但native成功；不宣称事件dispatch。细节见acceptance，别重复广审。
- 旧SVG图与相应测试/工具已分批退出，数学断言保留到真实owner测试。ignored历史候选仍有旧文本，不是当前模块；恢复找交付文件先用 `rg --no-ignore`。其他旧对象层尚未全部清完。

## 执行约束与既有改动

Root独占构建、native tests和Git；同一时间一个build，parallel≤4。异常进程退出后停止广测，先有界诊断。Console探针必须链接folia_console_test_runtime；C++23 modules使用import std。小批提交，不reset，不覆盖无关改动；生成物与日志留ignored build/math-port。

以下11条原dirty仍保护，不得顺手修改/暂存/提交。媒体2条normalized blob与HEAD相同，仅CRLF/stat状态。若确需退出保护bindings/tests，先逐diff明确account并备份原patch。

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

本轮inventory工具原dirty已由Root完整读diff、保留resolver与自测并明确account入b38c7627；原字节/patch在 ignored `build/math-port/inventory-fix-candidate/{before-accounting.cjs,prior-worktree.patch}`，不再列保护dirty。不要据此覆盖其余11条。
