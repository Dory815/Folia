# MathJax 原生移植：当前交接

恢复或压缩后先实际读本文件。架构、职责和已完成验收细节见 [native-raii-architecture.md](native-raii-architecture.md)、[native-raii-files.json](native-raii-files.json)、[native-raii-acceptance.md](native-raii-acceptance.md)。只复查可能推翻当前状态的新变化；忽略目录中的候选用 `rg --no-ignore` 查找。

## 当前 checkpoint（2026-10-21）

- HEAD `9f24ae80`（table Source），前一批 `6e7adb88`（multiscript Source）；script旧consumer删除 `6fdaf9f0`。simple/script既有验收不重复。
- Root 已执行 inventory 与 `verify-consumer-removal script-consumer 39 92516`；全通过：478 活动源、保护6路径、已验证9输入/二进制、保留7文件、17 targets、248接口/map一致、active unresolved imports 0。build session 92516 exit 0；当前无 live build。
- simple 迁移证据：106场景、7,667 Source assertions、1,605矩阵行、8,625有序 p/list 值、296递归值；原 fixture 339,824 bytes 完整核对。已完成的 staging bridge 勿重跑。其他既有验收见 acceptance。
- ignored 窄 consumer 图已刷新至 `6fdaf9f0`/39测试、FileAPI `index-2026-10-21T13-04-07-0092.json`；保留 provider/importer/API 引用的全部 hash 均已核。
- multiscript canonical226/1304、table canonical135/4700 exact；完整旧Cpp/矩阵/operand证明见 `multiscript-source-bridge-v4.log`、`table-source-bridge-v3.log` 及各 `*-source-promoted.json`。本提交退两旧Cpp/三专用工具，共享gate只移引用/计数（未跑），保留88-case parent inc。
- build5483 parallel4 exit0，`table-multiscript-consumers-final-check.json`核478活动/保护6/verified9/retained15/targets17/248接口/unresolved0，测试importer39→37。无live build/agent/native。BaseItem bridge v2已过85 exact；旧Cpp限定951385dc两适配后全文相等，见 `base-item-source-migration/executed-proof.json`，不能称原check直接通过；canonical未改。

## 精确下一动作

Root 先窄刷新consumer图39→37，再提升BaseItem实际Source owner，旧p42b JSON仅移cpp字段保留全部85有序label/expected，canonical exact后提交；再独立清理旧consumer/专用工具。已有staging/执行proof勿重跑。

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
