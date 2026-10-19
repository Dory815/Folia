# MathJax 原生移植交接记录

更新：2026-10-20。恢复上下文后先读本文件，再进行任务规划或工具操作。
本文件是当前进度入口；架构细则见 `native-raii-architecture.md`，验收规则见
`native-raii-acceptance.md`。用户后续明确指令优先。不要把本记录当作完整功能验收。

## 恢复时先看这几行

- **Common core 首批已验收**：原地 Wrapper/Factory/Jax、Mo/Mrow/Inferred 类、闭合 warning
  记录，完整原生组件 **220 项 / 32,185 断言** exit0。日志
  `build/math-port/native-common-core-batch-{build,full}.log`；格式整理后的构建
  `native-common-core-batch-formatted-build.log` 和完整
  `native-common-core-batch-final-full.log` 同样 exit0。不要拿下方旧 205 项结论覆盖新进展。
  文本六类及 Mtext/Math 的实现已在工作树，尚未完整验收，当前 CMake 只接入首批三家族，
  为独立小批提交暂时保存的全列表在 ignored `build/math-port/common-full-CMakeLists.txt`。
  六个正式 `common-*-source-witness` 已由 Root 运行成功，输出
  `build/math-port/common-{factory,jax,variant,text,row,mo}-formal-source.log`；row reflow
  另见 `common-row-reflow-formal-source.log`。正式工具覆盖与限制见 `common-source-witness-coverage.md`。
  正在补 LayoutJob 的 inline policy/marking 数学生命周期；实际 LinebreakVisitor 和其余家族
  候选均已交付，尚未编译整合。不要重写这些候选，也不要宣称 TeX→SVG 已完成。
- 本轮确定的修正：cpp 是 `module folia.math.output.common; import :partition;`，
  不能再声明与 interface 同名的 implementation partition；`coreMO()` 必须返回普通
  `CommonWrapper*`，非 Mo 的真实 core 不能被 cast 成空；getBBox(false) 已缓存时返回 cache，
  未缓存时从 BBox.zero() 开始；fontfamily/fontweight/fontstyle 属性优先于移除的 CSS。
  Mrow.shiftLines 在重新对齐前清 L，重复宽度 6/8/10 的 1em 左缩进不累加，已做 Source 对照。
  CommonOutputJax 嵌套 mstyle/semantic-added 分支标记 scanned parent；semantic 的递归也可能
  标记 scanned node，不能断言它始终未标记。正式 Jax Source witness 已覆盖两者。
- 还未整合的关键包：`common-inline-policy-native-impl`（Source 布局入口生命周期，注意其
  overflow 候选用了 setDefault，但 Source 实际 getAllInherited，推广应为 setInherited）；
  `common-fraction-class-native-impl`（实际 Mfrac+288 案例）、`common-radical-class-native-impl`
  （实际 roots+192 案例）、`common-scripts-native-impl`、`common-fence-action-native-impl`、
  `common-linebreak-native-impl`（完整 cpp，已改 signed depth，Source 孤立 close: -1/2700）。
  所有包均已交付，不要重新从头分派。正式 header/source 是推广后的权威，旧 mirrors 不能覆盖。
- MML 集成基线 `33ea03fb`：43 类实际 MML 已接入唯一工厂和文档 arena。
  后续 exact clone、StyleJson、Common 值类型/字体 CSS 数据也已实现并验收，
  TreeVisitor 也已原生实现并验收；当前组件测试 **205 项 / 32,065 断言**通过。
  不要重新迁移这些类或重跑已完成的大盘点。
- 下一项是恢复 Common 实际类协作，契约见 `common-native-implementation.md`。
  `MmlClone` / `copyNativeStateTo` 已在原有模块中
  实现并构建测试，**不再是待实现的讨论方案**；家族状态、辅助节点映射和异常回滚已补齐。
- TeX→SVG 和 Folia 原生接入尚未完成；Mermaid 继续暂停。
- 现有未提交改动见文末“工作区保护”，本次记事不改变其归属。
- 2026-10-20 用户再次要求压缩后读取小本本。已有记录不能代替实际执行：
  恢复时必须读取本文件正文，不能只凭压缩摘要或文件名声称已经恢复状态。

