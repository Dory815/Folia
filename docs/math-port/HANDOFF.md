# MathJax 原生移植：当前交接

恢复/压缩后先实际读本文件。完整架构、历史验收、职责见 [architecture](native-raii-architecture.md)、[acceptance](native-raii-acceptance.md)、[files](native-raii-files.json)。只检查能推翻记录的新变化；ignored候选用 `rg --no-ignore`。

## 当前 checkpoint

- 上个代码HEAD `8632e901`；本次accent两oracle/gate及两consumer已改，待分批提交。WinUI worker已接原生SVG、数学QuickJS已退出；剩余对象模拟源码/consumer清理未完，不能宣布移植完成。Mermaid暂停。
- 源树未改：648源、162未配置cpp/ixx；478活动+83生成=561配置、248接口/map一致、active unresolved0、17targets。最新FileAPI `index-2026-10-21T12-31-24-0772.json`；未配置不等于可删。
- BaseNode四分支测试 `5acd4e70` **1/55通过**。本轮已归责退9旧Cpp consumer、9专用工具；class_binding直接cpp/ixx importer生产2+测试41（inc另计）。生产算法未改。
- Source exact均通过：constructor88、container6480、parent rebind2160、HTML75/434、mfenced length4、token walk780、replaceChild1504、raw/math accent72/33；旧profiles保留。HTML新profile有完整旧fixture/全部Source期待迁移证明，见 `html-source-promoted.json`，勿重复跑锁旧owner/已有staging的bridge。
- 最新removal build97637 parallel4 exit0；`build/math-port/accent-protocols-final-check.json`确认478活动、保护6、17targets、9已验输入/二进制、7保留不变。当前二进制基线 `script-base-final-check.json`。此前SVG1047/完整math/font/RAII/worker见acceptance；本轮不重复广测，legacy native runners均未跑。
- 窄图 `build/math-port/mml-current-consumer-graph.json/.md`目前到8632e901/43测试；提交accent后只需减两旧边至41并刷新FileAPI。Fields→ContextBindings→ParserRuntime owner链仍完整，values唯一调用fields.cpp:439；不能孤删/stub。

## 精确下一动作

先分批提交accent工具/consumer，再刷新窄图和此checkpoint。无live build/native。

唯一live Luna `/root/class_scripts`，仅写ignored `build/math-port/scripts-retirement-next/simple-source-migration/`：Root已拒绝仅保留kinds/pins而丢掉simple_mml的原期待值。新要求与HTML相同：ignored observer先完整匹配旧Cpp，再证明全部Source输入/期待顺序，生成被证明的独立JSON；clean Source-only原owner不得有Cpp模板/槽表/并列oracle。尚未交付/执行。原 `run-retirement-gates.cjs`只静态交付，勿运行来重复已接受Common/string广测；reference_script/simple_mml均未删。

已决事项勿重开：paired scripts、wrapper classes、BaseNode、forms/accent缺口均已修/验；512/513是旧host预算而非MathJax算法，walk/replace consumer已退，不重建；现有typed遍历/parent/return与RAII恢复保留，不声称任意深树有界。后续沿八组 `class-review-*.json` 既有报告推进。

## 固定边界

TeX → request-owned MML → 原Common共享布局/placement → immutable Scene → SVG → Folia。保留MathJax职责/算法，去JS身份/反射/投影协议；不增平行布局/Scene或解释器。Root独占Git/CMake/build/native，一次一个build、parallel≤4；console链接 `folia_console_test_runtime`，异常退出后只做有界诊断。小批提交，不reset，不覆盖无关工作；生成物留ignored build。

## 保护中的6个dirty路径

备份 `build/math-port/context-dirty-backup-20261021/` 已验。处置前读diff/核备份/归责；不得顺手覆盖或暂存，保护不等于永久保留。

- `src/app-winui/media/MathJaxSvgParser.cpp`
- `src/app-winui/media/SvgNormalizer.cpp`
- `src/math/core/mml_tree/mml_context_bindings.cpp`
- `src/math/core/mml_tree/mml_context_bindings.ixx`
- `src/math/core/mml_tree/mml_nodes/mtable_bindings.cpp`
- `src/math/core/mml_tree/mml_nodes/semantics_bindings.cpp`
