# MathJax 原生移植交接入口

本文件是唯一当前 checkpoint。恢复先读本文件，再检查可能使当前结论失效的改动。架构与职责见 [native-raii-architecture.md](native-raii-architecture.md)，验收与 Source fixture 细节见 [native-raii-acceptance.md](native-raii-acceptance.md)。更新状态只改本文件，不另建进度笔记。

## 当前 checkpoint

当前无live agent/build/test。native full **43545 exit0**：869 tests/217108 assertions/175 suites无skip，retire-svg-full-math.log/results.json。删除后parallel4 native build exit0/no work；支持的WinUI Debug+真实worker **8116 exit0**，retire-svg-worker-verify.log（stop join4423ms/total8.72s）。旧61693是180s驱动timeout而非pass，保留retire-svg-full-timeout180.*；后按原600s上限完整通过。

本轮关键提交：deb2b2f4保留fixInitialMO/maction多行/length/过滤器priority；254e0860保留public会话行为1/23；b4a0a410退旧Scene测试；c8ea9201保留75两路几何及Source150SVG(profile1/1354)、XMLfamily1/6。15旧测试及31 tracked tools CPP已分批退出，独立Source oracles/inputs/locks/fixtures保留。**fab831cc** 移除旧SVG9模块对18文件；**4383d220** 完整验收及ignore例外清理。before-removal graph retire-svg-graph.json证明这些模块不在全部configured targets/active247且无剩余tracked Src/tests/tools imports；ignored历史候选仍有同名文本，不是当前模块。其他旧对象层尚未清完。

下一步：用CMake fileAPI准确更新既有inventory的facts，再按Core/TeX/旧Scene等职责逐批account其余inactive文件和实际消费者；不要重做已通过广审/算法。旧inventory脚本漏foreach112 sources，勿覆盖canonical；正确facts候选在ignored current-inventory-candidate/configured-target-facts.json（target560 sources/247模块）。6个Luna及后续只读报告均完成，ignored retire-svg-*为审阅材料而非自动删除授权。12保护dirty继续保留；媒体2文件Root刚核normalized blob hash与HEAD相同，仅CRLF/stat状态，无需迁移其代码。下一批如要退出其他保护的旧bindings/tests，须先逐diff明确account并保留原patch，不能顺手删。

默认worker接入ba9ee591、可复验probe入口7f38e4d9；平台80/3429/core455/40840此前通过，未改编辑器。原a><b typed迁移3159d12c与字符串协议退休2e09ac91已完成。

- 原生API full **4060** exit0：860 tests/216708 assertions/174 suites无skip，`renderer-owner-full-math.log`；12专项全过，精确数见acceptance。原Renderer/CABI DLL/API tests **a08e9723**，验收/worker架构锁 **12b62b30**；247 active modules无旧对象层，DLL仅9个Cexports。

- 组件与Source witness的完整旧结果在acceptance，不重复恢复。已接受caller-owned ParseRequest/TeXRequest；真实MML/Common/DOM及CRT基线192+768请求通过，旧+5 blocks/144bytes scratch问题已修。strict Lite仍73 equal+2 source_error_native_success（缺addEventListener），hosted75/150独立通过；不宣称browser事件dispatch。

## 目标与边界

目标管线：TeX → request-owned MML → 共享 Common layout/placement → immutable Scene → SVG → Folia 现有 SVG 路径。Mermaid 暂停；无 D2D 像素门槛。SVG 不重算数学布局，Scene 不持有 MML、DOM、请求指针或回调。

架构已定：保留 MathJax 原 owner、算法和消费方边界；不模拟 JS 对象，不建平行 native/new/typed 实现，不加请求释放补丁。完整约束及各组件证据以架构和验收文档为准。组件通过不等于 Folia 集成。

38包及专用family已有Source映射，不凭scope陈旧状态重复实现。Renderer lock在architecture末尾；已确认canonical75实际tag_side全空、em16/ex8、width四值，全部可直接用现API验原fixture，无新Source/隐藏选项。原math.cpp dirty已由Root读diff/account，旧callback/projection退休随退出Context图结束，resident诊断随旧probe退出；备份与原patch在ignored `build/math-port/renderer-owner-candidate/math.cpp-before-integration` / `math.cpp-prior-work.patch`。只有math.cpp经明确原owner重写授权退出保护名单，其余12条仍保护。API/CABI及WinUI build/worker probe已通过；磁盘旧对象层尚未退出。

## 执行约束

- Root 独占 build、native tests 与 Git；parallel 不超过 4，同一时间只跑一个 build。
- 异常进程退出后停止广测，先做有界诊断。C++23 modules 使用 import std;；Windows console tests 使用 folia_console_test_runtime。
- 日志与生成物放 ignored build/math-port。不 reset worktree，不覆盖无关更改；小批提交。
- Source fixture 及精确 hash、profile 范围、聚焦断言和固定 commit 记录在验收文档；Source fixture 独立生成，不为迎合 C++ 改 expected。

## 保护的既有 dirty 路径

以下 12 条路径必须完整保留，不得顺手修改、暂存或提交：

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
- tools/math-port/native-raii-inventory.cjs
