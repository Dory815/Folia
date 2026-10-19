# MathJax 原生移植交接记录

更新：2026-10-20。恢复上下文后先读本文件，再进行任务规划或工具操作。
本文件是当前进度入口；架构细则见 `native-raii-architecture.md`，验收规则见
`native-raii-acceptance.md`。用户后续明确指令优先。不要把本记录当作完整功能验收。

## 每次恢复上下文的执行顺序

1. 先读本文件，确认已解决的问题、用户决定和“下一步”，再规划或分派任务。
2. 只检查 HEAD、工作区差异和下一步实际依赖的文件；已通过的组件不从头调查。
   若证据与记录冲突，指出具体变化并修正对应条目，不把整份记录作废。
3. 将“声明草稿”“已实现”“已构建测试”“已接入”分开记录；未完成的步骤不能
   因摘要省略而被当作完成，已完成的修正也不能再次当成待办。
4. 有实质进展后更新本文件的状态、验证日志和下一步，并随相应小批提交保存。
   不另建多份竞争的进度笔记。忽略目录中的草稿须记录路径与校验值，不能只靠对话记忆。

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

- `47eaaa2c`：Scene 变换和 SVG 字体资源语义键修正。
- `1a31f62e`：将巨大分式/根式测试整理为数据和紧凑 runner，保留 288/192 对照案例。
- `e1ebbfcd`：字体生成/原生数据实现已独立提交；完整 Source 对照已跑过：
  14,568,049 个字段，0 差异。日志 `build/math-port/font-semantic-keys-compare.log`。
  对照包含 24,307 个直接字形记录、2,624 个 remap、238,666 个 SVG alias。
- 原生组件测试 102 项 / 30,600 条断言通过：
  `build/math-port/native-raii-font-resource-full.log`。
- 字体初始化已改用 POD 表和有界循环，解决默认栈下初始化问题；没有提高栈限制。
- `-bbm-bold` 与 `-bboldx` 共享缓存 ID 但路径不同，现按语义 variant 查询路径。
  Source 对照脚本 `tools/math-port/font-cache-source-witness.cjs` 已提交。
- 上述结果不等于完整数学布局、SVG 文档或应用接入验收。无需无故重复完整字体对照。
- `797e5926`：Styles 已在原文件/原类中完成原生存储迁移，保留解析、TRBL/WSC/font 展开及合并
  方法；删除 Property/property_host 依赖、旧正则对象字段访问和仅用于绑定的函数名元数据。
  使用有序 string/u16string 值容器、原生正则游标、纯 C++ 数字格式化；没有新建解析器。
- Styles 的 Source 对照脚本 `tools/math-port/styles-source-witness.cjs` 跑过 19 组断言；
  原生 Styles 9 项 / 99 断言通过，包含精确 CSS、插入顺序、返回副本、数字/布尔和 UTF-16。
  草稿曾损坏的换行字面量已修复；未确认的“顺序疑点”已核对，没有发现差异。
  最终组件全量 111 项 / 30,699 断言通过：
  `build/math-port/styles-native-reuse-final-full.log`；最终构建同前缀 `final-build.log`。
  旧测试文件中两项 StyleJson 通用对象别名/null 行为测试已退出此目标；它们不是 Styles
  数学/CSS 算法验收，不得把尚未原生迁移的 StyleJson 宣称为已完成。
- LineBBox 已原地完成类型迁移（本记录对应提交）：保留 from/append/copy/getIndentData/
  copyIndentData 的原有职责，用固定三行的原生缩进记录代替 PropertyArray；共享只涉及
  具体缩进值，无节点/对象模拟反向引用。protected 边界保留，测试通过派生 probe 访问。
  Root 补充的语义修正：Source getAllAttributes 的字段读取不解析 `_inherit_`；新增
  AttributeSet::lookupRaw，lookup 继续保留原解析语义。LineBBox 使用 raw 查询。
  `tools/math-port/line-bbox-source-witness.cjs` 对固定哈希的 Source 运行通过；
  原生 LineBBox 3 项 / 25 断言通过，全量 114 项 / 30,724 断言通过：
  `build/math-port/line-bbox-native-reuse-full.log`，构建 `line-bbox-native-reuse-final-build.log`。
- `2dc1bce0`：Typed Tree Factory/Wrapper 已在原有五个文件中迁移。保留
  FactoryServices/AbstractFactory/WrapperServices/Wrapper/AbstractWrapper/
  AbstractWrapperFactory 类职责：一个有序类型登记表，明确 Arguments，构造后移交
  unique_ptr 给具体 owner；没有 Context、any、通用 Property 或第二 creator 对象。
  保留构造登记、替换顺序、默认类别、子类实例判断、源节点/父 wrapper 转发和先序遍历。
  Source witness `tools/math-port/tree-factory-source-witness.cjs` 对固定哈希上游运行通过。
  原生新增 6 项 / 28 断言，全量 120 项 / 30,752 断言通过：
  `build/math-port/typed-tree-factory-full.log`，构建 `typed-tree-factory-final-build.log`。
  当前 Common/MML 消费者尚未接入这些模板，不代表端到端完成。
