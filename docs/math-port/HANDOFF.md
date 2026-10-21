# MathJax 原生移植：当前交接

恢复时先读本文件，再只检查可能推翻下列状态的新变化。架构、验收与文件职责分别见 [native-raii-architecture.md](native-raii-architecture.md)、[native-raii-acceptance.md](native-raii-acceptance.md)、[native-raii-files.json](native-raii-files.json)；已接受的细节和历史验证留在那里，不在此重复。这里只保留一个当前 checkpoint。

## 当前状态与下一步

- 代码/清单快照 `2b547b6d`：648源/644代码、162未配置cpp/ixx、561配置源、接口/map248一致、active unresolved imports0；File API `index-2026-10-21T11-41-32-0150.json`。窄图已刷新，class_binding直接importer为生产2+测试50。未配置不等于可删除。
- WinUI worker已接入原生SVG，数学QuickJS退出；完整既有验证及历史提交见acceptance，勿重复广审。原9个dirty备份 `build/math-port/context-dirty-backup-20261021/` 已验；下列6路径仍保护。
- **最新已验**：class-state Source-only `cd959200` 194观测通过、52旧有限outcomes保留；registry `f13b3e4f` 桥核实旧期待值尾段后转存JSON，43keys/172构造/1849关系/35协议观测通过；verification `1b01ff26` 原profiles72+18 exact通过。证据 `class-source-only-check.json`。这些JS协议观测仅为Source来源记录，不重建native对象协议。
- 历史清理验收：`d40acdef`/`8ec3261c`/`2b547b6d`与build63913、`class-source-retirement-final-check.json`已接受，native3/477及7/141保留。base legacy gate仍保留required primary boundary fixture；旧runner均未跑。勿把历史的二进制hash用于新测试build。
- **最新停点**：BaseNode `5acd4e70` native **1/55通过**；constructor88/container6480原Source exact通过，container转Source-only后再次exact通过。工具退场 `39ab4396`；两Cpp已删除待分批提交，removal build57997 exit0。`constructor-container-final-check.json`确认478活动源、6保护、17targets、9不同已验输入/二进制（10entry含script重复）、6保留文件不变；直接class_binding测试50→48。无生产算法更改。
- **精确下一动作**：分批提交两Cpp；HTML bridge首次执行发现52是递归emitter graph槽数、仅29 distinct顶层deep值，尚未写profile/canonical，Luna正在修正29/52来源记录与桥验证，Root须复审后重跑。positions单一rebind consumer尚未Source检查/删除。无live build/native，operations Luna静态工作中；不可按候选语法通过声明验收。
- HTML最终替换现有html-node-oracle.cjs，不能新增并列owner；带emitCpp的observer仅ignored迁移证明。replaceChild/512边界仍保留。positions建议/覆盖见 `positions-retirement-next/positions-retirement-next.json`，不按旧agent消息推断已执行。
- **仍待单独归责**：mfenced class_length的自定义callback数组变更及512/513 walk边界均保留待审；现有原生family未显示classification期间改变所属mfenced children。Fields/values/methods仍有完整ContextBindings/ParserRuntime owner链（见窄图），不能孤删/stub。其余候选按既有八组报告推进。
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
