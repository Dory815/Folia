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
- Typed Tree Node 已在原有 node_types/node 文件中迁移（本记录对应提交）。保留
  Node/AbstractNode/AbstractEmptyNode 的结构操作、虚拟分派、子类查询、复制、遍历、
  空节点行为；NodeOwner 借用具体文档来执行结构修改，没有第二棵树或通用对象状态。
  Source witness `tools/math-port/tree-node-source-witness.cjs` 跑过固定哈希上游；
  原生新增 3 项 / 43 断言，全量 123 项 / 30,795 断言通过：
  `build/math-port/typed-tree-node-full.log`，构建 `typed-tree-node-final-build.log`。

## 下一步，按此接续

1. 字体实现已独立整理提交，不要重复提交或重新做完整对照。
2. Styles 与 LineBBox 均已原地迁移并验证，直接接续 Common/MML 类协作。
   旧 inactive LinebreakVisitor、mrow、mtext 和 bbox_bindings 仍引用旧数组 API；
   在各自迁移中重接类型或移除模拟绑定，不加兼容桥，不宣称这些消费者已完成。
3. 依照原有 C++ 类，先确定 CommonWrapper/Factory 的原生类型与所有权边界，再迁移
   分式/根式方法和调用者。不得继续新增独立数学 helper API 充当最终 wrapper 架构。
   契约候选在 `tools/math-port/drafts/native-raii/common-wrapper-contract/CONTRACT.md`。
   Root 已要求纠正两点：保留 CommonOutputJax 类及 jax() 的布局服务职责、唯一状态来源；
   非缓存 BBox/outer BBox 不能返回临时引用。LineBBox 实际算法需要的类型明确、无环共享
   所有权可以保留，不能为了消灭所有 shared_ptr 而破坏算法别名关系。
   完整 MML core/embellished/spacing 语义仍是缺失依赖，不能加空实现掩盖。
   契约尚未接受：不能直接删掉 tree::AbstractWrapper/AbstractWrapperFactory 的真实
   kind/wrap/walkTree/工厂分派职责。Root 已纠正，Luna 已在同一草稿中记录。
   工厂登记表已确定并实现，详见架构新增 'Typed factory and actual MML family decision'。
   MML 已决定恢复真实多态家族类，作为 MmlDocument arena 的实际对象。当前正式代码
   仍是 MmlNode final，只含结构，旧类还不能进入 arena；不得用外置门面掩盖。
   复用旧 MML 基类的 C++ 可从 `f4d04f25:src/math/core/mml_tree/mml_node.ixx` 及
   对应实现读取（dc287b48 之前）；具体家族文件仍在当前树中。
   注意：当前 util/lengths 已是纯原生 LengthContext API，草稿所称仍依赖 Property
   是过时信息，不能据此重做长度模块。
4. Node/AbstractNode/AbstractEmptyNode 已正式推广并验证，不要重做。MML 接入时继承
   此唯一 parent_/children_ 存储；具体 MmlDocument 的 NodeOwner 接口仍需实现。
5. MML 基类声明草稿在 `tools/math-port/drafts/native-raii/typed-mml-base-contract/`。
   尚未接受：Root 正在要求修正错误 module 名、重复 texclass 状态、缺少的 Source 默认
   虚方法、错误 addInheritedAttributes 参数和继承规则表类型。修复前不能分派家族实现。
   已确定的工厂所有权：文档拥有唯一 MmlFactory；NodeFactory/测试借用 doc.factory()。
   doc.make -> factory.allocateRaw -> 单一登记表 -> 实际派生节点 -> doc.adopt；
   MmlFactory.make 做 defaults/arity 再直接 allocateRaw，不能回调 doc.make 形成递归。
   doc.make 仍是原有无 defaults 的底层构造，保持现有 absent 属性测试语义。
   Root 已选择保留原有 XmlObject/XmlAdaptor/XMLNode/HtmlAdaptor/HtmlNode 协议，
   以不可变 XML 值及 shared const adaptor owner 明确 RAII 生命周期，不新建 DOM；
   属性接口只用字符串/optional 字符串，不再用通用 Property。
   gpt-6.1-sol 仅做了这一份 MML 头文件只读复核，发现了需要保留的继承 origin、
   token defaults/walkTree、TextNode 虚拟 setter、verifyTree 省略语义和 factory adopt/friend。
   继承输入已修正为 {name,origin:NodeKind,AttributeValue}，additions 保持普通键值；
   display/level/prime 改为 Source 的 bool/double/bool 默认 false/0/false。
   完整复核在该 Sol 的最终消息；Root 应将修正落实到草稿再冻结，不得假定已实现。

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
