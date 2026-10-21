# MathJax 原生移植：当前交接

恢复时先读本文件，再只检查可能推翻下列状态的新变化。架构、验收与文件职责分别见 [native-raii-architecture.md](native-raii-architecture.md)、[native-raii-acceptance.md](native-raii-acceptance.md)、[native-raii-files.json](native-raii-files.json)；已接受的细节和历史验证留在那里，不在此重复。这里只保留一个当前 checkpoint。

## 当前状态与下一步

- 最后运行验证的代码提交为 `186a1fd7`，清单提交为 `d1e9cbb7`。现存 inventory 与 files 清单一致：168 个未配置 cpp/ixx、active unresolved imports 0、接口/map 248 一致。未配置不等于可删除。
- 已验 native parallel4 build、9 个聚焦测试/26,340 assertions、WinUI Debug worker PASS，数学 QuickJS 路径已退出。本轮只做静态审阅；未重跑 build/test，未验 Release/GUI 视觉。
- 9 个非媒体 dirty diff 已读；原字节/patch 保存在 `build/math-port/context-dirty-backup-20261021/`，九份原件哈希及补丁 reverse-check 通过。现已单独退役 `context_object_record_retirement.cpp` 的 15 个纯旧对象协议测试；其余保护路径不变。未删 Context/Lite/MML provider。
- **下一步**：Root 读 `build/math-port/{lite-dirty-disposition,mml-dirty-disposition}.md`，核实真实行为的现有测试后，按消费者顺序小批处置。Luna 三份报告已完成，无 live agent/build。MML 报告发现两处旧 `initializeFreshProperties` 调用没有现存定义；不要恢复旧投影 API。不能直接删 provider，仍有旧 runtime/MML()/classBinding 调用链。
- 搜索被忽略的交付候选时用 `rg --no-ignore`。不要按旧 agent 的完成消息推断当前状态，也不要重做已接受的广审。保护 dirty 不是永久保留理由，但处置前必须读 diff、备份并归责。

## 固定架构与执行边界

保留原 MathJax 的模块职责和数学算法：TeX → request-owned MML → 原 Common 共享布局与 placement → immutable Scene → SVG → Folia 现有 SVG 路径。移除 JS 对象身份、反射和投影语义，不等于删除数学算法。禁止平行布局/Scene 层、重复实现或无依据删算法；新模块须有明确职责、消费者及被替代的原 owner。Mermaid 移植暂停，勿碰其 normalizer/Rust 依赖。

Root 独占 Git、CMake/configure、build 和 native 测试；同一时间仅一个 build，parallel 不超过 4。Windows console probe 链接 `folia_console_test_runtime`。异常进程退出后停止扩大测试，只做有界诊断。小批提交，不 reset，不覆盖无关改动；生成物和日志留在 ignored build 目录。详细架构和验收标准见上述文档。

## 保护中的 10 个 dirty 路径

处置前必须逐 diff 阅读、备份原字节与 patch 并明确归责；不得顺手覆盖或暂存。

- `src/app-winui/media/MathJaxSvgParser.cpp`
- `src/app-winui/media/SvgNormalizer.cpp`
- `src/math/adaptors/lite/lite_module.cpp`
- `src/math/adaptors/lite/lite_module.ixx`
- `src/math/core/mml_tree/mml_context_bindings.cpp`
- `src/math/core/mml_tree/mml_context_bindings.ixx`
- `src/math/core/mml_tree/mml_nodes/mtable_bindings.cpp`
- `src/math/core/mml_tree/mml_nodes/semantics_bindings.cpp`
- `tests/math/lite_array_view_request_retirement_g259.cpp`
- `tests/math/mml_private_renderer_allocation_domain_g227.cpp`
