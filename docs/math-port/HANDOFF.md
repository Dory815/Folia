# MathJax 原生移植：当前交接

恢复时先读本文件，再只检查可能推翻下列状态的新变化。架构、验收与文件职责分别见 [native-raii-architecture.md](native-raii-architecture.md)、[native-raii-acceptance.md](native-raii-acceptance.md)、[native-raii-files.json](native-raii-files.json)；已接受的细节和历史验证留在那里，不在此重复。这里只保留一个当前 checkpoint。

## 当前状态与下一步

- 最后运行验证的代码提交为 `186a1fd7`，清单提交为 `d1e9cbb7`；本轮清理提交 `2ea7d904`。现存 inventory 与 files 清单一致：168 个未配置 cpp/ixx、active unresolved imports 0、接口/map 248 一致。未配置不等于可删除。
- 本轮 parallel4 build 与15个聚焦测试/1733 assertions通过（`lite-coverage-check.json`）；补齐分式子节点类型、真实XMLNode序列化、公有Renderer结果/Scene存活和恢复断言。既有Source SVG/C ABI同时通过。上轮WinUI Debug worker PASS，数学QuickJS已退出；本轮未重跑worker/full math/full字体，未验Release/GUI。
- 9 个非媒体 dirty diff 已读；原字节/patch 保存在 `build/math-port/context-dirty-backup-20261021/`，九份原件哈希及补丁 reverse-check 通过。现已单独退役 `context_object_record_retirement.cpp` 的 15 个纯旧对象协议测试；其余保护路径不变。未删 Context/Lite/MML provider。
- **下一步**：四个Lite工具消费者已逐项归责，真实行为已补测通过；按 `lite-probes-retirement-review.md` / `lite-renderer-consumers-review.md` 和 `lite-retirement-check.cjs` 分批退役消费者、G259，再检查Lite module/store/text helper闭包。G227报告也已读，分式/复制与Renderer缺口已解决。全部报告/候选在ignored `build/math-port/`，无live agent/build。MML两处旧 `initializeFreshProperties` 无现存定义，不恢复旧投影API；MML provider仍有旧调用链，不能直接删。
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
