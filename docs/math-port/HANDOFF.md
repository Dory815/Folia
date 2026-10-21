# MathJax 原生移植：当前交接

恢复时先读本文件，再只检查可能推翻下列状态的新变化。架构、验收与文件职责分别见 [native-raii-architecture.md](native-raii-architecture.md)、[native-raii-acceptance.md](native-raii-acceptance.md)、[native-raii-files.json](native-raii-files.json)；已接受的细节和历史验证留在那里，不在此重复。这里只保留一个当前 checkpoint。

## 当前状态与下一步

- 代码/清单快照 `9bb90408`：648源/644代码、162未配置cpp/ixx、561配置源、接口/map248一致、active unresolved imports0；File API `index-2026-10-21T11-19-15-0988.json`。窄图已刷新，class_binding直接importer为生产2+测试54；未配置不等于可删除。
- WinUI worker已接入原生SVG，数学QuickJS退出；完整既有验证及历史提交见acceptance，勿重复广审。原9个dirty备份 `build/math-port/context-dirty-backup-20261021/` 已验；下列6路径仍保护。
- **最新已验**：两包focused 3测试/477断言、7测试/141断言通过（mfenced类型、43 flags、wrapper分类、nested Math、multiscript、HTML、replaceChild），均已小批提交。HTML虚派发empty-node mError noop，旧TypeError/merror推断已撤销；Math/merror实际含inferred row，旧错误镜像勿提升。证据 `fenced-flags-coverage-check.json`/`class-followup-coverage-check.json`。未重跑full math/font/worker，未验Release/GUI。
- **精确停点**：三旧consumer已退（constructor identity `83cedbf5`、mfenced boundaries/string_errors `9bb90408`）；两Source oracle已解耦 `a0c2890b`，冻结11+9观测exact check通过。清理build1051 exit0，478活动源、6保护路径、17target、9已验输入/二进制及10保留文件不变，见 `mfenced-identity-retirement-final-check.json`。清单/窄图已更新，文档随本次提交；无live agent/build/native。工作区仅余保护6路径。
- **下一步**：只处理class_state/registry工具及两consumer；静态候选 `class-retirement-tools/` 未提升。两oracle没有独立SourceJSON，不能丢掉exact核验或盲造golden。原class-state --check已通过（29值、1682/2523行，旧值缺4/5/6，不是完整8×8；active测试覆盖8×8）。registry原--check因CPP wrapper的Key/creator slot适配而失败；有界诊断证明387 Source值+175生成cases的137776-byte期待值尾段逐字一致，见 `registry-original-check-diagnosis.json`，未修旧wrapper、未改golden。下一动作是据此解耦CPP wrapper、保留Source期待值核验，再退consumer/纯fixture工具；不用重跑上述调查。
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
