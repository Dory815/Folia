# MathJax 原生移植交接记录

更新：2026-10-20。恢复上下文后先读本文件，再进行任务规划或工具操作。
本文件是当前进度入口；架构细则见 `native-raii-architecture.md`，验收规则见
`native-raii-acceptance.md`。用户后续明确指令优先。不要把本记录当作完整功能验收。

## 已确认的用户决定，不要重新讨论

- 复用已经实现过的 C++，原地替换 JS 对象语义与所有权边界；不是重新翻译一遍 TS。
- 移除 Context / Property / ObjectRef 等通用 JS 对象模拟，不再堆释放补丁或兼容桥。
- 保留 MathJax 文件职责、类、方法与调用关系，尤其 CommonWrapperFactory、
  CommonWrapper、CommonMfrac、CommonMsqrt、CommonMroot。
- 数学布局共享给 SVG 和将来的 Direct2D；当前先完成 SVG，经 Folia 现有 SVG 路径接入。
  不需要 D2D 像素对照。Mermaid 暂停。
- 主要使用 gpt-6-luna。确有必要使用 Sol 时只能选 gpt-6.1-sol。
- 单次一个构建，总并发不超过 4，math 编译池为 2。控制磁盘占用，小批提交。
- 使用普通本地开发、测试术语。Windows 控制台测试必须使用仓库的 console runtime。

## 对当前状态的判断（已经核查，不要再次从零盘点）

存在局部架构偏离和未清理的旧实现，不能宣称干净移植或完成接入；不建议整个仓库
reset。用户询问是否需要 reset 不是执行授权，目前没有执行 reset/revert。

保留已验证的数据、算法修正和紧凑测试；修正类职责与生产调用关系；撤掉被替代的职责。
旧 Common 类可从 Git 读取，不能把整套旧 JS 模拟恢复回来，也不能覆盖无关改动。

明确的问题：`b23b2b46` 将分式/根式数学函数移回原 Common 文件，并移除了重复的
placement 和 Scene 家族文件，但当前导出仍是独立输入/结果/函数，只有组件测试调用。
这只是整理，不满足用户要求的原有 Common 类协作，不能再以“路径已经正确”报告完成。

原始 CommonMfrac / CommonMsqrt / CommonMroot C++ 基线可用：
`git show b030b874:src/math/output/common/wrappers/<family>.<ixx|cpp>`。
其他原 Common wrapper 文件还在工作树中。复用类和数学方法，移除其模拟类型依赖；
保留新测试发现的修正，例如根式临时 BBox 只复制 w/h/d 的 Source 行为。

## 当前实际完成与未完成

- 已有原生 MML 基础、部分 TeX 状态/工具、BBox/长度/正则、Scene 和 SVG 组件。
- 完整 TeX parser/stack/package 流程、CommonWrapper 编排、Renderer 门面尚未接通。
- Folia 数学渲染仍使用旧 QuickJS 路径；原生目标不能端到端完成 TeX → SVG。
- 旧 inactive 源码仍有模拟依赖；不在 CMake 中不等于已经清理。不要把旧 runtime
  retirement 补丁作为接下来要提交的实现。

## 已完成验证与提交（仅对应组件范围）

- HEAD 在记录时为 `47eaaa2c`：Scene 变换和 SVG 字体资源语义键修正。
- `1a31f62e`：将巨大分式/根式测试整理为数据和紧凑 runner，保留 288/192 对照案例。
- 字体生成/原生数据实现已整理提交（见本记录对应的字体提交）；完整 Source 对照已跑过：
  14,568,049 个字段，0 差异。日志 `build/math-port/font-semantic-keys-compare.log`。
  对照包含 24,307 个直接字形记录、2,624 个 remap、238,666 个 SVG alias。
- 原生组件测试 102 项 / 30,600 条断言通过：
  `build/math-port/native-raii-font-resource-full.log`。
- 字体初始化已改用 POD 表和有界循环，解决默认栈下初始化问题；没有提高栈限制。
- `-bbm-bold` 与 `-bboldx` 共享缓存 ID 但路径不同，现按语义 variant 查询路径。
  Source 对照脚本 `tools/math-port/font-cache-source-witness.cjs` 已提交。
- 上述结果不等于完整数学布局、SVG 文档或应用接入验收。无需无故重复完整字体对照。

## 下一步，按此接续

1. 字体实现已独立整理提交，不要重复提交或重新做完整对照。
2. 原地迁移现有 Styles 类作为依赖工作，保留已有解析和 shorthand 方法。
   Luna 草稿在忽略目录
   `tools/math-port/drafts/native-raii/styles-native-reuse/`，尚未推广、构建或验收。
   Root 已发现草稿把若干字面量 `\n` 转成实际换行，破坏 C++ 字符字面量，必须修正；
   还需独立核对 Source 用例，不能把候选测试当作已通过。
   不为 inactive Lite 适配器增加兼容桥。原 StyleJson 测试退出的范围需明确记录。
3. 依照原有 C++ 类，先确定 CommonWrapper/Factory 的原生类型与所有权边界，再迁移
   分式/根式方法和调用者。不得继续新增独立数学 helper API 充当最终 wrapper 架构。

Root 负责正式文件推广、CMake、构建、原生执行和提交；Luna 在各自忽略草稿目录工作。
给每项任务指定既有 C++ 方法、保留职责、替换类型、调用者和验收，不让代理自行重划架构。

## 工作区保护

字体实现提交包括 `output/common/font_data*`、`font_path*`、`fonts/newcm_font*`、
`tests/math/native_raii_fonts.cpp` 和 `tools/math-port/font-data-*`。
`math.cpp`、旧 Lite/MML/tree bindings、三份旧 retirement 测试、WinUI SVG 修改及
inventory 工具另有未提交修改；不要顺手一并提交、删除或覆盖。

最近一次恢复时已核查 Git 状态、Common 当前接口与旧类声明，并重读字体比较日志。
接下来不要再重复这些检查来代替执行上述下一步。若 HEAD 或相关文件实际变化，
只核对相应差异并更新本记录。
