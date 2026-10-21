# MathJax 原生移植交接入口

本文件是唯一当前 checkpoint。恢复先读本文件，再检查可能使当前结论失效的改动。架构与职责见 [native-raii-architecture.md](native-raii-architecture.md)，验收与 Source fixture 细节见 [native-raii-acceptance.md](native-raii-acceptance.md)。更新状态只改本文件，不另建进度笔记。

## 当前 checkpoint

当前无live build/test；tracked worker probe runner **57717 exit0**（WinPS5.1实际返回0，stop join4575ms/total8.91s），`worker-reproducible-probe.log`；worker接入 **ba9ee591**。WinUI Debug **24161 exit0**；实际app对象worker probe exit0（mathbb/frac/Unicode/noundefined/Formula/recovery/cache/Clear/disable-reenable；stop join4454ms）；平台 **78599 exit0**，80/3429/17 suites；core **95496 exit0**，455/40840/50 suites，均无skip。部署DLL SHA与core一致0FB98CA1ADD2BB9D1BEA9315A03F31917C6FA40CDA73F173D0B990B5762CFFFC。下一步提交probe/验收，再审退休候选；生产/测试只读清单在ignored `retirement-{production,test}-inventory.md`，Root尚未批准删除。

- 原生API full **4060** exit0：860 tests/216708 assertions/174 suites无skip，`renderer-owner-full-math.log`；12专项全过，精确数见acceptance。原Renderer/CABI DLL/API tests **a08e9723**，验收/worker架构锁 **12b62b30**；247 active modules无旧对象层，DLL仅9个Cexports。

- limits full **78558** exit0：848 tests/215119 assertions/171 suites无skip；Source包 **d688445e**、UT **73ae4eef**。Source24条1/97、原数值6/34、hosted1/1279通过，Root default/write-no-op/旧numeric过。新fixtureSHA bb52168574af0b70a7f63a6dcff0c5bff05f9c5ca6922cdb10f76527abafc636。

- 2f76e389 锁定、af9a8891 实施 ParseRequest/TeXRequest：保留 ParseOptions 配置/session；请求数据进入 caller-owned frame。CompileScope 恢复 MathItem/adaptor 借用，新增生产文件为 0。physics 两个 creator probe 的局部借用先结束再换文档；build-physics exit0、专项8/5373过。
- 稳定构建 parse-request-build-fixed2.log exit 0；formatter 已完成。
- 请求 ownership 门槛 parse-request-ownership.log exit 0：1 test / 11 assertions。MML/Common/DOM 计数、真实 Debug CRT blocks/bytes、owner 退出后的 retained Scene 均通过。诊断 session 11188 已终止。
- 专项结果：scope 1/11，tex-jax 5/551，texhtml 9/182，textmacros 12/868；hosted 75 layout / 150 Scene / 150 SVG 1/1279；default 86 1/1021，均通过。
- 本次 strict canonical 已复验：73 equal 和 2 个 source_error_native_success，原因是 Lite 缺 addEventListener；wrapper exit 0 表示类别符合预期，native 本身 exit 1。需保留此差异事实。
- 当前请求门槛替代了旧的 +5 blocks / 144 bytes 失败结果：当时持久 ParseOptions 留有请求 scratch，现已移入 caller-owned frame。

## 目标与边界

目标管线：TeX → request-owned MML → 共享 Common layout/placement → immutable Scene → SVG → Folia 现有 SVG 路径。Mermaid 暂停；无 D2D 像素门槛。SVG 不重算数学布局，Scene 不持有 MML、DOM、请求指针或回调。

架构已定：保留 MathJax 原 owner、算法和消费方边界；不模拟 JS 对象，不建平行 native/new/typed 实现，不加请求释放补丁。完整约束及各组件证据以架构和验收文档为准。组件通过不等于 Folia 集成。

38包及专用family已有Source映射，不凭scope陈旧状态重复实现。Renderer lock在architecture末尾；已确认canonical75实际tag_side全空、em16/ex8、width四值，全部可直接用现API验原fixture，无新Source/隐藏选项。原math.cpp dirty已由Root读diff/account，旧callback/projection退休随退出Context图结束，resident诊断随旧probe退出；备份与原patch在ignored `build/math-port/renderer-owner-candidate/math.cpp-before-integration` / `math.cpp-prior-work.patch`。只有math.cpp经明确原owner重写授权退出保护名单，其余12条仍保护。API/CABI及WinUI build/worker probe已通过；磁盘旧对象层尚未退出。

worker lock在architecture末尾；全部Luna已freeze ignored `build/math-port/worker-integration-candidate/` 三份接线镜像和 `winui-math-worker-probe.cpp`(194行)。Root加stop已请求时跳过native初始化、格式化新增backend并保留原queue/cache；真实probe仅include实际class，必须链接app生成的MathJaxRenderer/MathJaxSvgParser对象及queue/std必要对象与CABI importlib，手动编译加windows_console_runtime.cpp。Debug复用build/core，Release build/core-release；实际app对象已生成并用于probe，含pch/std/queue对象；手动build成功日志worker-probe-build.log。

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
