# MathJax 原生移植交接入口

本文件是唯一当前 checkpoint。恢复先读本文件，再检查可能使当前结论失效的改动。架构与职责见 [native-raii-architecture.md](native-raii-architecture.md)，验收与 Source fixture 细节见 [native-raii-acceptance.md](native-raii-acceptance.md)。更新状态只改本文件，不另建进度笔记。

## 当前 checkpoint

当前无 live build/test/agent。Full session **53985** exit0：`parse-request-full-math.log`，847 tests / 215022 assertions / 170 suites，无skip。请求所有权迁移及调用方已提交 **af9a8891**；生命周期测试/CMake提交 **2a8b888a**。下一步按下述具体 Source 对照包推进，不重跑已过全套。

- 2f76e389 锁定、af9a8891 实施 ParseRequest/TeXRequest：保留 ParseOptions 配置/session；请求数据进入 caller-owned frame。CompileScope 恢复 MathItem/adaptor 借用，新增生产文件为 0。physics 两个 creator probe 的局部借用先结束再换文档；build-physics exit0、专项8/5373过。
- 稳定构建 parse-request-build-fixed2.log exit 0；formatter 已完成。
- 请求 ownership 门槛 parse-request-ownership.log exit 0：1 test / 11 assertions。MML/Common/DOM 计数、真实 Debug CRT blocks/bytes、owner 退出后的 retained Scene 均通过。诊断 session 11188 已终止。
- 专项结果：scope 1/11，tex-jax 5/551，texhtml 9/182，textmacros 12/868；hosted 75 layout / 150 Scene / 150 SVG 1/1279；default 86 1/1021，均通过。
- 本次 strict canonical 已复验：73 equal 和 2 个 source_error_native_success，原因是 Lite 缺 addEventListener；wrapper exit 0 表示类别符合预期，native 本身 exit 1。需保留此差异事实。
- 当前请求门槛替代了旧的 +5 blocks / 144 bytes 失败结果：当时持久 ParseOptions 留有请求 scratch，现已移入 caller-owned frame。

## 目标与边界

目标管线：TeX → request-owned MML → 共享 Common layout/placement → immutable Scene → SVG → Folia 现有 SVG 路径。Mermaid 暂停；无 D2D 像素门槛。SVG 不重算数学布局，Scene 不持有 MML、DOM、请求指针或回调。

架构已定：保留 MathJax 原 owner、算法和消费方边界；不模拟 JS 对象，不建平行 native/new/typed 实现，不加请求释放补丁。完整约束及各组件证据以架构和验收文档为准。组件通过不等于 Folia 集成。

覆盖登记已核对：38个TeX包均有Source witness；tables/mglyph/maction/merror/XML/semantics/mfenced有专用完整SVG证据。下一具体包：在原 `common-limits-source-witness.cjs` / `native_common_limits.cpp` 职责内补 munder/mover/munderover 的直接MathML完整SVG与请求退出后对照（现有数值和canonical例不重写）。scope旧状态/锁定fixture不为改进度而改。其后仍需应用 Renderer/C ABI 原 owner 改造及 Folia 接入、旧对象层退出；75/150不代表集成。

## 执行约束

- Root 独占 build、native tests 与 Git；parallel 不超过 4，同一时间只跑一个 build。
- 异常进程退出后停止广测，先做有界诊断。C++23 modules 使用 import std;；Windows console tests 使用 folia_console_test_runtime。
- 日志与生成物放 ignored build/math-port。不 reset worktree，不覆盖无关更改；小批提交。
- Source fixture 及精确 hash、profile 范围、聚焦断言和固定 commit 记录在验收文档；Source fixture 独立生成，不为迎合 C++ 改 expected。

## 保护的既有 dirty 路径

以下 13 条路径必须完整保留，不得顺手修改、暂存或提交：

- src/app-winui/media/MathJaxSvgParser.cpp
- src/app-winui/media/SvgNormalizer.cpp
- src/math/adaptors/lite/lite_module.cpp
- src/math/adaptors/lite/lite_module.ixx
- src/math/core/mml_tree/mml_context_bindings.cpp
- src/math/core/mml_tree/mml_context_bindings.ixx
- src/math/core/mml_tree/mml_nodes/mtable_bindings.cpp
- src/math/core/mml_tree/mml_nodes/semantics_bindings.cpp
- src/math/math.cpp
- tests/math/context_object_record_retirement.cpp
- tests/math/lite_array_view_request_retirement_g259.cpp
- tests/math/mml_private_renderer_allocation_domain_g227.cpp
- tools/math-port/native-raii-inventory.cjs