## 每次恢复上下文的执行顺序

1. 先读本文件，确认已解决的问题、用户决定和“下一步”，再规划或分派任务。
2. 只检查 HEAD、工作区差异和下一步实际依赖的文件；已通过的组件不从头调查。
   若证据与记录冲突，指出具体变化并修正对应条目，不把整份记录作废。
3. 将“声明草稿”“已实现”“已构建测试”“已接入”分开记录；未完成的步骤不能
   因摘要省略而被当作完成，已完成的修正也不能再次当成待办。
4. 有实质进展后更新本文件的状态、验证日志和下一步，并随相应小批提交保存。
   不另建多份竞争的进度笔记。忽略目录中的草稿须记录路径与校验值，不能只靠对话记忆。
5. 对准备重复调查的问题，先查本文件的既有结论和证据。只有相关源码、HEAD 差异或
   新测试结果使结论失效时才重查，并记录具体原因；不能把压缩造成的信息缺失当成源码问题。

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

1. **继续 Common 下一批整合**：core Wrapper/Factory/Jax、Mo/Mrow/Inferred 已通过
   220 项 / 32,185 断言。不要再冻结一遍旧草稿或重做其基础。
   先补 `common-inline-policy-native-impl` 的布局入口步骤并接入工作树已有的 Text/Mtext/Math，
   完成剩余 UT，再用已交付的实际 fraction/radical/scripts/fence/action 与 visitor 包继续整合。
   分式/根式当前生产文件仍是 scalar helper API；候选实际类尚未推广，不能当作已完成。
2. 接通完整 TeX parser/stack/38-package 流程、Common 共享布局、Renderer 和 SVG；
   按固定上游 profile 做真正 TeX→SVG 对照，再替换 Folia 数学 QuickJS worker。
   当前 Folia 仍是旧路径。Mermaid 暂停，不做 D2D 像素对照。
3. inactive 模拟 bindings/hosts/receiver 算法仍未清理，不能把未进 CMake 当作已移植完成。
   在其全部真实消费者迁移后移除对应旧职责，不推广旧 retirement 补丁。

本轮按家族分批提交，整组以同一集成测试验收；接口/工厂/CMake 在最后一笔接通。
已提交：attributes `f99d1a13`，string `cd54ad7f`，branches `a6ff5a5a`，row/tokens
`4d7d8686`，arithmetic `cd97192c`，scripts `20a66b90`，table `5680452a`，containers
`a4b4245f`，Mo `d595a4d7`；最后的文档/唯一工厂/基类/CMake 接入由本记录所在提交保存。
Root 负责正式推广、构建、原生执行与提交；Luna 不自行改变共享架构。

### Native exact clone（2026-10-20 已完成）

- 文档内单次 `MmlClone` 只保存源/目标的借用指针映射，不拥有节点；目标 arena 仍是唯一 owner。
  先登记 raw 目标再复制 normal children，最后 virtual hook 复制实际家族状态。
- Mo 原类复制 mode/lspace/rspace；XML/HTML 共享不可变 payload/adaptor；Mfenced
  在同一映射中复制生成 Mo/Text，保留别名、空项和唯一父链接；Mrow/inferred row 清空借用 cache。
  不调用可能改状态的查询来重建快照，也不改变 Source `copy` 的重构行为。
- 失败时销毁本次新 arena tail，已有目标节点/root 与源树不变。测试通过实际派生 hook 抛异常，
  检查节点析构计数；另覆盖辅助引用父冲突。没有新增通用对象、释放补丁或第二所有权层。
- Root 实际构建 exit0：`build/math-port/mml-exact-clone-final-build.log`；
  聚焦 exit0：`mml-exact-clone-final-focused.log`；完整 **194 项 / 31,793 断言** exit0：
  `build/math-port/mml-exact-clone-final-full.log`。新增 15 项 / 175 断言。
- 四个 Luna 按冻结接口各自负责 Mo/XML/Mfenced/Mrow 原文件和独立测试；Root 负责文档/base、
  回滚测试、审查、CMake 和执行。正式职责/验收在 `typed-mml-implementation.md`。
- 此结果依旧只是组件验收，不是 TeX→SVG/Folia 接入证明。

