# MathJax 原生移植交接入口

唯一当前 checkpoint。恢复或压缩上下文后先实际读本文件，再检查可能使结论失效的新改动。架构与文件职责见 [native-raii-architecture.md](native-raii-architecture.md)，验收及 Source fixture 见 [native-raii-acceptance.md](native-raii-acceptance.md)。只更新本文件，不另建进度笔记。

## 当前状态与下一步

- 无live agent/build/test。71657 build exit0，后续仅test TU重编也通过parallel4。新mrow-origin **1/10**、原explicit break **1/14**、family Source-SVG **1/1021**，`retire-scene-focused-results.json`及对应日志。11保护文件/2binary旧hash在`scene-batch-before.json`。Luna报告均完成，Root已读。
- 工具修正 **b38c7627**、快照版本/生成器hash **a074b59f** 已提交：保留既有生成字体 resolver/13项自测，新增隔离 File API fixtures，总55 assertions通过；Root与独立Luna复验，包括从 C:\Windows 运行。缺失/坏配置、缺生成文件、stale inputs、module map差异不会被当成权威配置；默认输出 ignored，canonical manifest须显式指定。Root最终manifest断言通过，snapshotRevision=a074b59f且generator hash匹配当前工具；HEAD标记不等于宣称dirty工作树与HEAD相同。
- 清单已用该工具刷新，随本 checkpoint 提交。当前 `src/math` 728文件/724代码源；最新 File API index **2026-10-21T06-50-21-0412**：folia_math **560=313cpp+247ixx**，含83生成字体cpp；247模块声明与map完全一致，活跃未解析imports=0；bridge另属FoliaNativeMath；配置数学测试源157。File API只证明配置成员，mtime gate不证明全部源码未变。清单不再按文件名自动 remove/rewrite；Source注释仅索引，不是正确性或完整映射验收。
- Mrow窄差异已由Source+native聚焦收口：own CSS的unbroken inferred dx=0，broken stack局部origin=1.125；仅程序化MML状态。新Source oracle exit0，JSONL/stderr在ignored build/math-port。sceneDx改virtual非const，CommonMrow覆写fast path为0，无新状态/helper。最终测试复用math预置inferred子项；broken控制加入真实operand以避免被识别成embellished operator。早期不匹配fixture的日志保留retire-scene-mrow-style-{initial-fixture,world-control,embellished-fixture}.log，均不是通过结果。
- 已提交：**8fe4445b** Mrow语义修正及Source/UT；**d9222851** 旧registry+13families=28源；**3d9a966d** script/limits重复10源；menclose3源随后独立提交（见git log）。Root逐批确认无configured/剩余tracked imports，before-removal JSON在ignored build/math-port/scene-{scripts,menclose}-before-removal.json；28源完整hash/consumer图为scene-batch-graph-review.json。
- Border有效断言已接入原CommonScene测试；Source generator仅追加2项，Root比对确认此前148 records全部byte-identical（首75固定）。parallel4 build与扩展88-case聚焦 **1/1047**通过，border-coverage-native.log/results.json。2旧Transform候选CPP及其1条ignore例外随本轮coverage提交退出；独立border/texttip Source oracle/fixture/comparer保留。下一步退最后glyph2/support7源（先精确closure check），保留8 active源；然后一次full math+worker及清单刷新。Core factory_types仍活跃。
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
