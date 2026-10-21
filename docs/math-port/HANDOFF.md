# MathJax 原生移植：当前交接

恢复/压缩后先实际读本文件。架构、完整历史验收和文件职责分别见 [architecture](native-raii-architecture.md)、[acceptance](native-raii-acceptance.md)、[files](native-raii-files.json)。只检查能推翻当前结论的新变化，不重复已完成广审；搜索 ignored 候选用 `rg --no-ignore`。

## 当前 checkpoint

- 最新代码提交 `3d359392`。WinUI worker 已走原生 SVG，数学 QuickJS 已退出；对象模拟层的剩余源码/consumer 清理尚未完成，不能宣布整个移植完成。Mermaid 暂停。
- 当前源树仍为648源/644代码、162未配置cpp/ixx；478活动源+83生成源=561配置源，248接口/map一致、active unresolved0、17targets。最新FileAPI `index-2026-10-21T12-19-27-0154.json`。未配置不等于可删除。
- 本轮新增BaseNode四分支测试 `5acd4e70` **1/55通过**；constructor/container、parent rebind、HTML、mfenced class_length5旧Cpp+7专用工具已提交。另walk/replace两consumer+两专用gate已删除待分批提交，780/1504原Source及Source-only exact通过，build57382通过，`walk-protocols-final-check.json`核478活动/保护6/17targets/9已验输入/8保留不变。class_binding测试现43（生产2，inc另计）；生产算法未改。
- Source exact：constructor88、container6480、parent rebind2160、HTML75/434、mfenced length4通过；旧profiles保留。HTML原owner换Source-only，桥先匹配完整旧52398byte fixture，再核75场景/87期待调用顺序、29顶层/52递归值、19pins；不要重跑锁旧owner且拒绝已有staging的bridge。详见 `html-source-promoted.json`。
- 本轮parallel4 builds57997/42085/74847/28779均exit0。最新 `build/math-port/mfenced-length-final-check.json` 确认478活动源、保护6、17targets、9已验输入/二进制、7保留路径不变。当前二进制hash基线 `script-base-final-check.json`；早期build的二进制hash已过时。此前完整SVG1047、字体、RAII、worker等验证见acceptance，本轮未重跑full math/font/worker/Release/GUI；legacy native runners均未跑。
- 最新窄图 `build/math-port/mml-current-consumer-graph.json/.md` 已包含上述退场（标注5ce152c2+dirty mfenced删除，现已提交3d359392，源内容未再改）。Fields→ContextBindings→ParserRuntime owner链完整，values唯一调用在fields.cpp:439；不能孤删/stub provider。

## 精确下一动作与 live handles

没有live build/native。三个Luna仅写ignored候选，Root须读报告/diff、验Source/必要native、提升并小批提交：

1. walk/replace已验收，先分批提交工具/consumer。512/513是旧host预算，不重建；现有typed遍历/parent/return及RAII恢复保留，不声称任意深树有界。operations已完成，无live handle。
2. `/root/class_scripts`：`build/math-port/scripts-retirement-next/`，仅reference_script/reference_simple_mml两consumer。此前paired scripts、wrapper classes、BaseNode缺口均已验收，勿按旧报告重复开问题。
3. accent Luna已完成：`build/math-port/accent-retirement-next/`，Root已读manifest/bridge/diff但未执行；仅raw_accent72/math_accent33，原profiles保留。下一步原oracle/bridge检查→Source-only提升→canonical exact→gate仅减105/2→退两Cpp/build。候选loader须改回 `./mo-pattern-reference.cjs`，去profile writer。forms/accent真实缺口已在原owner修正并验收，勿重开。

收到候选先检查实际文件和当前hash，不按旧agent完成消息推断状态。后续沿 `class-review-*.json` 八组既有结论推进，不另做广审。新Source profile须有完整旧期待值迁移证明，canonical不得保留Cpp emitter或新增并列oracle。

## 固定执行边界

TeX → request-owned MML → 原Common共享布局/placement → immutable Scene → SVG → Folia。保留MathJax模块职责/算法；去掉JS身份、反射和投影协议，不增平行布局/Scene或解释器。Root独占Git/CMake/build/native，一次一个build、parallel≤4；console链接 `folia_console_test_runtime`，异常退出后只做有界诊断。小批提交，不reset，不覆盖无关改动；生成物留ignored build。

## 保护中的6个dirty路径

备份 `build/math-port/context-dirty-backup-20261021/` 已验。处置前必须读diff、核备份并归责；不得顺手覆盖或暂存，保护不等于永久保留。

- `src/app-winui/media/MathJaxSvgParser.cpp`
- `src/app-winui/media/SvgNormalizer.cpp`
- `src/math/core/mml_tree/mml_context_bindings.cpp`
- `src/math/core/mml_tree/mml_context_bindings.ixx`
- `src/math/core/mml_tree/mml_nodes/mtable_bindings.cpp`
- `src/math/core/mml_tree/mml_nodes/semantics_bindings.cpp`
