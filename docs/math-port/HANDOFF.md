# MathJax 原生移植：当前交接

恢复时先读本文件，再只检查可能推翻下列状态的新变化。架构、验收与文件职责分别见 [native-raii-architecture.md](native-raii-architecture.md)、[native-raii-acceptance.md](native-raii-acceptance.md)、[native-raii-files.json](native-raii-files.json)；已接受的细节和历史验证留在那里，不在此重复。这里只保留一个当前 checkpoint。

## 当前状态与下一步

- 当前代码/清单快照 `5bd8b49c`：648源/644代码、162未配置cpp/ixx、561配置源、接口/map248一致、active unresolved imports0。清单据最新File API；未配置不等于可删除。
- 本轮补齐唯一inferred-row虚复制：真实TextNode、属性独立、源树不变；原有测试文件 `mml_native_factory.cpp` 新增一项。parallel4 build及11测试/1989断言通过（`build/math-port/mml-copy-consumer-check.json`），含完整Source SVG。478活动源和6保护路径字节不变。既有WinUI worker已接入原生SVG，数学QuickJS退出；本轮未重跑full math/full字体/worker，未验Release/GUI。
- Lite对象门面/store/反射textContent及消费者12路径已分6批退出，G227也已单独退出；数学算法、原typed DOM、Source oracle保留。原9个dirty文件及patch在 `build/math-port/context-dirty-backup-20261021/`，已校验。候选已吸收进三个既有测试，不要用旧镜像覆盖。
- **精确停点**：copy补测已提交 `ac063e20`；allocator/node-fields/methods draft三消费者已分别退至 `43ed535d`。childPosition逻辑槽位补测也已吸收到既有 `mml_typed_table.cpp`，build75046 exit0、1测试/12断言通过（`mml-position-consumer-*`）。提交此补测后退factory-fields/values共三个文件。Root已读对应报告，勿重审已退消费者。无live agent/build。
- 窄图 `mml-current-consumer-graph.json` 的三个已退消费者边待末尾刷新；class_binding仍有61测试消费者，values仍由旧factory_context_fields实现调用，methods仍被ContextBindings/ParserRuntime用，不能直接删provider。两处旧 `initializeFreshProperties` 无定义，不恢复旧投影API。
- 搜索被忽略的交付候选时用 `rg --no-ignore`。不要按旧 agent 的完成消息推断当前状态，也不要重做已接受的广审。保护 dirty 不是永久保留理由，但处置前必须读 diff、备份并归责。

## 固定架构与执行边界

保留原 MathJax 的模块职责和数学算法：TeX → request-owned MML → 原 Common 共享布局与 placement → immutable Scene → SVG → Folia 现有 SVG 路径。移除 JS 对象身份、反射和投影语义，不等于删除数学算法。禁止平行布局/Scene 层、重复实现或无依据删算法；新模块须有明确职责、消费者及被替代的原 owner。Mermaid 移植暂停，勿碰其 normalizer/Rust 依赖。

Root 独占 Git、CMake/configure、build 和 native 测试；同一时间仅一个 build，parallel 不超过 4。Windows console probe 链接 `folia_console_test_runtime`。异常进程退出后停止扩大测试，只做有界诊断。小批提交，不 reset，不覆盖无关改动；生成物和日志留在 ignored build 目录。详细架构和验收标准见上述文档。

## 保护中的 6 个 dirty 路径

处置前必须逐 diff 阅读、备份原字节与 patch 并明确归责；不得顺手覆盖或暂存。

- `src/app-winui/media/MathJaxSvgParser.cpp`
- `src/app-winui/media/SvgNormalizer.cpp`
- `src/math/core/mml_tree/mml_context_bindings.cpp`
- `src/math/core/mml_tree/mml_context_bindings.ixx`
- `src/math/core/mml_tree/mml_nodes/mtable_bindings.cpp`
- `src/math/core/mml_tree/mml_nodes/semantics_bindings.cpp`