### StyleJson（2026-10-20 已完成）

- 原 `util/style_json` 类/方法原地迁移为有序、拥有值的 CSS 定义树；叶子只有字符串/数字，
  嵌套只有 CSS 定义。没有 Property/Context、通用对象身份、布尔/null/function 或兼容桥。
- 保留 shallow merge、替换/追加顺序、嵌套规则和精确 whitespace/数字/UTF-16。
  原生输入快照不与外部可变结构共享，属于用户要求的原生值类型边界。
- Root 执行 `tools/math-port/style-json-source-witness.cjs`，11 项通过，运行前强制校验
  CJS SHA-256 `dfd45381fc15fa9b8a038e8ac782550934b5327871d10534df52af02aa43f11f`。
  日志 `build/math-port/style-json-source.log`。
- 构建 exit0：`native-style-json-verified-build.log`；聚焦 exit0：
  `native-style-json-focused.log`；完整 **200 项 / 31,805 断言** exit0：
  `build/math-port/native-style-json-full.log`。新增 6 项 / 12 断言。
- Root 修正测试里遗漏的 sibling color 和孤立 surrogate 整条规则期望，已执行 Source 核对；
  继续使用 compare/ranges::equal 避免 Boost.UT 的 UTF-16 运算符/打印器问题。
- inactive Common bindings/output bridge 仍引用旧 PropertyList，后续随其真实消费者迁移移除，
  不保留临时转接。StyleJson 完成不代表 CommonOutputJax 或 SVG pipeline 已完成。

### Common 值类型与字体 CSS 数据（2026-10-20 已完成）

- `common:types` 已原地移除 Property/旧反射 class-ref 类型，进入当前构建图并由 primary 导出。
  它只定义有限的 metrics/options/measurement/stretch 状态和 Common 前置声明，不是 Jax 实现。
- `CssFontData` 归实际 `font_data` 模块唯一所有；Common 导入，不重复声明跨模块类型。
  FontFaceData 的 cssFamilyPrefix/cssFontMap/styles 和 getFamily/getCssFont 已原生实现。
- 正式生成器是 **tools/math-port/font-data-generate-native.cjs**（CMake:98），
  不是旧 ignored font-data-generate-typed.cjs 或旧非原生生成器。新增 33 条 CSS 元数据，
  没有复制另一份字形负载。Source `styles===undefined` 经实际 StyleJsonSheet 验证无贡献，
  故此固定 profile 的原生值为空 CSS 定义；这不是为缺失数据发明 fallback。
- Root 跑过 `font-css-source-witness.cjs`：33 个实际 NewCM/extension CSS tuple，
  map SHA-256 `db47e065c8d2a277c6268c1420cfa36bc1f02d1a424c66949baf402624d021e9`。
  构建/聚焦/完整 exit0：`build/math-port/native-font-css-{build,focused,full}.log`。
  完整 **201 项 / 32,044 断言**；真实 provider-backed 元数据测试新增 1 项 / 239 断言。
- Mo 所需 delimiter 数组已做有限 Source presence census：222 条配置记录的
  sizes/variants/schar/stretch/stretchv 均没有 present-empty。沿用现有原生 vector，
  生成器拒绝语义不同的 present-empty variants/stretch/stretchv，不能无证据重写 optional。
  census 见 ignored common-mo-native-impl/delimiter-array-presence.json；不需重复 14M 字形对照。

### Common 当前执行断点（候选实现尚未整合验收）

- 正式责任/模块约定：`common-native-implementation.md`。types 与 wrapper/factory/Jax 已推广、
  构建测试通过；正式 source/header 是权威，不得用旧 `common-contract-refresh/headers` 覆盖。
  已接入的 Mo/Mrow/Inferred 与尚未接入的 Text/其他家族应按文首断点区分。
- 已确认的重要 Source 细节：factory adopt 只拥有并初始化；**CommonWrapper::wrapNode**
  才负责显式 parent 时追加和完成后的 nodeMap 登记，base initialize 的默认 child wrap 只追加一次。
  禁止把同一逻辑同时塞进 factory.wrap 和 wrapper.wrap。通用 Common defaultNodes 为空，
  具体 catalog 和 typed LinebreakFactory 由调用方构造输入提供；不注册 generic 未实现 family。