- `e4405e62`：Typed Tree Node 已在原有 node_types/node 文件中迁移。保留
  Node/AbstractNode/AbstractEmptyNode 的结构操作、虚拟分派、子类查询、复制、遍历、
  空节点行为；NodeOwner 借用具体文档来执行结构修改，没有第二棵树或通用对象状态。
  Source witness `tools/math-port/tree-node-source-witness.cjs` 跑过固定哈希上游；
  原生新增 3 项 / 43 断言，全量 123 项 / 30,795 断言通过：
  `build/math-port/typed-tree-node-full.log`，构建 `typed-tree-node-final-build.log`。
- nodeDefaultAttributes(NodeKind) 已移入现有 node_defaults 模块，唯一物化 kind/global
  默认数据；当前工厂已使用它，后续实际 MML 类初始化也使用它，避免各家族重复。
  构建 `build/math-port/typed-mml-defaults-build.log`，组件全量 123 项 / 30,795 断言通过：
  `build/math-port/typed-mml-defaults-full.log`。这仍不代表 MML 家族已切换。
- OperatorDictionary/mo_patterns 已按闭合原生记录和不可变表迁移，保留 27 个命名
  定义、50 个区间、8 行 spacing、1,252 个 operator 表项和 5 个有限字符模式。
  `node tools/math-port/operator-data.cjs --check` 重现并逐字比较固定 Source 生成结果，
  同时实际执行 30 项模式对照（包括五类末尾换行）；生成证据只写 ignored build。
  全量 129 项 / 30,902 断言通过：`build/math-port/typed-operator-full.log`；
  构建 `typed-operator-final-build.log`。Root 修正了 UT 错误期望：infix + 左间距为 4，
  空文本在 Source 中是显式 ORD infix 项，不是 REL fallback。
  本次仅是运算符数据/查询/有限模式，不代表 Mo 行为类已接入。

## 最新集成结果和下一步（以本节为准）

本轮已复用原有 MML 基类和所有 43 类，完成原生类型迁移并进入实际 arena/唯一工厂。
不是外置语义门面，也没有保留结构节点与数学节点双轨。文件分工见
`typed-mml-implementation.md`；各 Source 脚本的实际覆盖与限制见
`typed-mml-source-coverage.md`。不要用旧 ignored 草稿全量覆盖正式文件，Root 已修过差异。

- 原有模块职责：MmlNode/AbstractMmlNode、Token/Layout/Base、Empty/Text/XML/HTML、
  行/标记、分式根式、脚本、表格、容器和 Mo 行为均在原家族文件中。
- Tree 基类提供唯一 parent_/children_；文档拥有 unique_ptr 节点 arena 与唯一工厂。
  `doc.make` 只 raw 分配，保留空属性/空 shared state；`factory.make` 才调用实际类的
  virtual initializeMml，一次安装状态/defaults/children。测试不能把 raw ctor 当 Source 完整构造。
- `mml_factory.ixx` 是 `mml_document:factory` partition，由文档 primary 导出。
  前置声明/定义归属同一 C++ 模块；消费者 import 文档，不重建旧独立 factory 模块。
  NodeFactory 原来已经借用 MmlFactory&，本轮只调整 import，别再重做所有权。
- XmlRef 是不可变 shared const 值；adaptor shared const owner 不持有 MML/parser/wrapper。
  Token getText 读取实际 TextNode/HtmlNode 子节点；基类 setText 不是 token 内容回退。
- Mfenced 继续使用实际 Mo/Text；open/close/separators 是 arena 内的借用引用。
  doc.attachAuxiliary 在唯一 parent_ 上关联生成节点，不加入 normal children。
  Source 表格错误处理也通过它表达临时语义父关联。旧“改为独立 operator records”决定已撤销。
- Mo 的 unset/cleared/explicit 三态仅由领域枚举表示；实际 class 值只有
  MmlIntrinsicState.working_tex_class 一份。texClass() 非 const/非 noexcept，有真实查询作用。
- 已复用原 util/string 和 js_numeric_input/js_number 算法；原 string 模块的失效
  option_types/Error 依赖改为原生标准异常。不要另造 split/parseInt/number 子集。
- AttributeSet 已修 UTF-16 literal 意外选择 variant<bool> 的问题；四层 setter 的
  text overload 保持字符串类型，bool/null 仍使用闭合原生值。已有回归测试。

### 本轮验证证据

- 实际 43 类构造/RTTI/registered subtype 检查通过：
  `build/math-port/typed-mml-registry-test.log`。
