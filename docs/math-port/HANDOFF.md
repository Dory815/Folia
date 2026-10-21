# MathJax 原生移植：当前交接

恢复或压缩后先实际读本文件。架构、职责和已完成验收细节见 [native-raii-architecture.md](native-raii-architecture.md)、[native-raii-files.json](native-raii-files.json)、[native-raii-acceptance.md](native-raii-acceptance.md)。只复查可能推翻当前状态的新变化；忽略目录中的候选用 `rg --no-ignore` 查找。

## 当前 checkpoint（2026-10-21）

- HEAD `6fdaf9f0` 完成 script 旧 C++ consumer 删除；Source 批为 `6a289ec5`。simple Source 迁移 `094324b0` 与旧 C++ consumer 删除 `5e5c123c` 已通过。
- Root 已执行 inventory 与 `verify-consumer-removal script-consumer 39 92516`；全通过：478 活动源、保护6路径、已验证9输入/二进制、保留7文件、17 targets、248接口/map一致、active unresolved imports 0。build session 92516 exit 0；当前无 live build。
- simple 迁移证据：106场景、7,667 Source assertions、1,605矩阵行、8,625有序 p/list 值、296递归值；原 fixture 339,824 bytes 完整核对。已完成的 staging bridge 勿重跑。其他既有验收见 acceptance。
- ignored 窄 consumer 图已刷新至 `6fdaf9f0`/39测试、FileAPI `index-2026-10-21T13-04-07-0092.json`；保留 provider/importer/API 引用的全部 hash 均已核。
- multiscript Source-only owner/profile已提升并canonical226/1304 exact；Root完整旧Cpp/480矩阵/2631有序operand证明见 `multiscript-source-bridge-v4.log`、`multiscript-source-promoted.json`。旧Cpp/tools尚未退。table bridge v3已过135/4700、4146operand，staging已存在，待提升。
- class_inheritance 正修 BaseItem bridge：原JSON85项exact，旧Cpp只有951385dc的两处历史类型适配差异（`base-item-original-mismatch.json`），须逐处核再全文比较，不能宣称原check直接通过。无live build/native。

## 精确下一动作

Root 先提升已验table Source-only owner/profile，再按独立小批清理table/multiscript旧Cpp和专用工具/共享gate引用，parallel4 build后核byte-equal，再提交。BaseItem等待修正bridge并执行，不采用先前hash-only候选。所有已完成staging bridge勿重跑。

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