- Jax 单独借用 document，LayoutJob 只拥有 Jax、转发 document()；Jax 拥有 factory 再 visitor，
  析构自然为 visitor 再 factory。metrics.containerWidth 是像素，containerWidth_ 是计算后的 em。
  pxPerEm=metrics.ex/face.params.x_height；getLinebreakWidth 先取 root maxwidth。
  data-semantic-added 是已有 AttributeSet 属性，不是缺少的 NodeProperties 字段。
- CommonText stretch 字符：selectedCodepoint 或 recipe.c 非零时直接使用，不再 remap；
  两者均无则 parent.remapChars(unicodeChars(...))。sourceCodepoint 只是 recipe key。
- 原生 NewCM bundle 函数 **makeNewcmFontData() 确实存在**，由 newcm_font_module_provider 导出；
  它内部调用 installNewcmFontData，不要根据较窄 data 模块误判缺失。
- 以下 ignored 根目录各已有实质代码/UT/Source 证据，需审查整合，不要重新分派从头写：
  common-wrapper-native-impl（raii_common_font_audit）；common-factory-native-impl
  （raii_mml_foundation，含合并 Jax 候选）；common-jax-native-impl（fraction_table_runner，
  math/service 方法原件）；common-mo-native-impl（raii_scene_foundation）；
  common-row-native-impl（font_assert_audit，Root 已把家族文件移回 packet 的 wrappers 子目录）；
  common-text-native-impl（layout_fixture_cleanup，6 个实际家族）。
- Root 实际执行过 common-wrapper-native-cases/run.cjs，4 个 Source 案例通过，
  日志 `build/math-port/common-wrapper-native-cases-source.log`。其他新包 Source 执行由 Luna 报告，
  尚不可当作 Root 原生验收。外部 UT 必须 import Common primary，不能 import partitions。
- 仍需整合实际 Common LinebreakVisitor（不能用生产 no-op）；正式 header 已 typed，
  cpp 当前仍未进入 CMake，完整原生 cpp 已在 `common-linebreak-native-impl` 候选。
  `Linebreaks` 最小纯虚接口已冻结在正式 Common 文档。**TreeVisitor 已完成**，下一步用它改造
  Common visitor 声明/实现，并整合真实 Common core；不要重做 TreeVisitor 或把旧依赖加回。
- 未解决验收项：无效 mathvariant Source 警告需要闭合诊断值接口；Common 基类 forceInlineBreaks
  是 false，但 Source SVG 返回 options.linebreaks.inline，最终 SVG 对照必须保留这个实际策略。
  TeX parser/packages、CommonMath/其他家族、fraction/radical 实际类、Scene 发射和 Folia 接入仍未完成。

### Typed TreeVisitor（2026-10-20 已完成）

- 原 visitor.ixx 原地替换为 typed Product/Kind/Argument/Result 模板；保留 visitTree/Node/Default、
  factory kind snapshot、set/remove handler 和 live children 遍历。移除 std::any、反射 methodName/
  MethodLookup/VisitorMethod、ChildValue 和 JS receiver/coercion 行为。
- 每个已登记的实际 callable 用标准 shared_ptr<const Handler> 保持正在执行时的生命期；
  dispatch 只复制该 owner，不复制 std::function target，因此 callback 自替换/移除及 mutable
  closure 状态正确。只涉及本 Visitor 的有限 typed callback，无对象堆、retirement 或释放补丁。
- Root 执行正式 `tools/math-port/tree-visitor-source-witness.cjs` 成功；新增 4 项 / 21 断言，
  覆盖 Source snapshot/replace/remove/live append/result 和实际 callback 析构时机。
  构建/聚焦/完整 exit0：`build/math-port/native-tree-visitor-{final-build,focused,full}.log`，
  Source 日志 `native-tree-visitor-source.log`。完整 **205 项 / 32,065 断言**。
- 这不代表 Common LinebreakVisitor 已移植；其余 inactive Tree/MML visitor 消费者尚需原生改造，
  不增兼容别名，也不宣称 inactive 老源码已清理。