- 集成目标 FoliaMathTests 构建通过。主构建为 `typed-mml-cohort-build-9.log`；
  后续窄修复构建均 exit0，最后为 `typed-mml-cohort-group-fixture-build.log`。
- 完整组件测试 **179 项 / 31,618 断言**，exit0：
  `build/math-port/typed-mml-cohort-final-full.log`。该结果不是 TeX→SVG 或 Folia 接入验收。
- 11 个正式 `tools/math-port/mml-native-*-source-witness.cjs` 均已由 Root 实际运行 exit0，
  日志 `build/math-port/typed-mml-<family>-formal-source.log`；版本/哈希和具体覆盖见 manifest。
- 遇异常后已停止 broader evaluation 并逐案例定位。Munderover 原迁移错用角色 index
  处理物理位置：Source 使用 slots 1/2，Mover 的 under=2 并不意味着必有第三节点。
  已恢复物理 slot 遍历并保留角色索引与 accent table，focused mover 与 full suite 均通过。
- 表格测试曾错误读取不存在的属性导致异常：data-break-align 虽传递给 cell，Source cell
  不存它；maligngroup 传给 mi 的 groupalign 同样不存。已实际运行 Source 核实并修测试。
  `_inherit_` 原始 defaults 用 defaultValue/lookupRaw，不能通过解析后的 lookup 断言哨兵。
- Source copy 与 native exact clone 不同：copy 重建 intrinsic class/prev state，只复制
  own attrs（按 keepIds）、typed properties 和真实 children。已修错误测试并对照 Source。
- Boost.UT optional<size_t> 不与 `_u` literal 比较；用普通 std::size_t 避免 printer 模板错误。
  UTF-16 vector 比较用 ranges::equal/native bool；测试文件使用共享 support/folia_test.hpp。
- 所有构建进程已结束，没有待轮询的 live build；代理本轮包均已交付，不需要等旧状态。

### 紧接着执行

1. **补齐 native exact clone 的新增家族状态**：当前 cloneSubtree 保留原 shared base
   fields/normal children，但没有复制 Mo mode/lspace/rspace、XML payload/adaptor、
   Mfenced auxiliary nodes。需要类型明确的类职责与目标文档内 remapping，不能复制源 arena
   指针、把 Source copy 代作 exact clone，或增加通用对象层。Mrow core cache 也要明确处理。
   已有具体案例建议在 ignored `mml-copy-clone-review/REVIEW.md`，不要重新排查已确认差异。
2. **恢复 Common 实际类协作**：MML family 现已具备基础，不再说“只有 final 结构节点”。
   复核 `common-wrapper-contract/CONTRACT.md`，冻结 CommonOutputJax / CommonWrapper /
   Factory 的实际依赖与状态所有权后，复用旧 C++ 方法原地迁移。此前 Jax/jax() 和临时 BBox
   引用问题仍需处理。分式/根式当前 scalar helper API 不是最终 wrapper 架构。
3. 接通完整 TeX parser/stack/38-package 流程、Common 共享布局、Renderer 和 SVG；
   按固定上游 profile 做真正 TeX→SVG 对照，再替换 Folia 数学 QuickJS worker。
   当前 Folia 仍是旧路径。Mermaid 暂停，不做 D2D 像素对照。
4. inactive 模拟 bindings/hosts/receiver 算法仍未清理，不能把未进 CMake 当作已移植完成。
   在其全部真实消费者迁移后移除对应旧职责，不推广旧 retirement 补丁。

本轮按家族分批提交，整组以同一集成测试验收；接口/工厂/CMake 在最后一笔接通。
已提交：attributes `f99d1a13`，string `cd54ad7f`，branches `a6ff5a5a`，row/tokens
`4d7d8686`，arithmetic `cd97192c`，scripts `20a66b90`，table `5680452a`，containers
`a4b4245f`，Mo `d595a4d7`；最后的文档/唯一工厂/基类/CMake 接入由本记录所在提交保存。
Root 负责正式推广、构建、原生执行与提交；Luna 不自行改变共享架构。

## 工作区保护

字体实现提交包括 `output/common/font_data*`、`font_path*`、`fonts/newcm_font*`、
`tests/math/native_raii_fonts.cpp` 和 `tools/math-port/font-data-*`。
`math.cpp`、旧 Lite/MML/tree bindings、三份旧 retirement 测试、WinUI SVG 修改及
inventory 工具另有未提交修改；不要顺手一并提交、删除或覆盖。

字体证据、Common 原接口与旧类声明已经核查。最近两笔 Tree 实现的构建与测试也已完成。
接下来不要重复这些检查来代替执行上述下一步。若 HEAD 或相关文件实际变化，
只核对相应差异并更新本记录；继续保持一个构建、总并发 4/math 编译池 2。
