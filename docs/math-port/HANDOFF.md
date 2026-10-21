# MathJax 原生移植：当前交接

恢复时先读本文件，再只检查可能推翻下列状态的新变化。架构、验收与文件职责分别见 [native-raii-architecture.md](native-raii-architecture.md)、[native-raii-acceptance.md](native-raii-acceptance.md)、[native-raii-files.json](native-raii-files.json)；已接受的细节和历史验证留在那里，不在此重复。这里只保留一个当前 checkpoint。

## 当前状态与下一步

- 当前代码/清单快照 `01e584cb`：648源/644代码、162未配置cpp/ixx、561配置源、接口/map248一致、active unresolved imports0；File API `index-2026-10-21T10-48-30-0986.json`。未配置不等于可删除。
- WinUI worker已接入原生SVG，数学QuickJS退出；既有完整验证见acceptance。Lite层及此前6个MML消费者已小批退出，勿重审。原9个dirty字节与patch备份 `build/math-port/context-dirty-backup-20261021/` 已验。
- **已接受基础**：helper七补测/66断言；maction继承非string拒绝原owner修复 `be421fe0`，修后3/53+Source SVG1/1047；positions四旧test+六fixture工具已退出（`09d0fd01`/`7288e2e8`/`01e584cb`），独立Source exact check12978通过。证据见acceptance；不重复广审。class_binding剩57测试消费者。
- **本包停点**：mfenced非string `d6360942`、43初始flags `0a352237`、wrapper分类 `f7ff0c02` 已提交。前包三测试477断言通过；followup五文件/七测试parallel4 build exit0、七测试141断言通过。`class-followup-coverage-check.json`确认478活动源与6保护路径未变；本次提交nested Math及fixMmultiscripts两分支，剩HTML校验noop、直接/负arity替换两文件待提交。无live build/native/agent；未重跑full math/font/worker，未验Release/GUI。
- **精确下一包**：新三Source probe全部exit0，见 `script-wrapper-class-source-observations.json`/`mml-verification-source-observations.log`/`mml-html-replace-source-observations-v2.json`。HTML实际虚派发至empty-node mError noop，旧TypeError和merror/title推断均撤销，没有生产fix。Math/merror实际含inferred row，勿提升旧错误镜像。提交后评审小退场包 `class-consumer-exit-prep-3.json` 与 `class-retirement-mfenced-three.json`（仅boundaries/string_errors建议退出，class_length保持待定）；先处理工具引用，不凭未配置删文件。512/513 walk边界单独待审；旧fields/values/methods完整ContextBindings/ParserRuntime链不能孤删/stub。
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