### Common 候选关键文件校验值（避免拿旧包覆盖 Root 已修结果）

以下路径均相对 `tools/math-port/drafts/native-raii/`；均未原生整合验收。

| 文件 | SHA-256 |
| --- | --- |
| common-contract-refresh/headers/types.ixx | 58DC2CFEA8B0AE29766B14C5B197C5B7E63FE3ACBA958A628B78A2478683332B |
| common-contract-refresh/headers/wrapper.ixx | 1F1FD1D6C83909B8E2ADD3AEAFF6BD0FA58EAA69946C58C79E347917359D01C4 |
| common-contract-refresh/headers/wrapper_factory.ixx | D1AED312E89EC4147FA4C81AECD53AB2CB129C87B66025D0D57F8FF9C6714339 |
| common-contract-refresh/headers/output_jax.ixx | 5E218322FCE8CFD8DFF57DFE16D08012CDACB56890164C38142F1C7DCB826D98 |
| common-wrapper-native-impl/src/math/output/common/wrapper.cpp | D5FD30B9FD2519116FE24837E2883ECFF8EDA708BAE194513F3CA617D0350D7E |
| common-factory-native-impl/src/math/output/common/wrapper_factory.cpp | F573547A38FDE8A28C3AA421204215779CB644D7A524FCF6920027EC591C6C4A |
| common-factory-native-impl/src/math/output/common/output_jax.cpp | AC04596D01E845803292CE40CAB7EE62CAD9354A96876F7F4A616627D132D77C |
| common-mo-native-impl/src/math/output/common/wrappers/mo.cpp | 4999861803C0E66E7669E0E0B6B5578642CC75BF692BDB3A0B68723709F64EB6 |
| common-text-native-impl/src/math/output/common/wrappers/text_node.cpp | 858E307ED486265BFD274D4FEE95BF10FF2E074B41C2BE23DBD02503B903CFA1 |
| common-row-native-impl/src/math/output/common/wrappers/mrow.cpp | 18C11407FF455D0ED894A4F16AEDC83FBF5EB150014451D4CB41156274D244B2 |

Root 已修 factory packet 的 merged output_jax.cpp 和 lifecycle snippet：nodeMap 登记应
insert_or_assign（Source Map.set），不能抛出“同节点已有不同 wrapper”。其旧 unified patch/hash
因此已过期；整合时读当前文件并重新生成 diff，不要直接应用旧 patch。

所有本轮构建已结束；Luna 包均已交付，没有需要继续等待的旧 build/agent 句柄。

本轮提交：`8a0ac1e5` exact clone 实现、`a9b5bd51` clone 验收、`ac28a245` StyleJson、
`a86df3fa` Common 值/字体 CSS、`98f601b2` TreeVisitor 和 CMake 列表修正。
最终 native C++ HEAD `98f601b2` 已重新 configure/build/full-test，exit0：
`build/math-port/native-foundation-final-head-{configure,build,full}.log`，205 项 / 32,065 断言。
随后仅作本交接记录和 Source witness 文件末尾空行整理，不改变原生实现。

## 工作区保护

字体实现提交包括 `output/common/font_data*`、`font_path*`、`fonts/newcm_font*`、
`tests/math/native_raii_fonts.cpp` 和 `tools/math-port/font-data-*`。
`math.cpp`、旧 Lite/MML/tree bindings、三份旧 retirement 测试、WinUI SVG 修改及
inventory 工具另有未提交修改；不要顺手一并提交、删除或覆盖。

字体证据、Common 原接口与旧类声明已经核查。最近两笔 Tree 实现的构建与测试也已完成。
接下来不要重复这些检查来代替执行上述下一步。若 HEAD 或相关文件实际变化，
只核对相应差异并更新本记录；继续保持一个构建、总并发 4/math 编译池 2。

分批暂存 CMake 不要过滤 zero-context diff 的前部 hunk 后直接应用后部绝对行号；
本轮因此曾把 types 行暂存到 FILE_SET 括号外，工作树构建虽过但暂存版本不正确。
最终提交已恢复正确列表位置，并对最终 HEAD 重跑 configure/build/test；不能用工作树
先前通过的日志推断每个中间暂存提交都能独立配置。
