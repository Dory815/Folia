# MathJax 原生移植：当前交接

恢复时先读本文件，再只检查可能推翻下列状态的新变化。架构、验收与文件职责分别见 [native-raii-architecture.md](native-raii-architecture.md)、[native-raii-acceptance.md](native-raii-acceptance.md)、[native-raii-files.json](native-raii-files.json)；已接受的细节和历史验证留在那里，不在此重复。这里只保留一个当前 checkpoint。

## 当前状态与下一步

- 当前代码/清单快照 `c5b4853e`：648源/644代码、162未配置cpp/ixx、561配置源、接口/map248一致、active unresolved imports0；最新File API `index-2026-10-21T10-07-55-0680.json`。未配置不等于可删除。
- 两项真实行为已补进原测试：唯一inferred-row虚复制（真实TextNode、独立属性、源树不变），以及跨inferred-row的逻辑childPosition。先后parallel4 build通过，11测试/1989断言（含完整Source SVG），再1测试/12断言通过。既有WinUI worker已接入原生SVG，数学QuickJS退出；本轮未重跑full math/full字体/worker，未验Release/GUI。
- Lite对象门面/store/反射textContent及消费者12路径已分6批退出，G227也已单独退出；数学算法、原typed DOM、Source oracle保留。原9个dirty文件及patch在 `build/math-port/context-dirty-backup-20261021/`，已校验。候选已吸收进三个既有测试，不要用旧镜像覆盖。
- **精确停点**：补测提交 `ac063e20`、`3454c4e0`；6个旧消费者分5批退至 `c5b4853e`。清理后parallel4构建session60121 exit0；478活动源、6保护路径、17 target sourcePaths、248接口/map均未变，测试/二进制与通过版本一致。inventory/canonical及窄图已刷新；证据 `build/math-port/mml-consumer-retirement-final-check.json`。无live build/native/agent；不要重复已退消费者审查。
- **下一包**：逐断言审查class_binding现存61个测试消费者；并厘清旧factory_context_fields的唯一values调用与ContextBindings/ParserRuntime导入边，之后才能退provider。窄图 `build/math-port/mml-current-consumer-graph.json/.md` 已更新。methods仍由上述两个Context owner使用。两处旧 `initializeFreshProperties` 无定义，不恢复旧投影API。
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
