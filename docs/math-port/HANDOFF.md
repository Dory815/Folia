# MathJax 原生移植：当前交接

恢复或压缩后先实际读本文件。架构、职责和已完成验收细节见 [native-raii-architecture.md](native-raii-architecture.md)、[native-raii-files.json](native-raii-files.json)、[native-raii-acceptance.md](native-raii-acceptance.md)。只复查可能推翻当前状态的新变化；忽略目录中的候选用 `rg --no-ignore` 查找。

## 当前 checkpoint（2026-10-21）

- 最新代码 `8ba6b468` 退BaseItem旧Cpp/专用工具；Source owner批 `a6215622`。此前table/multiscript清理 `031da185`，Source批 `9f24ae80`/`6e7adb88`。原生SVG已接Folia，残余对象模拟源码/consumer清理仍未完，不能宣布完整移植完成；Mermaid暂停。
- 最新build89489 parallel4 exit0；`base-item-consumer-final-check.json`核478活动、保护6、verified9、retained24、17targets、248接口/map一致、active unresolved0，测试importer37→36。无live build/native。此前SVG1047/完整math/font/RAII/worker验收见acceptance，不重复广测。
- multiscript226/1304、table135/4700、BaseItem85 Source exact，全部旧期待已保留；证明见acceptance与各 `*-source-promoted.json`。BaseItem旧Cpp只限定951385dc两历史适配后全文相等，不能称原check直接通过。已有staging/执行proof勿重跑。
- ignored窄图已到 `8ba6b468`/36测试、FileAPI `index-2026-10-21T13-27-41-0497.json`；保留引用hash均已核。Fields→ContextBindings→ParserRuntime owner链仍完整，唯一values调用fields.cpp:439，不能孤删/stub。
- 两个Luna仅准备ignored候选：class_verification→`verification-source-migration/`（reference_mml_verification/p35现有72行profile）；class_operations→`copy-source-migration/`（copy/copy_boundary）。均待Root审与Source执行，未提升；只动ignored，不跑Source/native/build。

## 精确下一动作

Root 审两Luna候选，保留原profile全部期待并运行实际Source exact后才提升；共享gate仅删对应引用/计数，专用旧runner退场，不运行旧对象协议native gates。按职责小批提交，别重复已验问题。

## 固定边界

- 数据路径：TeX → request-owned MML → 原 Common 共享布局与 placement → immutable Scene → SVG → Folia。保留 MathJax 职责与数学算法；移除 JS 身份、反射和投影协议不等于删除数学行为。不得引入平行布局/Scene 或解释器。Mermaid 移植暂停。
- 未配置源不等于死代码。删除前核实直接消费者、owner与当前 FileAPI/configuration；保留算法和必要数学覆盖。
- 已完成的 staging bridge/profile 不重复运行。512/513 是旧 host 遍历预算，不是 MathJax 数学算法边界；不要以此重建旧 walk/replace consumer。
- Root 独占 Git、CMake/configure、build 和 native 测试；一次一个 build，parallel ≤4。Windows console probe 链接 `folia_console_test_runtime`。异常进程退出后停止扩大测试，仅做有界诊断。
- 小批提交，不 reset，不覆盖无关改动。生成物留 ignored build。处置保护路径前必须读 diff、核对备份并明确归责。

## 保护中的6个 dirty 路径

原字节与 patch 备份在 `build/math-port/context-dirty-backup-20261021/`；备份已验。以下路径不得顺手覆盖、暂存或删除：

- `src/app-winui/media/MathJaxSvgParser.cpp`
- `src/app-winui/media/SvgNormalizer.cpp`
- `src/math/core/mml_tree/mml_context_bindings.cpp`
- `src/math/core/mml_tree/mml_context_bindings.ixx`
- `src/math/core/mml_tree/mml_nodes/mtable_bindings.cpp`
- `src/math/core/mml_tree/mml_nodes/semantics_bindings.cpp`
