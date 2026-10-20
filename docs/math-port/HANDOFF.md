# MathJax 原生移植交接记录

更新：2026-10-20。恢复上下文后先读本文件，再进行任务规划或工具操作。
本文件是当前进度入口；架构细则见 `native-raii-architecture.md`，验收规则见
`native-raii-acceptance.md`。用户后续明确指令优先。不要把本记录当作完整功能验收。

## 当前唯一执行断点（2026-10-20；恢复时先读）

本节是唯一当前状态。下文旧阶段仅保留结论/证据，不是待办。压缩后实际读取正文，
不能根据旧消息重派已完成包或重新调查已解决问题；后续用户明确指令优先。

### 恢复时先用这个短检查点（2026-10-20 Textmacros/Textcomp已验收提交）

- **当前无live构建/测试/代理**。最终完整Math回归587项/142,968断言，117套suite，无失败/
  跳过，exit0，text-mappings-full.log；84690已终态。Root补全Textmacros两张表116项Source
  独立fixture，实查名称/handler/typed参数；新增1项348断言通过，d6866698已提交3文件。
  构建38727 exit0（并发4），text-mappings-build.log。此前生产构建95206 exit0，
  textmacros-textcomp-build.log；完整586/142620 exit0，textmacros-textcomp-full.log，91558终态。
  聚焦Textmacros原11/520、Textcomp5/2338均exit0，新增mapping用例后Textmacros共12/868。
  提交73482d77（10文件，TeX拥有子配置/原TextParser与共享构造）、21e2e6f7（9文件，原
  Textmacros Methods/Maps/Config与测试/Source）、ac28848f（10文件，Textcomp两组、删除
  多拆出的Methods两文件，独立117条Sourcefixture和runner）。原13项无关修改保留。
  Root独立重放Source均exit0：Textmacros91compile/12错误类+真实TextParser结构检查；
  Textcomp358compile/全117映射，原4TS+4CJS、原2TS+2CJS各自/full38锁。
  Source日志分别textmacros-source-final.log/textcomp-source-final.log；Node堆512MB。
  Root已完整读原Textmacros四TS、当前Maps/Methods/Config全部CPP，
  TextParser CPP数学主体及后半反射层；原Textcomp两TS/当前全部CPP与117条mapping_data.inc。
  保留原TextParser分区和Methods/Mappings/Configuration三组（平铺原路径），不新造副本。
  Root已冻结原头/共享：TextMacrosData在parse_options.ixx，unique_ptr持有textConf/parseOptions，
  TexParser*仅internalMath作用域借用；TeX独占Data，主/文本ParseOptions借用Data，不塞进session。
  TeX::installTextMacros接管；ParserConfiguration新增registry/warnings只读借用accessor。
  TextParser原header改具体vector/envStack/optional<ScriptLevel>、DeferParsing构造，导出原分区。
  environment.math_size改optional<variant<double,u16string>>，原SetSize数字不能转em字符串。
- **代理已交付**：GPT-6.1 Sol /root/base_methods仅处理text_parser.cpp、textmacros_methods.cpp、
  tests/math/textmacros.cpp、tex-textmacros-source.cjs四文件，未改共享/native构建测试/Git。
  原18Methods/11Base别名/原TextParser实际算法及10组测试已交付冻结；Root接管格式化并新增
  子配置options共享/样式捕获与全116映射两组（共12），已原生/Source验证。
  Root负责Maps/Configuration、Textcomp、全部共享/CMake/独立Source/原生验收/Git。
  固定38profile无autoload/require，保留CheckAutoload无动态加载分支，不造Promise/调度器。
  Math的slash分支必须保留Sourcefallthrough，正确匹配\\(...\\)，旧CPP此处条件有差异。
- **Root已验收实现**：原TextMacros Maps/Configuration（同session/options、独立子ParseOptions/
  Tags；internalMath RAII恢复主parser borrow；preFilter绑定本次MML factory）。Textcomp回归
  原Mappings内Insert callback与Configuration两组，删除原本额外拆出的methods.cpp/.ixx，
  复用117条原数据，数学/文本模式均已验收。
  Textcomp实际Source witness/独立117条Sourcefixture和5组原生runner已写并格式化。
  Root另核对Source ParseOptions子构造先捕获自身config.mathStyle，再共享主options对象；
  新SharedTexOptions构造重载复用同一初始化，避免对子配置重复mergeDefaults覆盖主选项。
  Source/native测试主French/子ISO与options同地址；常规构造仍原merge语义。验收全18方法/
  所有映射、文本group/字号/字体/quotes/comments/escape/Unicode、数学嵌套、Basealiases、
  color/html转发、新宏与let、错误恢复与parser借用清空、creator定义和node lists迁移。
- **下一具体行动**：BBM/Bboldx/Dsfont。Root已完整读原三个Configuration.ts；BBM与Dsfont
  本来各单配置，Bboldx原Methods/Mappings/Configuration三组，后两原TS和当前全部CPP/IXx
  尚未读，下一轮先读完再冻结接口/分工。BBM ChooseFont复用BaseMathFont，mathversion改
  jax.options.bbm.bold跨式保留；Dsfont按sans选-ds-ss/-ds-rm。Bboldx priority3覆盖mathbb，
  text-bboldx通过新Data子配置接入（同Textcomp），要验数学/文本字体、bfbb/light与覆盖次序。
  已验收Textmacros/Textcomp与上一批包不重派、不重测；全profile/最终SVG gate仍待完成。
  原13项无关修改仍保护。完整38包/Common→Scene/SVG对照/Folia切换未完成，goal active。

### 上一批基线（2026-10-20 Braket/Units；历史证据，不重做）

- **当前无live构建/测试/代理**。完整Math回归570项/139,762断言，115套suite，无失败/
  跳过，exit0，braket-units-full.log；96246已终态。构建98047 exit0（并发4），
  braket-units-build.log。聚焦Braket8/323、Units6/135均exit0，*units*另匹配旧测试不算新增。
- **提交**：5306baf4 Braket（14文件，原四组8文件+3共享+测试/Source/CMake），
  b96122de Units（5文件，原单组+测试/Source/CMake）。无平行实现；原13项无关修改保留。
- **Braket职责**：Root完整读取原4TS和原8CPP/IXx；冻结原四组文件职责。
  items/methods与mappings/configuration均原位替换；共享仅增加
  TexEnvironment::braket_item_depth（optional<size_t>，overlay复制）及CloseItem::braket_bar。
  GPT-6.1 Sol /root/base_methods交付items/methods CPP与测试/Source四文件；Root接管并
  独立重放Source85实际compile/17错误类/11bar场景/6种over分段及直接factory/items检查
  exit0，braket-source-final.log，原4TS+4CJS/full38锁。其余共享/注册/构建/验证/Git由Root做。
  源码算法保留：先验证参数再恢复scanner，Push后写当前item.env depth；非stretchy不增加
  barcount（原行为），只stretchy保存CLOSE/BINbar/OPEN并累加；标记Close解决over分段，
  toMml(false)仅返回当前段，不加围栏；不增JS对象语义或平行布局。验收实际所有映射、
  single/braced/nested、font环境、单双bar/限数/低优先回退、over前中后、creator/let/错误恢复。
  Source特殊\\Braket{a|{b|c}|d}报ExtraCloseMissingOpen，不能改成成功；mspace省略text在
  Source callback为undefined/native固定接口为emptyview，均落到空mspace无Text子节点。
- **Units职责**：Root完整读原UnitsConfiguration.ts及现旧CPP/IXx，原单配置内三个方法
  Unit/UnitFrac/NiceFrac直接操作scanner或创建现有MML mfrac，不增布局算法/包文件。
  使用已存在UnitsOptions(loose/ugly)，原两个CPP/IXx和tests/math/units.cpp六组已验收。
  Source witness Root从C:/Windows/512MB实跑35compile/6错误类exit0，锁原TS+CJS/full38。
  loose=true原~生成NBSP mtext，false原\\,生成0.167em mspace；0字符串参数仍真值。
- **下一具体行动**：Textmacros/Textcomp。Textcomp只读了原configuration和当前配置，
  未更改；实际依赖Textmacros服务，不能只接数学
  映射就宣布整包完成。Root另读原TextcompMappings.ts前95行及当前methods/mappings、
  原TextMacrosConfiguration.ts/TextParser.ts全部、现四个TextMacros头及旧configuration.cpp前190行。
  当前TextMacros八文件实际平铺在input/tex/（text_parser、textmacros_methods等），不在
  textmacros子目录；不新造同名平行文件。剩余原Methods/Mappings和现CPP需读完再冻结。
  文本子配置需要同一session/options与独立ParseOptions/Tags，主TexParser只能按作用域借用；
  TexSessionState当前明确只拥有可持久值，不能塞入请求parser/MML或复活Context对象图。
  原TextParser的Parse/Push/saveText/PushMath/addAttributes/ParseTextArg/ParseArg/copyLists
  等职责继续在原TextParser；未冻结下一批接口，需先读完原Methods/Mappings及现CPP再派工。
  上两批已验收包不重派、不重测；完整38包、Common→Scene emission、端到端SVG对照与
  Folia切换仍未完成，goal保持active。Mermaid暂停；Luna额度到10月25日，无新信号不重试，
  必要Sol只GPT-6.1 Sol；编译一次一个build、并发4。FunctionList命名已解决，不重新调查。

### 上一批基线（action/centernot/fontsizev3；历史证据，不重做）

- **当前无live构建/测试/代理**。完整Math回归556项/139,304断言，113套suite，无失败/
  跳过，exit0；action-centernot-fontsize-full.log，16303已终态。构建72843 exit0，
  action-centernot-fontsize-build.log，并发4。聚焦新增Action7/180、Centernot4/114、
  Fontsizev3 4/94均exit0；*action*还匹配既有fraction等名称，不把该重叠算作新增测试。
- **本批提交**：a964f49c Action、160520e6 Centernot、3f5eb5f8 Fontsizev3，各5文件。
  每包原configuration.ixx/.cpp原位替换、各原生测试/Source witness/CMake注册，无平行实现。
  原13项无关修改保留。GPT-6.1 Sol /root/base_methods完成Action；Root处理其余两包与共享/
  独立Source复放/全部原生验证/Git。Source均exit0：Action42次compile/11错误类，
  Centernot19次，Fontsizev3 18次；各原TS+CJS与完整38profile锁；对应各source-final.log。
- **冻结职责**：Action保留原2方法与BaseMethods::Macro constexpr别名、原3映射，
  不另造Methods文件。Toggle逐个局部parser同环境解析，到精确endtoggle终止；零分支
  creator请求为空，而MML继承后补空子节点。Mathtip两个ParseArg，texttip原二参数模板。
  Centernot保留CenterOver原两次base解析和filterCenterOver，注册centerOver真实node list；
  后处理复制子节点TeX类到六层物理parent外TeXAtom，用NodeUtil::setProperties保证working
  texClass与属性同时更新；不另加overlay模块。Fontsizev3直接复用BaseMethods::SetSize
  原10映射，保留tiny/Tiny和全部原版本比例，无新尺寸算法。Source实测连续tiny/large
  的creator调用顺序为1.2em再0.5em（退出栈时内到外），不是树的外到内顺序；测试已更正。
- **历史下一步Braket现已完成**，见顶部当前批次；上一批三个包不重派、不重测。
  共享布局→Scene emission、完整38包、端到端SVG对照和Folia切换仍未完成，goal保持active。
  现有Scene提案尚未采纳，不从头另造Common布局；Mermaid继续暂停。Luna额度到10月25日，
  无新信号不重试；必要Sol只GPT-6.1 Sol。
- **最新命名反馈已经核对**2022cb1a/e6126948：function_list测试与生产owner均原名，
  旧bindings尚待退。不重做已完成命名清理，不宣称旧消费者已经清空。架构文件已有禁止
  native_/typed_/new_替代版本命名、原位替换与清理旧注册的规则；不得仅改名隐藏遗留实现。

### 上一批验收（enclose/cancel/html；保持其证据，不重做）

- **当前无live构建/测试/代理**。最新完整Math回归541项/138,916断言exit0，无失败/跳过，
  `enclose-cancel-html-full.log`，76606已终态。构建60610 exit0，并发4，最终
  enclose-cancel-html-build-final.log；96930因UT两个u16string_view的==重载歧义exit1，
  Root改ranges::equal后重建通过。新聚焦enclose5/76、cancel5/89、html7/377全通过；
  *cancel*同时匹配两组enclose用例，不把该重叠额外计入新测试总数。
- **提交**：fa692c96（8文件）enclose/cancel原配置两组+测试/Source；3d5c31e5（7文件）
  HTML原Methods/Configuration两组+测试/Source。Root独立重放enclose/cancel36实际compile，
  html113实际compile/76非法属性名均exit0，原TS/CJS+完整38profile锁。源日志分别为
  enclose-cancel-source-final.log/html-source-final.log，Node堆512MB。
- **已验收职责**：Enclose/Cancel共用原ParseUtil keyval和原七项ENCLOSE_OPTIONS静态表；
  各保留原Configuration单owner。HTML原两组文件（本无Mappings）含原五方法、真实mrow
  构造/children迁移/NodeUtil.copyAttributes；属性留同一MML树，不加输出旁路。Source的
  oldStyle.charAt(新style.length-1)索引必须保留。MmlToken class=true会形成'true outer'；
  style真值非text原抛TypeError，nativeinvalid_argument测试已捕获并验证parser借用清空。
- **下一具体行动**：Root已完整读ActionConfiguration.ts及原旧CPP，原Action单组保留
  Toggle/Mathtip及constexpr BaseMethods::Macro别名；先写冻结configuration.ixx，再派已有
  GPT-6.1 Sol /root/base_methods限定原action/configuration.cpp、tests/math/action.cpp、
  tools/math-port/tex-action-source.cjs。Toggle逐个GetArgument到\\endtoggle，在同环境/
  configuration下局部TexParser解析；Mathtip两个ParseArg；原3map（texttip原2参数模板）。
  验收实际TeX child顺序/空参数/嵌套/环境继承/终结符与缺参恢复/creator/固定let callbacks。
  Root可并行处理已完整读TS的centernot、fontsizev3原单组；开始前还须读各自现CPP/IXx。
  不重派已完成包或重新查已解决命名；保留原13项无关修改。
- **后续核心断点已确认但未改代码**：CommonWrapper/LayoutJob仍没有Scene emission API；
  已完整读旧draft common-scene-emission-contract/contract.md及两header提案（不是已采纳）。
  当前Common算术/字体类在原文件内，须复用；SVG/Scene纯consumer已有，但全TeX→Scene仍缺。
  Scene Paint.dash_array已存在，旧提案该缺口已过时；FontMetrics.cap_height仍是无Source依据
  的未消费字段，需在正式emission接口冻结时处理，不能随意填x-height。未新增Scene接口/布局类。
  Unicode/Entities也未改，原UnicodeCache实际状态归属和纯Entities依赖尚待冻结；
  noerrors不在38包profile，不增加。全38包/共享布局→SVG对照/Folia切换仍未完成，goal保持active。
  Luna额度到10月25日，无新信号不重试，必要Sol只GPT-6.1 Sol；Mermaid暂停，编译并发4。

### enclose/cancel/html批次详细证据（历史，已完成；不是待办）

- **无live构建/测试/代理**。最新完整Math回归524项/138,374断言exit0，无失败/跳过，
  `bbox-keyval-full.log`，92898已终态。构建92056 exit0（并发4）。BBox6/209，keyval8/215
  聚焦exit0。Source Root独立重放BBox50compile/keyval33parse+5validators均exit0，
  原TS/CJS+完整38profile锁；对应bbox-source-final.log/keyval-options-source-final.log。
- **分批提交已完成**：d28a6e4b原ParseUtil共享keyval实现/8测试组；76ca3958原BBox单组
  配置方法/6测试组。既有ParseUtil数学算法未改，只在同owner加原helper和验证器；BBox
  仍原配置单组，未拆平行methods/maps模块。两个提交各5文件；原13项无关修改保护。
- **当前行动**：Root已完整读原EncloseConfiguration.ts/CancelConfiguration.ts及旧CPP。
  两包原configuration.ixx已冻结并写入typed接口；已派已有GPT-6.1 Sol
  /root/base_methods：只限原enclose/cancel各configuration二文件、tests/math/enclose.cpp、
  tests/math/cancel.cpp、tools/math-port/tex-enclose-cancel-source.cjs（共7文件）。Root掌握
  头接口/所有共享/CMake/独立Source复放/构建与提交；代理不原生build/test、不再派工。
  不重派已完成AMS/AMScd/extpfeil/BBox/keyval，不重查已解决FunctionList命名。
- **冻结职责**：Enclose原单owner导出ENCLOSE_OPTIONS七项（data-arrowhead,color,mathcolor,
  background,mathbackground,data-padding,data-thickness），用KeyValueOption固定表，type均
  nullopt，返回typed AttributeSet；EncloseMethods::Enclose仍读notation→brackets→ParseArg→
  原parse_util::keyvalOptions→覆盖notation→menclose。Cancel原单owner有Cancel/CancelTo，
  复用同ENCLOSE_OPTIONS和公共parser，原4命令表，固定notation字符串是原MathML属性数据。
  初始化接口均initializeModule(ConfigurationHandler&,MapHandler&)，Cancel装配原Enclose依赖。
  Enclose导出extern const array<KeyValueOption,7> ENCLOSE_OPTIONS，Methods为struct静态方法，
  两方法均MacroHandler签名；Cancel同样struct含两静态MacroHandler，不改头接口。
- **Root并行HTML包**：完整读原HtmlMethods/HtmlConfiguration TS及原四CPP/IXx；保持原两组
  文件（本来无Mappings文件，不新增）。已写5typed方法+GetArgumentMML/属性名检查helper，
  配置原5命令Map，复用keyvalOptions和NodeUtil.copyAttributes，正在写测试/Source。
  Style保留原oldStyle.charAt(新style.length-1)索引；MmlToken class=true真实可达，Class
  拼接须处理已有typed scalar；Style真值非字符串原Source抛TypeError，C++用invalid_argument。
  Unicode暂未改：原UnicodeCache global实际状态归属待定，原Entities旧模块仍带Property/
  Promise，不能直接导入新图；noerrors不在38包profile，不增加该包。没有上述包新文件/修改。
- **验收**：Source+native实际TeX覆盖4cancel命令、cancelto参数顺序、notational逗号替换、
  七允许属性/忽略其它属性/嵌套braces与转义值、两包独立选择/组合嵌套、上标padding、
  configured node creator原请求和错误恢复；不复制keyval解析。Source验证Node堆512MB，
  MML身份用bool比较，不直接assert.equal两个循环图对象；逻辑Parent会跳过部分mstyle。
  Luna额度到10月25日，无新信号不重试，必要Sol只GPT-6.1 Sol。Mermaid暂停，并发4。
  全38包/共享布局→SVG对照/Folia切换仍未完成，goal保持active。

### bbox/keyval批次详细证据（历史，已完成；不是待办）

- **上一批已验收**：amscd/extpfeil完整510项/137,950断言exit0，23528已终态，
  `amscd-extpfeil-full.log`；最终构建17337 exit0（并发4），聚焦amscd7/302、extpfeil6/234。
  Source Root独立复放amscd39compile、extpfeil60compile均exit0，原TS/CJS+完整38profile锁。
  已分批提交523805c4（原AMScd三组文件/Base尺寸字段/测试）、3a73fc54（原extpfeil单组）。
- **重要新细节**：generic array中的@没有CD尺寸，但Source仍写显式无值minsize，isSet=true；
  CommonMo.getSize经length2em(undefined,1,1)得1，不能省略成mo默认0em。原AmsCdMethods现
  用已有AttributeNull表达显式无尺寸，Common长度转换同为1，无新JS对象/类型。空水平标签
  在generic array仍原样形成缺尺寸kern并报MissingDimOrUnits，不能改成正常CD默认宽度。
- **完整回归已结束exit0**：92898（bbox-keyval-full.log），524项/138,374断言，
  最终构建92056 exit0（bbox-keyval-build.log，并发4）。BBox新6组/209断言通过，
  聚焦*bbox*另外匹配了既有Common BBox用例；keyval8组/215断言exit0。
  Root完整读了原BBox/Cancel/Enclose TS及旧CPP，
  ParseUtil readKeyval/removeBraces/readValue、keyvalOptions、KeyValueDef/KeyValueTypes原算法。
  Root负责bbox原configuration二文件、tests/math/bbox.cpp、tex-bbox-source.cjs及所有共享
  接口/CMake/验证/提交；bbox原pair+6组测试已写，Source50compile从C:\Windows独立重放
  exit0，`bbox-source-final.log`。已统一构建/聚焦和完整回归验证。
  BboxMethods::BBox和initializeModule(registry,maps)头接口已落地。
- **共享选项职责冻结在原ParseUtil**：KeyValueDef持原name和编译Verify/Convert函数指针，
  oneof仅拥有允许字符串vector；KeyValueTypes保留boolean/number/integer/string/dimen。
  KeyValueOption是key+optional<KeyValueDef>，keyvalOptions返回原MML AttributeSet，
  optional<span> allowed区分不筛选与空允许集，error/l3keys保持Source。Root已写原.ixx声明。
  已派已有GPT-6.1 Sol /root/base_methods限定原parse_util.cpp、tests/math/keyval_options.cpp、
  tex-keyval-options-source.cjs；代理不改头/共享/CMake/docs/Git，不原生构建/测试/再派工。
  不新增generic对象字典、registry或另一套选项解析器。enclose/cancel尚未派工，先共用层。
  代理已确认API足够，公开number.convert需保留parseFloat数值前缀行为，允许在原CPP
  匹配数值前缀再交js_input::number，不加共享数值API或模块。Source验证建议Node堆512MB。
  Root bbox初次验证逻辑Parent断言错误导致Node格式化巨大循环MML图，已只终止70852
  （43008终态-1），修正为物理parent.parent和bool身份比较后Source50compile exit0。
  Mstyle逻辑Parent可能跳过单子节点，不能用于此包装次序断言；不再直接assert.equal两个MML对象。
  代理已交付并冻结：原CPP新增helpers/类型，既有算法未改；8组native测试；Source33parse/
  5标准validator及public convert/oneof/custom用例，Root从C:\Windows独立重放exit0，
  keyval-options-source-final.log（堆512MB，TS/CJS+完整profile锁）。Root逐段复核后构建，
  代理额外只读复核BBox未发现具体差异，现无live代理。原CPP现约680行，保留原owner。
  原13项无关修改继续保护；全38包/共享布局→SVG/Folia仍未完成，goal保持active。

### amscd/extpfeil批次（历史，已完成；不是待办）

- **最新命名反馈的既有结论**：`native_function_list.cpp` 已在2022cb1a归回
  `tests/math/function_list.cpp`；生产唯一实现是原 `util/function_list.ixx`。
  e6126948已删除旧对象语义测试/配置投影。旧 `function_list_bindings` 文件仍有未迁移
  消费者，尚未完成退场，不能称为全量清理。没有新证据时不要因历史消息再次重开命名调查。
  后续生产文件沿MathJax原模块职责原位改造，不用native/new/typed等前缀建立平行实现；
  删除旧实现需连同消费者和构建引用处理，不能只改名掩盖遗留职责。本次仅复核此既有结论，
  未改算法、未重复运行测试；amscd/extpfeil仍在下面记录的断点，代理尚未重新派工。
- **无live构建/测试**；上一批完整497项/137,414断言仍为最新验收。Root已完整读原
  AmsCdMethods/Mappings/Configuration、ExtpfeilConfiguration及当前旧CPP；已改amscd三.ixx
  为typed接口，Base ArrayItem新增两个optional<string> cd_min_width/cd_min_height，其余尚未改。
- **冻结职责**：amscd保持原methods/mappings/configuration三组文件，5原方法（CD为env builder，
  arrow/cell/minCDarrowwidth/minCDarrowheight为MacroHandler）；3原映射、原AmscdOptions。
  CD仍创建原ArrayItem，环境尺寸在打开时捕获到ArrayItem两字段，不另建CD帧/对象状态层。
  cell在非Array顶层须按Source把缺席table/row视作空，再Push strut/cell让原栈产生Misplaced&，
  不能dynamic_cast引用抛bad_cast。arrow保留fallback Other的位置恢复、奇偶行/列补格、标签布局。
- **Root负责**：amscd三组原生产文件、Base字段、tests/math/amscd.cpp及tex-amscd-source.cjs，
  CMake、Source独立复放/统一原生验证与分批提交。AMS已验算法不改、不复制。
- **准备复用已有GPT-6.1 Sol /root/base_methods限定4文件**：原extpfeil/configuration两文件、
  tests/math/extpfeil.cpp、tex-extpfeil-source.cjs。原单owner含NewExtArrow方法、xArrow编译别名、
  原6命令Map及配置；复用AmsMethods::xArrow/NewcommandConfig/NewcommandUtil，定义使用现
  NativeMacroAlias固定callback ID和3个numeric TexArgument，不引运行时函数对象或新registry。
  代理不得改共享/CMake/docs/Git、不得原生构建/测试或另派代理；缺口报告Root，不造替代API。
- **验收**：完整Source TS/CJS/full38profile锁；amscd方向/单双标签/空格/尺寸捕获/隐藏标签/
  奇偶表格及错误恢复；extpfeil 5固定箭头+Newextarrow decimal/hex、参数错误、scopes/global、
  alias与跨式/reset、configured creator前属性和复用原箭头布局。先聚焦再完整回归，并发4。
  完整38包、共享布局→SVG对照、Folia切换仍未完成，goal保持active。原13项无关修改保护。
  Luna额度到10月25日，无新信号不重试，必要Sol仅GPT-6.1 Sol；Mermaid暂停，不reset改历史。

### 上一已验证提交批次（历史；当前以顶部断点为准）

- **完整Math回归497项/137,414断言exit0**：`ams-full.log`，64538已终态，无跳过/失败。
  构建97088 exit0（ams-build.log，并发4），AMS聚焦12项/355断言exit0：Methods9项/259，
  Items/配置3项/96（ams-focused.log）。当前无live构建/测试/代理；保留原13项无关修改。
- **分批提交**：6b05b950（6文件）原Methods/Items；2255190f（6文件）原配置/映射与生成器；
  dc2778a5（4文件）真实AMS方法/结构测试与Items Source。生产原八文件净减少359行，无平行模块。
- **已验收原职责**：Root已完整读原
  AmsMethods.ts（全部25成员+2 helper）、AmsItems.ts、AmsConfiguration.ts及Mappings开头，
  当前原CPP/共享接口。原四.ixx/.cpp和tests/math/ams.cpp、ams_items.cpp已进入CMake；
  上述实际构建/聚焦/完整回归均已覆盖这些文件，不重新审计已验功能。
- **冻结职责**：Methods原15实现（5 EnvironmentBody、9 Macro、1 TokenInput）+10 BaseMethods
  constexpr函数别名，不复制基础算法；helpers splitSideSet/checkSideSetBase仍同methods.cpp。
  Items原MultlineItem:ArrayItem、FlalignItem:EqnArrayItem；Multline构造(factory,bool)，
  Flalign构造(factory,u16name,bool numbered,bool padded,bool center)，继承原name/numbered，
  新字段仅optional<double> xalignat（保留NaN分支）、bool zeroWidthLabel/padded/center，
  ArrayItem原shove已存在。parser_types已有全部StackKind及Multline/FlalignItemInit，不改共享。
- **Root已实现**：四.ixx；items.cpp、configuration.cpp、mappings.cpp及生成工具/原旧inc
  退出、CMake、Items/配置/表测试、Source独立复放与最终验收。配置复用NewcommandConfig和
  AmsTags:AbstractTags，items直接按真实typed init创建，映射完整原9表；不新增平行文件。
  旧ams_mapping_data.inc仅为构建产物，无tracked文件；旧include与Property生成算法已原位删除。
  原generate-ams-mapping-data.cjs现捕获未解析前原Source的9表/311条目，直接生成原mappings.cpp，
  注册SHA ffad7e5d055ba41cec9827a7327c7dc886781aedad4172c94d73bbef07fe5aaf，
  --check已从C:\Windows exit0，ams-mappings-check.log，TS/CJS+完整38profile锁齐。
- **已有GPT-6.1 Sol /root/base_methods已完成冻结限定3文件**：原ams/methods.cpp、
  tests/math/ams.cpp、tools/math-port/tex-ams-methods-source.cjs。不改.ixx/共享/CMake/docs/Git，
  不跑原生构建/测试，不另派代理；已交付15实现/10别名和9组真实TeX用例后结束。Root逐段
  复核并独立Source重放：最终73compile exit0（ams-methods-source-final.log），源锁齐全。
- **Root额外Source**：tex-ams-items-source.cjs配置/两项/9组行形状和3种列错误共12compile
  exit0（ams-items-source-final.log），3组原生测试ams_items.cpp覆盖相同结构与装配。
- **两处重要修正，不能重查或改回**：Flalign Source父EqnArray无args仍start空标签层，
  子构造再start实际环境；因此结束后stack保留1层，下一式startEquation清理。Root已保留原
  两次调用，typed TagInfo值状态，不是对象模拟。HandleOperatorName最初只设explicit_tex_class
  记录，working texClass未变；Root改NodeUtil::setProperties同步真实OP并补单mi/TeXAtom
  实际texClass测试，Source已验证（73例）。没有改Source预期迁就C++。
- **准确下一步：amscd与extpfeil**。本轮仅读原AmsCdConfiguration.ts及ExtpfeilConfiguration.ts
  imports/开头，尚未修改。amscd原三组methods/mappings/configuration文件，extpfeil原单组
  configuration（含原Methods和Map）；先读完Source与已有C++，冻结接口与Source验收再分工。
  复用本轮原AmsMethods::xArrow、NewcommandConfig/定义层、既有TexEnvironment.cd_min_width/
  cd_min_height和AmscdOptions，不另造运行时/状态层。实际缺共享字段再按原owner补充。
  明确核对每一步exit_code，不构建时改写输入。根本目标仍完整38包、
  共享布局→SVG对照及Folia切换，goal保持active；原13项无关修改保护。Mermaid暂停、无D2D像素门槛。
  Luna额度到10月25日，无新信号不重试，必要Sol仅GPT-6.1 Sol；不reset或改写历史。

### 上一已验证提交批次（历史；当前以顶部断点为准）

- **完整Math回归485项/137,059断言exit0**：`verb-upgreek-gensymb-full.log`，2160已终态，
  无跳过/失败。统一构建62996已exit0，并发4（verb-upgreek-gensymb-build.log）；
  verb聚焦6项/95断言、upgreek4项/650断言、gensymb4项/127断言均exit0。
  当前无live测试/构建/代理；工作区仅原13项无关修改，继续保护。
- **按包提交**：76d98fe6（5文件）verb；e56c38d7（5文件）upgreek；4dfefdf2（5文件）gensymb。
  各包原Configuration owner原位替换旧对象层，未新增Methods/Mappings平行文件。
- **Root拥有原verb/configuration两文件**：原位替换成原VerbMethods::Verb+CommandMap+配置；
  没有另拆Methods/Mappings模块。复用TexParser扫描、NodeFactory token(mtext,monospace)。
  新tests/math/verb.cpp 6组、tex-verb-source.cjs，CMake只加入verb，不含代理仍编辑的文件。
  首构建29465终态exit1：测试pair不能从两string_view隐式构造，已改分别赋值；重建终态
  exit1因dispatch_token的array CTAD生成string_view而TexError要求string span，已显式string。
  两处均仅C++类型转换错误，未发生原生进程异常退出。
- **Source已独立通过**：Root从C:\Windows重放verb20次真实编译exit0，verb-source.log，
  TS/CJS及完整38profile锁。空内容保留一个空TextNode，只有U+0020转NBSP，空白前缀跳过；
  Source GetNext返回完整码点但charAt比较单UTF-16单元，补充字符分隔符报NoClosingDelim，
  旧C++误用front可接受它，已按Source原扫描修正。错误分别用dispatch_token/currentCS。
- **已有GPT-6.1 Sol /root/base_methods已交付冻结限定8文件**：原upgreek/gensymb各
  configuration两文件、各tests/math/{upgreek,gensymb}.cpp和各tex-*-source.cjs。
  职责已冻结：原41Greek/5单位表和原名typed TokenInput回调，复制Token attributes后覆盖normal，
  gensymb另MathML-Unit，ohm U+2126；不改ParseMethods、不另建模块，不构建/改共享/Git。
  Root已纳入CMake并格式化、逐owner审查；Source由Root独立从C:\Windows重放128/20compile
  exit0（upgreek-source.log/gensymb-source.log）。Root校正Source的let/bold公式使逐字匹配
  C++用例（Upomega、micro、degree），并删upgreek空unit()及两包无用MapHandler导入。
  代理只读复核Root Verb未发现差异，已结束，未编辑/构建/测试。
- **准确下一步：AMS**。本轮仅只读了原AmsConfiguration.ts、AmsMethods/AmsItems的imports
  和AmsMappings头、当前mappings.cpp；未改任何ams生产文件。先读完原四职责及现共享接口，
  再冻结typed接口/文件职责/Source验收后分工。配置复用已验NewcommandConfig与AbstractTags，
  Items保留MultlineItem/FlalignItem继承原ArrayItem/EqnArrayItem，Methods保留原数学算法，
  Mappings保留原9表。现mappings.cpp引ams_mapping_data.inc（旧Property表），需按原owner
  生成typed表并移除被替代的旧表/注册，参考已验BaseMappings做法，不创建第二套实现。
  检查StackKind/StackItemInit、tags/array字段是否需要原owner扩展；不要凭旧summary猜API。
  完整38包、共享布局→SVG对照、Folia切换仍未完成；goal保持active。原13项修改保护。
  Luna额度到10月25日，无新信号不重试；必要Sol仅GPT-6.1 Sol。Mermaid暂停，无D2D像素门槛。
  不重复FunctionList命名/Stack/BaseMethods已验结论，不执行reset或改写历史。

### 上一已验证提交批次（历史；当前以顶部断点为准）

- **完整Math回归471项/136,187断言exit0**：`configmacros-noundefined-full.log`，28086已终态，
  无跳过/失败。构建18050/70852/84061均exit0，最终configmacros-noundefined-final-build.log，
  并发4；configmacros聚焦7项/50断言、noundefined聚焦5项/55断言均exit0。无live测试/构建/代理。
- **提交**：2f590193（5文件）原configmacros包；457aa14f（5文件）原noundefined包。
  生产更改原位删去旧对象层实现，均无新增平行模块；仅保留原13项无关修改。
- **已验收职责**：原configmacros/configuration两文件由现有GPT-6.1 Sol base_methods原位
  替换，保留原init三map/local priority3、active→macros→environments配置、BeginEnvItem。
  maps拥有Macro值，只使用现TexOptionValues定义和原展开/环境函数，不写session定义层。
  该代理交付对应tests/math/configmacros.cpp与tex-configmacros-source.cjs后结束。
- **Root负责**：原noundefined/configuration两文件原位替换及tests/math/noundefined.cpp、
  tex-noundefined-source.cjs；唯一macro fallback priority3，先配置text creator再mtext node，
  仅非空样式，无token creator。两包均已进入CMake；无平行模块或旧对象层依赖。
- **独立Source已过**：Root从C:\Windows重放configmacros 3maps/22compile及noundefined
  11compile，均exit0；TS+CJS和完整38profile锁齐全。日志configmacros-source.log、
  noundefined-source-final.log。代理只读复核两包后未发现具体差异，无额外编辑/构建。
- **测试修正依据**：首次noundefined构建97401因3_d的UT literal递归常量求值失败，已改3.0。
  原生聚焦2断言误把缺少显式mathbackground/mathsize当有效值为空；诊断70852构建后证实
  实际为transparent/normal，与Source一致。测试已分开验证显式缺席和有效默认值；Source也
  加相同验证并通过。未更改生产算法迁就测试，未发生原生异常退出。
- **准确下一步**：继续原verb/configuration两文件；本轮只读原VerbConfiguration.ts，尚未改。
  原职责含Verb方法+CommandMap+Configuration，没有独立Methods模块，不另拆文件。
  保留GetNext/UTF-16分隔扫描、普通空格转NBSP、monospace、MissingArgFor/NoClosingDelim及
  当前控制序列错误消息；先做Source及真实TeX测试，再原位替换并接CMake。后续upgreek/gensymb。
  完整38包、共享布局→SVG对照、Folia切换仍未完成，goal保持active。保留原13项无关修改。
  Luna额度到10月25日，无新信号不重试；必要Sol仅GPT-6.1 Sol。Mermaid暂停，无D2D像素门槛。
  不重复已处理FunctionList命名/Stack/BaseMethods审计，不执行reset或改写历史。

### 上一已验证提交批次（历史；当前以顶部断点为准）

- **完整Math回归459项/136,082断言exit0**：`begingroup-color-full.log`，34098已终态。
  当前无live测试、构建或代理。原13项无关修改继续保护，工作区其余本轮改动已提交。
  85464因测试UTF-16输出编译失败已终态，比较改ranges::equal；53433/61952重建均exit0，
  最终begingroup-color-final-build.log，并发4。begingroup聚焦9项/69断言、color7项/87断言、
  newcommand10项/94断言（另匹配原begingroup一项4断言）均exit0。
- **提交**：01803f6f（10文件）原begingroup包及实际scope service/newcommand测试接通；
  958417dd（7文件）原color包方法/配置与实际TeX用例。旧生产实现原位替换，无平行模块。
- **命名问题已处理，不重新调查**：本次最新消息再次提及native function list，已核对
  2022cb1a/e6126948及当前文件：生产owner为util/function_list.ixx，测试function_list.cpp，
  无native_function_list并行模块；旧bindings消费者未全退，仍须清理，不能宣称完成。
- **已验收职责**：ParseOptions新增唯一 `unique_ptr<BegingroupStack> begingroup`，
  配置真实装配service，借用现session.definitions；原Stack算法文件未改，未复制定义。
  NewcommandUtil.checkGlobal使用该service，newcommand测试已移除测试scope命令，换正式包。
  color沿原Methods/Configuration直接实现5方法与padding，config装配/重置同一session ColorModel；
  两包不走旧MapContext/Property/FunctionRef；完整回归已过，不代表完整38包已接通。
- **代理**：已有/root/base_methods（GPT-6.1 Sol）交付begingroup限定6文件（原4生产文件、
  tests/math/begingroup.cpp、tex-begingroup-package-source.cjs），9组真实TeX用例，已冻结。
  Root独立从C:\Windows重放Source 6members/35compile exit0，3TS+3CJS/fullprofile锁。
  该代理已完成只读审查Root Color方法/config/padding，未发现具体差异，已结束；无live代理。
- **Color Source已独立通过**：原tex-color-source.cjs保留68constants/26model cases/实际lifecycle，
  扩展5方法实际包解析、10padding用例并补完整profile锁。Source纠正padding尾LF例预期：
  原5px\n得到width +105px\n，正则无m不匹配整个输入，原unit字符串保留；不是生产特判。
  Root负责color四文件、tests/math/color.cpp、上述Source扩展、共享service与CMake。
- **首begingroup聚焦失败已解释**：配置通用构造补齐character/delimiter/macro/environment四类；
  新测试误断言只有一类且front为macro。已按原Configuration语义分别验证4类及各自maps，
  原Source补Object.keys及三个空类独立验证exit0，35compile不变；生产代码没有为测试修改。
- **下一步**：按原职责继续configmacros/configuration，复用已验newcommand/token maps和
  TexOptionValues配置定义，不复制JS对象/描述符/函数语义。已读该原文件仍为旧Context实现，
  尚未修改；不要重新审计已完成的FunctionList、Stack和BaseMethods。Luna额度到10月25日，
  无新信号不重试；必要Sol仅GPT-6.1 Sol。完整38包、共享布局→SVG对照及Folia切换仍未完成，
  goal保持active。Mermaid暂停，无D2D像素门槛，不执行reset或改写历史。

### 上一已验收包接通批次（历史；当前live状态以上节为准）

- **原boldsymbol/newcommand包已接通，完整Math回归443项/135,926断言 exit0。**
  最终日志 `build/math-port/newcommand-full-final.log`，**83606已终态**。
  当前无live agent、构建或测试；工作区仅原13项无关修改，继续保护。goal保持active。
  完整38包、Common→Scene→SVG端到端对照和Folia切换仍未完成，不能据此宣称全移植完成。
- **本轮提交**：`3fccf8e8`（13文件）原boldsymbol配置/宏/后处理及可null文本creator；
  `a1b2a998`（2文件）修正遗漏文本的Source默认参数；`11fa87ac`（10文件）共享持久定义
  与TokenMap借用接口；`bab72b31`（14文件）原newcommand五对文件、CMake、实际测试/Source。
  没有新增平行实现或兼容旧对象层；newcommand生产代码原位减少，方法/utility各约350/250行。
- **原职责/关键语义已验收**：boldsymbol原creator、作用域恢复、真实fixBold列表、继承后
  priority5字体重写，嵌套/Greek/mathbb/mtext均验证。newcommand原7成员（Macro直接复用
  BaseMethods）、6命令映射、BeginEnvItem、扫描/保护/定义/配置分别在原owner内。
  TexSessionState拥有唯一命令/分隔符/环境定义，TokenMap借用；展开时捕获值防止重定义悬空。
  Macro/EnvironmentDefinition.arity为double，普通宏可超过9且不分配arity个delimiter；
  template才检查最多9与delimiter匹配。substitute_parameters保留显式[0]的替换/default语义。
  MacroMap有固定templateExpansion，NativeMacroAlias/CharacterMacroAlias仍用固定callback ID；
  全38包callback表尚未组装。MacroSuppression直接InvokeFallback；null delimiter生成空mo；
  BeginEnd识别用户环境结束，环境begin/end定义按Source解析。全局写入复用原BegingroupStack。
- **重要默认参数结论，不要重犯**：NodeFactory.makeToken的text是optional<string_view>，
  **缺省值是present-empty的string_view{}，不能用nullopt当缺省值**。
  Source createToken(text='')：遗漏文本/显式空字符串有一个空TextNode，显式null才没有。
  Root首次全回归77071因把缺省误设nullopt而在旧重绑定测试.front异常退出，已立即停止广泛
  测试；限定`*node factory*`复现，真实Source新增default/null/empty三分支后发现差异。
  已修生产默认值并保留旧.front断言，未放松测试。最终相关6项/35断言及完整443项都通过。
- **构建/原生证据**：Root构建81620/4896/20697均终态exit0，最终token-default-build.log，
  并发4。newcommand10项/94断言（newcommand-focused-final.log）；boldsymbol4项/76断言；
  定义2项/9断言、command map3项/20断言、null token1项/4断言聚焦均通过，已纳入完整443项。
  覆盖参数10/显式0、def前后模板与控制序列边界、let快照/原生宏/字符/分隔符/隐藏、环境
  嵌套/错误、跨公式/reset/独立jax、作用域global、宏次数限制及下一式恢复。
- **Root独立Source重放**：原tex-node-creators-source.cjs增加3组真实boldsymbol包解析、
  10字体variant及default/null/empty文本；tex-newcommand-source.cjs实际54次compile，
  五TS+五CJS及完整profile锁，均从 C:\Windows exit0。Newcommand native global fixture用
  固定测试命令调用实际BegingroupStack remove/finish等，**不等于begingroup包已接通**。
  不把一次包测试成功当作38包或最终SVG验收。
- **代理已结束**：已有/root/base_methods（GPT-6.1 Sol）仅实现newcommand限定12文件；
  Root定义共享接口并负责boldsymbol、Source重放、实际构建、聚焦及全回归。该代理最后只读
  对照共享Map/Macro/BeginEnd/NodeFactory边界，未发现其他具体差异，未再改文件。
  Luna额度到10月25日，无新信号不重试；必要Sol仅GPT-6.1 Sol。
- **准确下一步**：接通原begingroup的configuration/methods/mappings，复用已验BegingroupStack
  和本轮newcommand定义层；不重写Stack，不把现有测试scope命令当作正式包。随后color等原包，
  继续完整38包及共享输出到SVG。原13项修改、Mermaid暂停、无D2D像素门槛及命名约束均保持；
  不重新处理FunctionList旧命名已完成的提交，不执行reset或改写历史。

### 上一已验收输入链（历史；当前live状态以上节为准）

- **原 base 输入链已接通，完整 Math 回归430项/135,800断言 exit0。**
  最终日志 `build/math-port/tex-jax-full-final.log`；句柄 **75033已终态**。
  当前无 live agent、构建或测试。完整38包、TeX→SVG对照及Folia切换仍未完成，goal保持active。
- **本轮提交**：`4d3567d3`（6文件）补 RangeDef 的 Source 第五个variant、原字典生成器
  和 NodeUtil.copyAttributes；`6b1aeef5`（20文件）闭合原 TeX/BaseConfiguration/FilterUtil
  及配置阶段、CMake、实际测试和Source检查。没有平行parser，也没有新增 native/new/typed
  实现；core/input_jax纯模板无需.cpp，已删除旧.cpp。原tex_jax.cpp由878行旧对象调用
  改为约140行真实编译生命周期，后处理全部归原FilterUtil职责，不保留内嵌旧副本。
- **已验收职责**：core/input_jax使用 `AbstractInputJax<Options,Data>`，filter record借用
  真实MathItem/MmlDocument/ParseOptions；TeX拥有session/configuration/ParseOptions，
  每次compile绑定调用方document.factory、clear、pre、parser、tag、post和末尾hsize。
  仅TexError转换为错误MML；其他异常传播；ParserScope在异常时RAII移除parser借用。
  BaseConfiguration有Other/两回退错误/初始letter-digit优先级4/26items/BaseTags/nonscript。
  七个FilterUtil按-7至-1注册，nonscript同-4先于moveLimits；配置config/add真实可用。
  NodeUtil.copyAttributes复制完整attributes值和已有属性，**不复制intrinsic texClass**。
  CJK normal来自原字典第五字段，不在Other硬编码区段。
- **验收证据**：构建46824/97639/50986均终态exit0，最终`tex-jax-format-build.log`，并发4。
  原生base configuration 6项/43断言、FilterUtil 9项/128断言、动态add 1项/6断言通过。
  输入jax测试覆盖实际mathbb/分式/次数根式/脚本、用户与包filters顺序、错误格式器、hsize、
  false停止和64个独立MmlDocument复用同一jax时产品数稳定/列表清理，全部已纳入完整430项。
  首构建失败是测试打印optional值，改取.value；首聚焦失败是测试错误地从token.walkTree
  读取TextNode，改用token.getText，生产算法未因此更改。只读审查找出formatError三行消息
  差异，已按原`/\n.*/`无dotAll行为修正，并补LF/CR/U+2028/U+2029实际用例。
- **Root独立重放Source**：`tex-filter-util-source.cjs`七方法/24场景；
  `tex-base-configuration-source.cjs`8个真实TeX fixture；`tex-input-jax-source.cjs`6组
  生命周期/配置fixture（含多行错误）；均从 C:\Windows 执行 exit0，TS/CJS及完整profile锁齐全。
  `operator-data.cjs --check` exit0（27MO/50ranges/8spacing/1252operators/5patterns）。
  Source因构造异常保留parser引用的行为不复制到C++：原生使用已确定ParserScope RAII。
- **代理**：已有 `/root/base_methods`（GPT-6.1 Sol）完成限定FilterUtil、BaseConfiguration
  及各自测试/Source文件后结束；最后只读审查Root输入jax/config阶段，除上述已修格式化差异
  外未报告具体数学行为差异。Root负责实际编译、Source重放、聚焦及完整验收。
  Luna额度已知到10月25日，无新信号不重试；必要Sol仅GPT-6.1 Sol。
- **准确下一步**：继续原38包逐owner接到真实TeX配置，先处理已存在真实helper但配置/方法
  尚待输入jax的 `boldsymbol/configuration`，再闭合其他原包；不要从头重写已通过的base链。
  原Common→Scene输出、完整端到端SVG对照和应用切换仍需按完整目标完成。DOM查找/HTML适配
  不属于本次字符串编译服务，仍是显式待完成职责；不能伪造实现或调用已撤去的对象层。
- **命名与清理约束仍生效**：FunctionList早已归原名（2022cb1a/e6126948），不要重新调查。
  旧function_list_bindings及handler_context_bindings/parser_runtime/output_jax/common_bindings
  消费者尚未全清，退出当前CMake不代表删除完成。工作区只剩原13项无关修改，继续保护；
  Mermaid暂停，不加D2D像素门槛；不执行reset，不改写提交历史。

### 上一已验收批次（历史；以下不是当前 live 状态）

- **2026-10-20 当前断点：原 BaseMethods、BaseMappings 和所需共享 ParseUtil 已接入编译图。**
  最新完整 Math 回归 **409项/135,195断言，exit0**，`base-methods-full.log`，
  **69186已终态**。当前没有live agent、构建或测试。完整TeX→SVG/Folia管线仍未完成，
  goal保持active。上一轮CommandMap/ParseMethods的390项已包含在本轮409项中，不重跑旧审计。
- **本轮已分批提交**：共享依赖 **9a4ac75d**（11文件）；BaseMappings及生成器/CMake
  **f3656c4f**（4文件）；测试 **9115d2bc**（7文件）；BaseMethods实现 **04d44b41**
  （2文件）。拆分CMake暂存时一次--recount失败，后续步骤未及时停，导致映射/测试先提交；
  已立即补齐BaseMethods，当前HEAD完整，未改写历史。以后依赖前一步成功的tools调用必须
  检查exit_code再继续，不能把exec_command的exit1当作JS异常会自动中止。
- **原职责/实现**：base/base_methods保留全部75成员（70个MacroHandler、5个真实
  EnvironmentBodyHandler），旧FunctionRef/Property调用表已删除；base/base_mappings
  保留13表/597条目，直接引用编译函数。表在原.cpp中，未再增平行inc/包装模块。
  generator从原始BaseConfiguration加载后的未运行公式快照生成，SHA256
  0dd8a1605cc7736bd18ba36f1452b14d640cb5d7dae42689339fffed785566c4，--check已通过。
  原ParseUtil拥有internalMath/internalText/underOver/checkMovableLimits/setArrayAlign/
  checkEqnEnv/copyNode及请求参数扩展，复用原TexParser/Stack/NodeFactory/Unicode16。
  原NodeUtil负责getMoAttribute、固定数学属性与XML属性分流；EquationItem有实际name；
  env.size按Source唯一数字写入改optional<double>。原MML.copy保留脚本缺席槽、去id；
  ParseUtil.copyNode调用原copy()并重登列表，不能误换成exactclone。
  tests/math/parse_util.cpp归回职责名，旧parse_util_native.cpp已移走无重复。
- **验收证据**：生产构建35644已终态exit0，base-methods-build.log；测试构建56363、
  30876及最终80477均终态exit0，最终base-mappings-build.log，并发4。
  原生BaseMethods12项/80断言exit0，base-methods-focused-2.log；共享helper4项/59断言
  exit0，parse-util-focused-final.log；原映射3项/32断言exit0，base-mappings-focused.log。
  实际C++原表解析mathbb R（double-struck）、分式、次数根式、hbar variantForm、sum、
  escaped delimiter通过，但Folia尚未切换。首BaseMethods聚焦一项异常是测试构造宏定义
  漏掉两条必需的空delimiter，已修测试，未放松已定数据验证。
  正式tex-base-methods-source.cjs全75方法/40解析、TS+CJS锁+完整profile，Root从
  C:\Windows重放exit0，base-methods-source.log；tex-parse-util-source.cjs原helper及
  实际base parser/基础映射Source对照exit0，parse-util-source-final.log。
  该独立Source检查先纠正测试直接传null给setChildren、下标猜测；未改数学算法。
- **代理**：/root/base_methods为GPT-6.1 Sol，完成限定原文件和真实测试后结束；其对Root
  ParseUtil逐方法只读审查未发现具体语义差异。Root独立构建、重放Source、原生聚焦及完整
  回归。Luna额度已知到10月25日，无新信号不重试；必要Sol仅GPT-6.1 Sol。
- **准确下一步**：原base/base_configuration.{ixx,cpp}及BaseTags/Other/csUndefined/
  envUndefined、初始字母数字模式与filters改为实际配置；随后闭合tex_jax.{ixx,cpp}
  及input-jax/filter。**新证据必须处理**：Source OperatorDictionary.RangeDef有第五个
  可选variant（CJK等normal），现native RangeDef缺该字段；接Other时补原字典数据职责，
  不能硬编码字符区段到Other。Source初始letter/digit的config注册优先级4，常规character
  maps为command/special；macro顺序包含delimiter在前。Source脚本缺席槽只能在input-jax
  后处理完成后交给Common，不宣称Common已接受任意未完成的msubsup。
  后续完整38包、nested tag TeX、输入jax/postfilters、端到端SVG对照及Folia接入仍未完成。
  其他包旧API消费者尚未改写，继续按各自owner原位改写，不用兼容层或native/new/typed
  前后缀保留第二套实现。不要回头重审已通过的列表/配置/帧所有权问题。
- **保留/执行约束**：工作区剩余原13项无关修改继续保护；Mermaid暂停，不加D2D像素门槛。
  单次构建并发4、不并行堆构建；控制台测试链接既有runtime，异常退出停止广泛评估；
  只用正常本地软件测试术语。FunctionList已归原职责名，旧bindings层的未改写消费者
  仍在；退出CMake不是清理完，禁止据此称旧对象语义全图已退掉。

### 上轮已完成检查点（历史；本轮以顶部执行断点为准）

- **2026-10-20 当前断点：用户再次指出 `native function list` 命名及旧文件残留。**
  先前提交 `2022cb1a` 已把 `native_function_list.cpp` 归回 `function_list.cpp`，
  生产实现一直在原 `util/function_list.ixx`，没有第二个 native FunctionList 模块。
  本轮清理提交 **`e6126948`**。读完旧 `function_list_native_argument_values.cpp`：只验 Context/ObjectRef/any
  参数编解码与旧身份注册，未参与当前构建，已删除，不把它换名继续保留。
  正常 FunctionList 的优先级、void/bool 回调、执行中增删、异常传播测试仍在原职责测试内。
  旧 utility bindings 的未改写消费者仍存在；不能声称旧对象语义图已全部清完。
  后续文件按原 MathJax 模块职责归位，不以 native/typed/new 前后缀保留第二套实现。
  同轮删除已无生产调用的 `configuration_prioritized_list_projection.cpp` 及对应三份
  旧 Context 测试；其 replaceable array/descriptor/record 身份规则不属于数学配置职责。
  正常包顺序、合并、init 动态追加及真实共享引用生命周期由原配置类聚焦测试承担；
  仍未完成的 config/add 并不因删除旧反射测试而算通过。
- **本轮配置实现已提交 `cc845a34`**：原 `configuration.{ixx,cpp}` 已原位改为真实
  Configuration/ConfigurationHandler/ParserConfiguration；借用现有 MapHandler/session/
  callbacks，使用已验 typed options、NodeCreators、FunctionList，不另造配置门面。
  已实现记录规范化、具名替换、反向包选择/优先级合并、handlers/items/tags/options/nodes
  合并与 init；`config(TeX&)`、运行时 `add`/filters 仍仅声明，完整 TeX 未接通。
  原 Tags 头按既有 typed 契约归位，Tags 实现未完成；ParseOptions 增加 request-owned
  equation，完整构造/列表实现仍未完成。新增八个真实组件测试，不伪造 TeX。
  Source witness `tex-configuration-source.cjs` 十 TS/CJS 锁已 exit0；配置 native
  首次测试构建因 Boost.UT 打印 vector<u16string> 失败，已改 ranges::equal 保持同一断言。
  构建 **18290 已终态 exit0**，日志 `configuration-final-build.log`，并发4；
  原79549已终态失败且句柄失效。八项配置+五项 FunctionList/PrioritizedList 聚焦
  **13项/326断言 exit0**（`configuration-lists-focused.log`）；Source 从 C:\Windows
  重放 exit0（`configuration-source-replay.log`）；FunctionList 正式 Source 十观察/四锁
  也重放 exit0（`function-list-cleanup-source-replay.log`）。完整 Math 测试 **357项/
  134,547断言 exit0**，句柄 **37764 已终态**，日志 `configuration-full.log`。
  当前无 live 构建/测试/agent，工作区仅原13项无关修改，继续保护。
  **下一步**：在原 Tags/ParseOptions/TexParser/input-jax 职责内接通构造、列表/工厂和
  config/add/filter 阶段；原 `tags.cpp` 尚是未进入构建的旧实现，须保留数学规则并原位
  改写其 Context 依赖，不另外创建 native_tags 或第二条 parser 路径。38包绑定、完整
  TeX→共享布局→Scene→SVG 与 Folia 集成仍未验收，不可重复宣称已完成。

### 以下为已完成组件的历史证据，当前操作以上述短检查点为准

- **本批实现：原NodeFactory四槽configured creators及boldsymbol token/rewrite已落原文件**；提交`0d7b188c`。
  NodeCreators为node/token/text/error四个optional typed函数指针槽；setCreators逐槽合并。
  make*先捕获对应函数再调用，默认createToken走configured text→node，createError同样走
  configured text/node；只有makeNode成功返回creator后登记。text明确nullable指针（null和
  空串不同），原setMmlFactory可重绑。NodeFactoryConfiguration仅借用typed env/登记服务，
  ParseOptions继承它但environment/构造/列表实现仍未闭合，**没有完整native parser**。
  **职责复核已归回原NodeUtil**：NodeDefinition在node_util.ixx；setProperties拥有attrs/
  native属性及texClass/movablelimits规则；NodeFactory只构造、委托、登记。新MML
  function_operator bool对应Source fnOP，merge/copy/clone已有聚焦验证；其他通用NodeUtil
  JS属性访问API已撤下，其他数学helper与完整parser调用仍待按类型接通，不能声称全文件完成。
  原boldsymbol/configuration.cpp已去Context：createBoldToken先默认token，非mtext时读
  当前env.bold_symbol；为真则标fix_bold并登记fixBold。span版rewriteBoldTokens实现原八种
  variant映射及其他值保留/true flag清除。**TexFilterData版filter、BoldsymbolMethods宏方法、
  initializeModule仍只有声明**，需真实ParseOptions列表/三配置类/TexParser，包没有接通。
  tests/math/native_tex_node_registration.cpp已归普通名node_factory_registration.cpp；
  新node_factory.cpp/boldsymbol.cpp测试实际MML和typed工厂host，不伪造TexParser。
  先前focused第一项因测试少写merror的inferred row层失败，已按原MML形状修正断言；
  11项/132断言exit0（node-creators-final-focused.log），包含原四登记测试；
  76165重建终态exit0。**完整349项/134,247断言exit0**（node-creators-full.log），
  18724已终态；随后NodeUtil职责归位，71828重建exit0，node-creators-source-roles-build.log。
  最终聚焦11项再次exit0，node-creators-source-roles-focused.log；最终完整
  **349项/134,247断言exit0**，node-creators-source-roles-full.log，66222已终态。
  当前没有live build/native/agent。原reference_node_factory生成的Property/FunctionRef/
  interface-fixture测试及旧oracle/runner已删（其正常数学构造由真实MML测试/正式Source
  比较覆盖）；P41证据已标historical，不能继续当当前TypedFactory/parser验收。
  正式tex-node-creators-source.cjs锁3 TS+完整profile cbb0...；Root从C:\Windows重放exit0，
  node-creators-source-replay.log。使用真实Source ParseOptions/NodeFactory/MmlFactory，含真实
  TeX解析的boldsymbol标记、原getList+rewrite和10 variants；native仍只验工厂/重写primitive。
  原13项无关改动继续保护。下一步完成此批验证/小提交，然后原三配置类可直接消费真实
  NodeCreators，别再把工厂四槽当缺口重查；StackItemCatalog沿用原typed FactoryClassMap，
  Tags creator签名按原Tags家族，不能用空profile facade替代实际配置合并。

- **已完成前置：原Macro/TokenMap/MapHandler三家族已原位改写并进入构建**；提交`4c5a7e92`。
  parser_family当前导出:types/:macro/:token_map/:map_handler/:parse_options；新代码无
  Context/Property/原通用函数对象。Macro为owned token+typed方法+TeX实参数组。
  MacroCallbacks复制两个固定C++函数指针表，输入jax生命周期不可复制/赋值/重绑；
  macroHandler/tokenParser和逆向binding使用one-based ID，0表示缺席，坏ID抛异常。
  **实际38包的函数表尚未绑定，也没有通过真实TexParser执行回调。**
  原MapHandler只持每名当前shared_ptr<TokenMap>，SubHandler优先队列持选中map的真实
  shared所有权；替换后仅真实配置/局部引用保留旧map，最后释放即RAII销毁。禁止照抄旧
  ignored map_handler候选的owned_maps_累积所有替换对象设计。256次同名替换weak_ptr
  检查已通过；这不是完整渲染请求内存验收。Registry/WarningSink由input-jax拥有，
  SubHandlers只借这两者，不能让它们先于借用者销毁。
  TokenMap保留lookup key与Token/Macro.token的区别；static查找返回值借用，实际调用
  保存本次Token/Macro/CommandDefinition值快照，防止回调改写原条目后参数失效。
  MacroMap动态查找直接读canonical session.findCommand，不复制出第二份定义存储；
  替换定义/native alias/character alias/suppression均选择当前项，Char alias保留捕获Token。
  RegExpMap的两次Source test顺序、null-char contains、反向add、last同对象remove、缺名
  中途停止且保留已加项、SubHandlers首次key顺序均按原Source实现/观察。
  **CommandMap构造/parse仍只有声明**：需要真实TexParser.currentCS协作；不能当普通
  MacroMap替用，不能伪造parser测试通过。其他map parse方法已编译，但本轮native只验
  lookup/composition/binding/lifetime，未执行这些parse方法或任何真实TeX回调。
  老token_map_raw_edge_trace纯Context边/符号保留测试两项已删（API已撤下）；旧
  reference_token_map/reference_map_handler含回调和currentCS行为要求，保留待原位替换，
  不在构建图/不算通过。token_map_types、token_map_context_bindings、handler_context_bindings
  仍由旧parser_runtime引用，须跟其native改写退掉；不是已清完的文件图。
  **验收**：focused13项/331断言exit0，map-handler-focused.log；Source正式
  tex-map-handler-source.cjs十TS/CJS锁从C:\Windows重放exit0，map-handler-source-replay.log。
  初测编译9591因Boost.UT不能打印vector<enum>而失败，已改ranges::equal；51887、48379
  重建终态exit0，map-handler-tests-rebuild.log、map-handler-final-build.log。
  **当时完整342项/134,166断言exit0**，map-handler-full.log，句柄37356已终态。
  随后职责复核把未被调用的command-prefix bool分支从MacroMap去掉，留给原CommandMap
  自己实现；MacroMap.parse保持原无前缀分发，没有另加helper角色。86784重建终态exit0，
  token-map-source-roles-build.log；最终focused13项/331断言再次exit0，
  map-handler-final-focused.log，未重跑完整suite；新旧parse方法本来都未作执行验收。
  当前无live build/native/agent，仅原13项无关修改受保护。
  **下一步仍是三配置类**：原configuration文件使用canonical选项schema、这次实际
  SubHandlers和已验FunctionList，保留Configuration/ConfigurationHandler/ParserConfiguration。
  节点creator hook/StackItem/Tags catalog仍须按原owner定义，不能用空配置记录假装接通；
  完成后实现ParseOptions构造/列表/工厂及真正TexParser，不另造profile facade。
  本轮为下一步精读NodeFactory.ts/BoldsymbolConfiguration.ts：原creator四槽是node/token/
  text/error；默认createToken要经configured text→node，createError也经这些槽；外层只有
  request kind=node在成功返回creator后登记。boldsymbol配置仅覆token：先默认token，再非
  mtext时读当前env.boldsymbol，标fixBold并登记同名list；节点creator typed catalog必须先
  补回这套原NodeFactory协作，不能只把原makeToken默认实现当configured路径。原Factory
  typed class map已经是vector<pair<Kind,FactoryClass>>，构造/last同kind覆盖可复用，不重写。

- **已完成前置：原ParseOptions已原位移除Context实现并纳入构建**；提交`9041b961`。
  仅原13项受保护修改留在worktree。`parser_types.ixx`已从审过的候选推广
  （增加core MathItem及FindTeX/TexFilterData前置声明）；parser_family
  当时只导出真实已构建的:types、:parse_options，没有旧runtime/fallback/第二parser。
  parse_options.ixx含42个观测top-level字段的TexOptionValues稀疏类型记录、14个包选项组及
  原ParseOptions服务声明。cpp实现这些值的mergeDefaults和原四种getVariant；约138行，
  不是另建helper或新的JS对象模型。**构造、factory/Tags/ColumnParser、临时列表方法仍只有
  声明，不能实例化完整ParseOptions/渲染TeX。三个配置类本轮仍未改写。**
  **新确认Source语义：defaultOptions是后层覆盖前层，不是仅补缺省**。原Options.ts/CJS
  insert实际递归合并record，数组复制/替换；native optional区分缺席/false/0/空串，权限保留
  bool整包规则与子选项map区别，macro定义复用session已验纯值。userOptions warnings、
  setoptions字符串分发、包默认值绑定尚未实现。不要重新猜测合并方向或自动重选mathStyle。
  tex-parse-options-source.cjs六TS/CJS锁、240个样式观察+实际Options合并观察通过；Source
  只生成style fixture，tests/math/parse_options.cpp是手写表驱动。原generic receiver两个
  clear Context测试已删（withdrawn JS API）；real_owner旧测试含root/list/clear数学状态需求，
  保留待原位改写，不在构建图/不算通过。原ParseOptions.cpp旧约739行实现已被替代。
  构建89232/51362均终态exit0；parse-options-build.log、parse-options-tests-build.log。
  focused五项/266断言exit0；**当时完整329项/133,835断言exit0**，parse-options-full.log，
  句柄41215已终态。没有live build/native/agent。原13项无关改动继续保护。
  **下一步**：原configuration.ixx应导入此canonical :parse_options值schema，不再写另一份
  ProfileTables/option schema；闭合原Configuration、ConfigurationHandler、ParserConfiguration
  三职责，init/config使用已验FunctionList。需要SubHandlers时推广原map_handler家族职责，
  不建新profile facade或接口空壳。随后完成ParseOptions真正构造及原服务方法，再接Parser。

- **本轮配置前置已恢复原Unicode16Normalization到实际folia_math图**，复用原算法和
  数据，未另写NFD实现。French样式需要这个依赖，但不是French resolver验收。
  本轮代码/夹具提交`158d828e`；原13项受保护修改保留。
  原reference_unicode_normalization.cpp含20,077行/8.67MB生成C++测试数据，首次编译
  cl占5.2GB；Root核对进程命令后主动停止该次编译，句柄27161终态exit1，非测试崩溃。
  已改成原职责名tests/math/unicode_normalization.cpp（约85行）+fixtures/unicode/
  normalization-corpus.tsv（19,965行数据，约2.2MB）和normalization-source.inc。
  原旧测试与旧runner已删，source oracle只生成数据，不再生成整份C++测试。
  数据/source两--check通过；新构建74581终态exit0，unicode-normalization-fixture-build.log；
  聚焦3项/99,928断言exit0，unicode-normalization-focused.log：官方五列99,825精确输出、
  85组合序列、17平面摘要覆盖1,114,112码点，再加语料行数断言。没重跑完整324项。
  **原Configuration/ConfigurationHandler/ParserConfiguration尚未改写，不把调查/前置说成接通。**
  下一步仍是在原configuration及parse_options中闭合typed值声明和三类职责；已读取原TS/
  canonical旧C++，无需重做包inventory、scanner/ParseUtil或NFD审查。
  Luna额度错误仍显示到10月25日，无live agent。当前无live build/native。

- **最新用户纠正：禁止用native/typed/新旧前缀掩盖重复职责或保留替代实现。**
  本轮清理已提交`2022cb1a`；Git仅剩原13项受保护修改，无live build/native/agent。
  本轮将新测试`native_function_list.cpp`归回`function_list.cpp`，priority测试归
  `prioritized_list.cpp`，各对应同名类；没有另建生产模块，生产代码仍在原两个ixx内。
  删除旧reference_function_list/function_list_array_owner/function_list_void_result/
  prioritized_list_source_operations四测试和function-list旧generator/inc/ledger/check工具。
  历史P19a证据已标superseded，不再作为当前完整FunctionList验收。
  **旧utility bindings还没有删除**：其直接消费者是core/output_jax、input/tex/
  handler_context_bindings、parser_runtime、configuration_prioritized_list_projection、
  output/common/common_bindings（均不在当前组件构建图）。这些消费者尚未完成原职责
  的类型改写，不能把测试清理说成整个旧绑定图已清完。后续在相应owner改写时一起
  删除其绑定依赖，不另建native_*生产文件或兼容分支。
  本轮构建句柄13785已终态exit0，function-list-cleanup-build.log；五项/26断言聚焦通过，
  function-list-cleanup-focused.log；正式Source十观察/四锁重放exit0。没有重跑完整321项。

- **先读本节正文，再核对Git/进程/代理，随后从“下一步”继续。** AGENTS已有该规则，
  不能只在摘要里提到路径。重开已解决问题必须指出使原结论失效的新代码或新证据。
  重要决定/验收/提交后、压缩或结束前更新同一本；下文历史不是当前待办。
- 保留原MathJax文件/类职责和已验证C++算法，去掉通用JS对象语义模拟，使用领域类型/RAII。
  不重写另一套parser/layout，不堆释放补丁，不自行reset。Mermaid暂停；Folia只先接SVG，
  不加D2D像素门槛。优先Luna，必要时仅6.1 sol；一次构建、--parallel 4，正常本地测试用语。
- **历史完整Math组件基线：316项/33,615断言，exit0**，
  `build/math-port/native-definitions-begingroup-final-full.log`，句柄5272已终态。
  构建48807/3406均终态exit0，日志native-definition-values-build.log、native-begingroup-build.log。
  当前无live build/native；完整TeX→SVG、38包接通和Folia替换仍未完成。
- **随后新增原native PrioritizedList/FunctionList已提交6c9cc6bb**：原模块保留类/继承职责，
  value vector + compile-time typed void/bool函数指针，无Context/记录身份/泛型实参/receiver。
  原function_list.cpp旧实现已删除（现在是模板header），旧bindings仍是inactive待退消费者，
  不可进入native配置图。构建73996终态exit0，native-function-list-build.log；五UT/26断言
  聚焦通过，native-function-list-focused.log；正式util-function-list-source.cjs十观察/四TS+CJS
  锁Root从C:\Windows重放exit0，native-function-list-source.log。没有重跑完整suite，
  不能声称当前完整321项已跑；上述316完整基线与新五项focused须区分。
- **配置新Source证据已提交80fb9771**：tex-options-lifetime-source.cjs五观察/完整profile锁，
  Root从C:\Windows重放exit0，native-options-lifetime-source.log。旧未推广sketch误把
  allowOptions.tex.maxBuffer=false当成允许false值：实际命令报TeXOptionNotSettable、5120不变。
  Root已去掉候选FalseOption/BufferLimit，Column候选maxBuffer直读size_t。
  实际effective options跨compile/reset保留；request ParseOptions应借input-jax lifetime唯一
  mutable typed选项值，不能每表达式恢复defaults。mathStyle resolver构造时捕获，后来setOptions
  改mathStyle=ISO并不替换resolver（A仍按初始TeX模式）；这是实际Source行为，不要动态重算。
  这些是新发现的候选契约错误，不重查已验收scanner/ParseUtil/MML/Common。
- 本轮已提交：**caff2e5d** 原Token+session纯值别名/单一定义存储；**3f056eef** 原
  BegingroupStack原生层算法+Source对照；**44fc49ef** 原Configuration三职责/typed dispatch约束。
  定义值四新UT+原七session+四column共15项/92断言；native-definition-values-focused.log。
  BegingroupStack五UT/38断言；native-begingroup-focused.log。以上均包含于最新完整suite。
- canonical **CommandDefinition** 持source_token和封闭body：原MacroDefinition replacement/
  NativeMacroAlias/CharacterMacroAlias/MacroSuppression。原MacroDefinition字段保留，新增
  parameter_template区分MacroWithTemplate。NativeMacroAlias持MacroHandlerId+TexArgument数组；
  CharacterMacroAlias持不同类型TokenParserId+owned Token（含character/attributes）。
  IDs指向固定编译C++函数表，不是方法名字、请求分配编号、JS对象身份或闭包。
  **实际函数表绑定和别名调用尚未实现，值测试不等于dispatch验收。**
- **只有一个TexDefinitionState**：每层原三个域commands/delimiters/environments各有typed map，
  groups外到内+global；旧macros_已去掉。findCommand选择最内层，包括alias/suppression；
  findMacro仅投影该条目的replacement，不回退外层。Parser/MacroMap必须用findCommand。
  原ColorModel/ColumnMacroDefinitions、tag/label/counter保持canonical已有所有权，不覆盖旧镜像。
- **Token保持原独立primary folia.math.input.tex.token**，不再是:token partition：它是无环纯值，
  session和parser共用，避免另一份captured-token结构或session→parser反向依赖。
  `begingroup/stack.{ixx,cpp}`也保持原独立primary，只借TexDefinitionState，原类掌管push/pop/
  finish/remove/reset/sandbox/checkGlobal，无ParserRuntime/handlers/marker Symbol/shared_from_this。
  Source明确只有begingroup使用低于-100的优先级；native按这些owned层同序lookup。
- **checkGlobal必须删除target旧值**：Source先以MARKER覆盖target再连MARKER一起移除。
  native按(domain,name)顺序清内层直到包括当前global target；sandbox target是-101保护层，
  原-100全局定义留在下方，下一sandbox可再见。旧候选“excluding target”已明确取代，勿照抄。
  正式tex-session-definitions-source.cjs现16组/12对TS+CJS锁，Root从C:\Windows重放exit0，
  native-begingroup-formal-source.log。真实TeX/MML成功输出、merror检查、handled-error前定义/
  未闭合组跨表达式保留、let builtin/CharacterMap重定义快照及三域target清除均已固定。
  不再重查“正常compile返回是否代表成功”——helper已检查树里的merror。
- **下一步实际动作**：在原configuration文件闭合Configuration、ConfigurationHandler、
  ParserConfiguration三类实际声明（Root本轮已读原TS与旧C++），接固定typed callback表及
  macroHandler(MacroHandlerId)/tokenParser(TokenParserId)和逆向binding；随后合ParseOptions/
  ColumnParser/Tags实际服务图，再继续原Stack/BaseItems/dispatch/Newcommand实现。
  旧tex-parse-options-contract的“单ParserConfiguration+ProfileTables”只是早期sketch，
  不足以表达原三个职责，不可直接当完整类契约推广。要保留package记录、named registry、
  init/config/filter优先级和handler/factory/options composition，不另造profile facade。
  PrioritizedList/FunctionList前置已完成，不再重写：configuration列表用
  PrioritizedList<Configuration const*>；init/config列表分别是FunctionList<ParserConfiguration&>
  和FunctionList<ParserConfiguration&,TeX&>，filters保持其专门typed数据签名。
  **先完成具体typed选项/配置声明**，按新的persistent option borrow/captured style约束闭合
  ParseOptions，无需再做全38包inventory。Root已从实际profile观察所有42个top-level字段/38配置：
  ignored build/math-port/configuration-options-observed.json；复现probe在
  tools/math-port/drafts/native-raii/configuration-options-observe.cjs。
  该JSON是需求证据，不是native schema/验收，不是可运行的函数名解析表。
  候选tex-parse-options-contract/README.md已加Root correction并修size_t/borrow签名，
  仍是完整性不足的历史sketch，不当已冻结可构建header。
- 原解析循环服务契约已审并提交abf3b72a，正式tex-parser-interface-source.cjs三例/
  66 CJS+14 TS锁Root重放通过，native-tex-parser-interface-formal-source.log。
  TexParser只用canonical scanner；ParseInput显式receiver+owned token；currentCS按值保存；
  nested ParserScope必须先于owned Stack入场；真正TextParser用protected DeferParsing阶段，
  在derived Parse/Push期间level仍未赋值。removeParser(TexParser&) noexcept负责精确unwind。
  ParseOptions公开真实SubHandlers&，configured createToken服务仍须连接boldsymbol hook：
  标准makeToken后读当前env、除mtext外标记/登记fixBold；不可绕过configured creator。
- 当前声明/候选路径（均未整体推广/编译）：
  * Root `tex-parser-declarations/` 四header；`tex-base-items-contract/` 原26类。
    FrameHandle=shared_ptr<BaseItem>，Factory unique构造后转换一次；这是实际NumCases同对象双Push
    的新证据，正式tex-frame-activation-source.cjs已Root验13锁。Pop保留frame env、clearEnv
    原地清scope；Replay先Pop/env再解reuse并递归；MmlStack First=back、Last=front，Peek是
    Source slice(Size-count)。这些已解决，不再重审；旧24+6只覆盖其有限replacement行为。
  * `tex-dispatch-contract/headers/` 已同步canonical Token独立module与CommandDefinition lookup；
    Root :types已去掉重复NullArgument/TexArgument和跨module Token forward，MacroArguments借
    CommandDefinition；已加Configuration/ConfigurationHandler/TeX同primary forwards。
    CONTRACT重复旧类型代码块已删，以canonical+Root :types为准。源码priority同级逆配置顺序，
    已有14hash观察通过；别恢复原先错误解释。NotItem.remap是CharacterMap*借用。
  * `tex-session-definitions-contract/` README已标旧session/group镜像历史，禁止推广覆盖canonical。
    它的Newcommand/ConfigMacros seams仍待接当前类型；不再等待其session状态重交付。
  * `tex-tags-contract/` 保留原Tags/base/AMS/cases/tagformat/mathtools；CasesTags持自己的subcounter，
    Mathtools持久form由session持，Tags借一份ParseOptions.equation。整份旧session镜像不能复制。
  * `tex-column-parser-native/` 只有两份未验收body/header，尚导入旧js_numeric_input/utf16且缺UT/
    完整服务编译，不能直接推。canonical UnitUtil接口已可用。Column runtime在bundle/src/cjs，
    .reference只有TS，不复制node_modules、不重编译TS。
- **所有Luna followup上轮已因usage limit停止**，没有代理仍在写；无新额度信号不循环重试。
  `tex-stack-native/`、`tex-dispatch-native/`、`tex-parse-options-native-contract/` 当时尚无文件，
  不能当已交付；Root已继续自行推进。已消费的旧完整包不重派、不等旧完成消息。
- 已完成并保留的native组件：43 MML/clone、font14,568,049字段零差异、Common core/families/
  行断行/分式根式/scripts/table/menclose，NodeFactory登记64a06e78、Color febc2e4e、
  MathItem a17e72e7、Column状态98598241、UnitUtil fdaecc6d、Scene dash 44b4b901。
  NodeFactory标准登记不等于configured token creator；MathItem服务测试不等于真实pipeline。
  UnitUtil复用native regexp_host/lengths，7项184断言与正式Source工具已通过；Scene dash
  只是Paint value/serializer，4项8断言和10 Source观察通过，无完整Common emission。
- Common→Scene提案在 `common-scene-emission-contract/`，Root读过未冻结body。真实Common家族
  发Scene、LayoutJob一次build/getBBox/freeze，不建Scene wrapper/factory树。TextRun、foreign、
  annotation、paint继承、href hitbox等仍缺；dash schema已补。完整输出和Folia尚未验收。
- Git原13项无关修改继续保护（见文末），只显式stage自己的文件，不reset。最新代码6c9cc6bb，
  随后80fb9771文档/Source提交；本记录另作独立小提交。当前没有live build/native/agent。

### 历史组件证据（当前状态以上方短检查点为准）

- **完整 Math 组件 273 项 / 33,051 断言，exit0**：
  `build/math-port/native-scene-factory-final-full.log`，最终构建
  `native-factory-owned-result-build.log` exit0。menclose已提交 `82e42966`，
  Scene用途 `e58a53b5`，Factory所有权返回类型 `a555d882`。
  正式56条native SVG对照及精确参数收紧也已通过，详情见下节。
  这不是端到端TeX/SVG/Folia验收；旧263项日志是此前table/containers基线。
- **9cbbb155已完成**：实际Common Table/Mtr/Mtd、Mpadded/Semantics/TeXAtom，
  CMake/primary导出与真实MML wrapper tests。保留原家族文件职责，未创建另一套树/家族层。
- 75705931：修shared defaults及root width。Source Wrapper.ts:421默认NOSTRETCH；
  native无消费者的UnresolvedStretch导致分式被排除于mrow尺寸计算，已删除并默认NoStretch。
  没在Mfrac加补丁。table括号已精确选中Source size3/-size3和全部尺寸。
  Source SvgMath.setChildPWidths的纯数值override已移到CommonMath：root传metrics容器宽/
  pxPerEm，nested转发w，clear=false，无DOM；正常root.getBBox最终确定百分比列宽53.04。
  Jax未设置linebreak width时初值NaN（Source unset算术行为），修auto表被初始0收窄。
  Mpadded containerWidth已通过base virtual dispatch。正式Source table工具锁新增对应TS/CJS，
  before/after lifecycle实际观测验证，`common-table-shared-math-contract-source.log` exit0。
- e8c504b2：CommonMfrac/Msqrt/Mroot共享placement geometry；原288/192输入未改，
  native origins/rule/overbar/placed height精确比较通过。单独focused日志
  `native-common-fraction-shared-geometry-focused.log` 8928断言、
  `native-common-radical-shared-geometry-focused.log` 16224断言，均exit0。
- 9c91ccad：Scene Paint追加optional StrokeCap/StrokeJoin，SVG writer纯序列化；
  继承与三组显式值UT通过。Common不import SVG/DOM。
- a56a54bf：四真实NewCM scripts树用固定Source数值替换formula/isfinite期望，
  189断言通过；原四synthetic UT保留22断言通过。多重脚标问题已解决：
  去掉getUVQ默认空参数，未cache时要求combined boxes，getCombinedUV返回完整array3。
  未采用“无参自动补偿”方案；不要重查旧msup继承slot问题。
- 7d8b7491 / 20d57b20：table/containers与真实scripts正式Source工具/锁/观测已提交，
  Root从C:\Windows重放exit0：`common-{table,containers}-formal-source.log`、
  `common-scripts-real-formal-source.log`。generated evidence只写ignored build，无draft依赖。
  table14个SVG观测不是whole-output baseline；8个branch有固定精确期望。容器6个真实Source
  情景及native5项/62断言，均不等于所有输入覆盖。详见common-source-witness-coverage.md。
- 43 MML类、exact clone、StyleJson、TreeVisitor、字体14,568,049字段零差异、Common core、
  texts/rows/inline、scripts/limits、fence/action、递归LinebreakVisitor均已组件验收。勿重做。

### 历史断点与候选记录（不是恢复执行入口）

- 已提交：`149ddacb` Mo非accent remap短路；`1a86ac59` 正式menclose Source工具与锁；
  `82e42966` 原CommonMenclose/notation/catalog、窄sqrt projection、CMake和原生UT。
  这些已消费，canonical是权威，不能重新推广旧menclose packet。
- 此前menclose完整组件269项 / 33,025断言 exit0：`build/math-port/native-common-menclose-final-full.log`。
  构建 `native-common-menclose-formatted-build.log` exit0。
  随后只收紧parameters测试为Source精确double，构建/14断言聚焦均exit0：
  `native-common-menclose-exact-parameters-{build,focused}.log`。生产代码未变，不需无故重跑全套。
- **实际native Scene→SVG 56行全部通过**：正式 `common-menclose-compare-svg.cjs`，
  日志 `native-menclose-formal-svg-compare.log`。原始dump `native-menclose-svg.tsv`。
  **最终Scene/Factory构建后已重新生成同一dump并再次通过**：
  `native-menclose-final-svg-dump.log`、`native-menclose-final-svg-compare.log`，均exit0。
  使用原始Source SVG十进制有理数精确归一化（无round/epsilon），相对/绝对path等价；
  仅明示rect/ellipse转path及mitre→miter。这个范围是28装饰handler×两个受控box，
  不是完整TeX文档/SVG byte equality/Folia验收。radical另有21断言真实投影验收。
- 正式Source/geometry/selftest均已在C:\Windows由Root重放exit0，日志
  `common-menclose-formal-source.log`、`common-menclose-geometry-formal-source.log`、
  `common-menclose-formal-selftest.log`；baseline+13种更改检查，含1e-15差异拒绝、
  负arc radius/overflow/raw-fill直接断言。工具无draft依赖，generated仅在ignored build。
- Root发现旧参数observer用parseFloat stub把2mu变成2，已改实际CommonWrapper.length2em，
  锁定util/lengths，实际Source厚度0.1111111111111111/extenders0.6111111111111112，
  去掉原生对应epsilon断言。原始失败观察 `common-menclose-real-length-observe.log` 是历史证据。
- 已解决勿重查：Mo只在isAccent才要求parent；recipe检查ordered Translation{-0.1,0}后
  Rotation{90,{1.5,.25}}（80断言）；Source getBBox初始化用BBox.zero而非new BBox；
  horizontal/vertical extenders已修；diagonal用实际childbox；双箭头末段已补；
  local y/rotation/sweep不二次翻转，phasor一path含bottom，fill/stroke继承分别保留。
- sqrt projection：辅助MML detached、document arena唯一owner、重新wrap同一base MML，
  不clone/reparent/共享旧wrapper/attachAuxiliary。已有radical geometry不覆盖。
  Module impl `module folia.math.output.common; import :family;`；fixture header不include std；
  长小数用普通double，_u不作算术。这些已修，不再当成新问题。
- **scene-element-roles已推广提交 e58a53b5**（原ignored包根目录tools/math-port/drafts/native-raii/）。
  Root契约已写common-native-implementation.md“Completed element purposes…”：11种用途值，
  Group.role/Builder最后参，writer只选typed translation separator，保留操作序列；
  unknown_text与math_wrapper/id使用comma，enclosure_content用comma-space，其余space。
  构建native-scene-element-roles-build.log exit0，focused日志
  native-scene-element-roles-focused.log中新增2项/12断言通过，Root核对8条Source原字符串。
  原候选patch SHA256
  454db98ba45def0415d04e93459ed087c3b1e88d202e71c935bb0aafa3a99c6a。
  无完整Common→Scene traversal/TextRun原点/foreign XHTML/响应root实现，不可冒充完成。
- **TeX Stack实际Source证据已新增，勿重做inventory**：ignored tex-stack-transition-source/
  {observe.cjs,expected.json,source-lock.json,README.md}；Root从C:\Windows重放24例/88文件hash
  exit0，`build/math-port/tex-stack-transition-source.log`。replay可能含原top自身、incoming和
  裸MML，先Pop再逐项递归；进入checkItem时两个frame由明确owner持有。真实BaseItem.env是prototype accessor，
  delete item.env并不清除_env！旧inventory prose对此不精确。不能据此错误清空存活frame的环境。
  完整package replay去重/环境所有权协议仍待Root冻结，不是native或38包验收。
- 已有tex-parser-architecture/{README.md,closed-inventory.md}是完整38包提案；MmlDocument拥有
  唯一MmlFactory，NodeFactory借用。parser_family.ixx已是parser primary，勿再另造竞争primary。
- **Factory已提交a555d882，build36072已终态exit0**：native-factory-owned-result-build.log。
  FactoryServices/AbstractFactory第四Result默认Product&的窄改动已推广并focused通过；
  新UT tests/math/native_factory_owned_result.cpp两项/14断言，以及匹配的原有Factory测试均通过，
  日志native-factory-owned-result-focused.log。候选factory-owned-result/已消费。
  Root契约在native-raii-architecture.md“TeX frame construction and transfer”。环境用闭合
  shared_ptr<TexEnvironment>域值保存真正scope别名/复制，不含parser/frame/MML owner或通用对象。
  generic factory只返回unique_ptr给caller，无新产品arena/重复registry/运行时selector。
  **完整native句柄80469已终态exit0**：native-scene-factory-final-full.log，273项/33,051断言。
  本批所有build/native句柄均终态；该批Luna当时已completed，无需等旧句柄。
- **包级ownership审查已交付并补证**：同tex-stack-transition-source包新增
  package-replay-review.md（SHA256 3bc743d6131f79aac4806a0e21b193b3f44ec188ac07727802227a9219524a2a）、
  package-replay-observe.cjs/package-replay-observed.json，未修改原24例。
  Root从C:\Windows运行补充6例/28hash exit0：tex-package-replay-source.log。
  静态未发现单replacement重复frame/第三个既存frame；唯一跨栈移交为ProofTree
  innerStack.Push(incoming)后返回Drop，真实Open frame由inner保留，global也改为inner.global。
  **Root已修正接口契约为checkItem(unique_ptr<BaseItem>& incoming)**：在Source实际调用点
  move给nested Stack，Drop只销毁外层尚持有的owner。不能使用“borrow+Drop必销毁”旧理解。
  Cases/Physics空Replay也已Source观察，必须Pop，与Accept/Drop区分。
  未发现checkItem中Pop外层当前top后继续用this的具体路径；这是有限审查，非native验证。
- **本轮已提交88864c48：TexScanner唯一不可变source buffer与sourceSnapshot()**。
  replaceSource不隐式reset游标；旧frame snapshot可跨替换和scanner销毁，最后owner释放后
  buffer自然销毁。构建7145已终态exit0：native-tex-source-snapshot-build.log；
  全部10条scanner测试/96断言聚焦通过：native-tex-source-snapshot-focused.log。
  没有无故重跑全Math suite；上方273项是此前完整基线，勿写成新版274项全量已跑。
- **本轮Root声明冻结进行中**：正式文件职责/验收写入typed-tex-parser-implementation.md，
  native-raii-architecture.md已追加scanner唯一buffer要求。ignored tex-parser-declarations/
  src/math/input/tex/{parser_types,stack,stack_item,stack_item_factory}.ixx由Root写出，
  都尚未推广/构建，BaseItems/ParseOptions/dispatch完整声明未就绪，不能派body或冒充parser完成。
  所有相互引用类归原parser_family primary，stack/item/factory将成同module partition。
- 此前分工记录（已被上方短检查点取代）：mml_base_contract_review审四份Root header；raii_session准备
  tex-parse-options-contract/；math_structure_audit修正tex-stack-constructor-contract/中错误TS行号
  并补构造运行证据（表曾把MJS/TS位置混淆，不可未经复核推广）。Multlined实际调用在
  MathtoolsMethods.ts:147附近，继承Multline ctor，defaultTags收到truthy parser对象；
  Root倾向保存实际bool语义，不保留parser对象，需要运行观察确认后冻结MultlinedItemInit。
  font_assert_audit的tex-scanner-source-snapshot/已消费，不再推广旧镜像。
  本轮没有live build/native句柄；上文所有旧句柄仍终态。
- 本轮声明review已发现并正在修正具体边界，勿回退：getErrors有sub/sup键而非StackKind，
  Root加独立StackErrorKey；left_root/up_root实际是带正号/单位的相对em字符串，不能double，
  draft已改optional<u16string>。**clearEnv必须原地清空TexEnvironment，旧Root说明写错了**；
  Left/middle显式env={}才换新scope。native-raii-architecture.md已纠正，reviewer在补alias观察。
- **四header reviewer已完成**：tex-parser-declarations/REVIEW-stack-core.md，Root已修上述类型和
  :stack reexport :stack_item，保留complete BaseItem供unique_ptr消费者。Root从C:\Windows
  重放同tex-stack-transition-source/stack-core-declaration-observe.cjs，4例/10hash exit0，
  日志tex-stack-core-declaration-source.log。还确认Peek是slice(Size-n)有符号规则、isKind
  只是kind相等；ProofTree.toMml(bool,bool)必须override但忽略两flags，不能沿用旧C++转发flags。
  这些是实际Source观察与声明review，不是native Stack编译验收。
- **constructor witness已收紧并由Root重放exit0**：tex-stack-constructor-contract/
  source-constructor-observe.cjs、source-constructor-observed.json、source-constructor-source-lock.json；
  日志tex-stack-constructor-source.log。Multlined传parser到Tags.defaultTags仅消耗truth value，
  Root已冻结MultlinedItemInit marker表示true，不保留parser/begin对象。注意实际DummyItem继承
  kind=base，默认lookup key=dummy；不能把该Source wiring差别默默改成可用fallback。
  初始constructor表的不准确TS位置已修，不能用其旧版本当新Source证据。
- 当前分工：radical_table_runner正写tex-base-items-contract/的原base26类声明（无body），
  raii_session已交付修订的tex-parse-options-contract/，待Root审查；Root要求去掉杜撰的TagOperations门面和无消费者
  NodeListSink、旧bindings import、错误跨module MML forwards。实际Tags类必须保留且借用唯一
  TexEquationState；Color/Begingroup持续状态属于session，不能每request重置。
  其他本轮代理均completed。当前无live native/build，未宣称完整声明已冻结。

### 当时的下一步（历史，以上方短检查点为准）

1. 四份Root核心header审查、constructor表/运行观察已收取并修正；现在审最新ParseOptions服务草案和BaseItems候选；
   基于24+6条Source transition及Root owner/environment协议冻结完整声明，
   明确所有包家族的typed构造参数和服务引用后，再按原文件划界让Luna复用旧C++实现。
   不再等待已结束测试、不重做38包inventory、不得恢复通用对象模拟。
2. 继续接通Common数学placement与已有immutable Scene发射，数学仍归Common原家族；
   Scene roles只保证已完成group的用途/transform写出，不是完整Scene遍历或家族发射。
3. 完整TeX parser/38-package、Common→Scene发射与Renderer/Folia替换仍未接通，目标保持active。
   Folia仍旧QuickJS；Mermaid暂停，不做D2D像素对照。
4. 一个构建、--parallel4/math pool2；保护文末13项无关dirty work，小批提交、不reset。

## 上一批已验收结论与证据（保留，不是当前待办）

- **最新：实际算术、scripts/limits、fence/action、LinebreakVisitor 已原生整合验收。**
  完整组件 **251 项 / 26,461 断言** exit0：
  `build/math-port/native-common-scripts-lines-fixed-full.log`；最终构建
  `native-common-msup-slot-build.log`，此前格式整理构建
  `native-common-scripts-lines-final-build.log` 均 exit0。这仍不是 TeX→SVG 或 Folia 验收。
  已提交算术 `95a119b1`、算术正式Source工具 `ee233f64`、MML msup修复 `f4df0031`、
  scripts/limits `c8dd25ec`、fence/action `5dbcc9b3`、其他Common正式Source工具 `89e8a178`。
  visitor/CMake/新UT由本交接所在集成提交接通，不能拿旧235项结论覆盖当前进展。
- **本轮异常已解决**：完整suite曾因span越界退出（不是普通UT失败），已停止广测，
  有界运行 `native-common-script-bounded-diagnose.log` 定位到真实NewCM msup子case2。
  `MmlMsubsup::setChildInheritedAttributes` 错把角色 `sub()==2` 用作两子节点的物理位置。
  Source固定遍历0/1/可选2，角色只决定prime；已修责任MML方法并新增msup继承回归，
  聚焦 `native-common-msup-slot-focused.log`、四家族真实NewCM
  `native-common-script-bounded-fixed.log` 和上述完整suite均通过。不要再次当作未解决。
- **正式类/结构结论**：Mfrac/Msqrt/Mroot原地撤掉仅测试使用的scalar API，288/192配对数据
  未改，runner原地替换。Munder/Mover/Munderover直接继承CommonScriptbase，与Source一致；
  不保留从Msub/Msup/Msubsup继承再抵消行为的覆盖补丁。LinebreakVisitor已是实际算法，
  不是no-op；signed depth、Mspace具体break应用、physical getPrevious均已修正并验收。
- **测试准备规则已实际核查**：factory.wrap(root)本身不登记root（Source相同）；需要coreMO
  的普通row必须经真实Math根递归wrapNode完成索引，不能给生产coreMO加self fallback。
  Mfenced原内容wrapper的parent保留fenced，只有生成Mo的parent改投影mrow，Source如此。
  单独Mo在embellished inferred父下local breakCount可为0，embellishedBreakCount为1；不能
  改生产算法迎合UT。Mspace要测试visitor实际选择需goodbreak，newline已在构造时断行。
  测试长小数用普通double；Boost.UT `_d` 对这些长literal计算出错误期望，不能改数学公式。
- **正式Source工具可独立重放**：`common-{fraction,radical,scripts,limits,fence-action,linebreak}-source-witness.cjs`
  均由Root运行exit0，日志 `build/math-port/common-<family>-formal-source.log`。
  fraction288×84、radical192×89逐字段重现tracked表（忽略旧provenance注释），fixtures未改。
  范围与受控输入见 `common-source-witness-coverage.md`，不能称作完整SVG文档对照。
- **上一批结束时的候选：table/containers/menclose。** ignored根目录为
  `tools/math-port/drafts/native-raii/`；对应 `common-table-native-impl`、
  `common-containers-native-impl`、`common-menclose-native-impl`。下一批先table/containers。
  table已修coreMO显式Mo cast、复用原生string.split/js_number、span借用唯一children；
  有label/百分比/fixed/equal/frame/stretch/实际visitor的UT和8条Source记录，但尚未native执行。
  containers所需virtual/override本轮已整合，验收状态看文首；
  menclose类已移回原wrappers目录，不能另建common/menclose并遗留旧文件。
- **Menclose已冻结接口**：Common `notation`对应Source Notation.ts，闭合29种spec；
  immutable catalog由具体creator传span，固定catalog为静态constexpr数据。Common仅依赖
  Scene纯geometry值，SVG与未来D2D共享数学结果。辅助sqrt factory-owned且detached，
  删除其默认inferred子关联但保留arena节点；可选projectedBaseNode在普通初始化之后
  通过wrapNode重新wrap同一源MML作为childNodes[0]，保留源parent，按Source覆盖nodeMap；
  不clone子树、不共享旧base wrapper、不attachAuxiliary改变Source父链。其数学走现有根式方法。
  候选sqrt改动必须定点合并，不能整包覆盖Root已修普通根式实现。
- **该批结束时运行状态（已被文首更新）**：`common-scripts-real-source`当时仍在准备，
  现在已交付；恢复后读文首及live状态，不重新分派整包。
  搜索ignored包须用 `rg --no-ignore` 或直接读明确路径，无默认rg结果不能判缺失。
- **该批未完成项（当前状态见文首）**：288/192表的SVG origins/rules/overbar等
  runner现已整合，尚待原生执行；
  Common的BBox测试不覆盖全部原输出字段，断言从旧32338降为当前26461不能称覆盖增加。
  完整TeX parser/stack/38-package流程、Renderer→Common→Scene→SVG和Folia替换仍未完成；
  Folia仍旧QuickJS路径。Mermaid暂停；不要D2D像素对照。无关dirty work保护见文末。
- MML43类、exact clone、StyleJson、TreeVisitor、字体14,568,049字段零差异、Common基础/
  文本/inline入口均已验收，禁止因压缩而重新从头调查或恢复JS模拟。现有正式源码是权威。

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

## 历史核查结论（已核查事项不要从零盘点）

存在局部架构偏离和未清理的旧实现，不能宣称干净移植或完成接入；不建议整个仓库
reset。用户询问是否需要 reset 不是执行授权，目前没有执行 reset/revert。

保留已验证的数据、算法修正和紧凑测试；修正类职责与生产调用关系；撤掉被替代的职责。
旧 Common 类可从 Git 读取，不能把整套旧 JS 模拟恢复回来，也不能覆盖无关改动。

历史问题：`b23b2b46` 只将分式/根式数学函数移回原 Common 文件，仍为独立输入/结果/函数。
该问题已由 `95a119b1` 复用实际 Common 类解决；本轮在这些类上继续共享 placement。
不能再把它当作当前缺失，也不能把类已整合等同于完整渲染入口完成。

原始 CommonMfrac / CommonMsqrt / CommonMroot C++ 基线可用：
`git show b030b874:src/math/output/common/wrappers/<family>.<ixx|cpp>`。
其他原 Common wrapper 文件还在工作树中。复用类和数学方法，移除其模拟类型依赖；
保留新测试发现的修正，例如根式临时 BBox 只复制 w/h/d 的 Source 行为。

## 历史组件范围记录（当前完成状态见文首）

- 已有原生 MML 基础、部分 TeX 状态/工具、BBox/长度/正则、Scene 和 SVG 组件。
- 完整 TeX parser/stack/package 流程与 Renderer 门面尚未接通；CommonWrapper 基础编排
  已验收，家族接入进度以文首为准。
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

## 历史阶段：MML 集成结果（当前状态只以文首为准）

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

### 当时的后续计划（历史，已完成部分不得重复分派）

1. **继续 Common 下一批整合**：core 与 Text/Mtext/Math/inline 入口已通过
   235 项 / 32,338 断言。不要再冻结一遍旧草稿或重做其基础。
   直接用已交付的实际 fraction/radical/scripts/fence/action 与 visitor 包继续整合。
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

### 历史阶段：Common 基础整合前断点（不是当前待办）

- 正式责任/模块约定：`common-native-implementation.md`。types 与 wrapper/factory/Jax 已推广、
  构建测试通过；正式 source/header 是权威，不得用旧 `common-contract-refresh/headers` 覆盖。
  已接入的 Mo/Mrow/Inferred/Text/Mtext/Math 与其他家族应按文首断点区分。
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
- mathvariant 闭合诊断值接口和 SVG forced inline 数学策略已解决并验收，不再列为待办。
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

### 历史候选关键文件校验值（不是当前包版本或推广指令）

以下路径均相对 `tools/math-port/drafts/native-raii/`；均未原生整合验收。

| 文件 | SHA-256 |
| --- | --- |
| common-table-native-impl/src/math/output/common/wrappers/mtable.cpp | 9EC4EACE145A172412DC1E374332775BA165D8A5102367A1FC1836BC81C68E07 |
| common-containers-native-impl/src/math/output/common/wrappers/mpadded.cpp | 167359DBF7486A698200D2DE76A229BD219A963AE55923D4D1895341C6CF51F9 |
| common-menclose-native-impl/src/math/output/common/wrappers/menclose.cpp | C67B3C628FB636A081BC7387CA07F04141A982CF9E47F5451C394BD2FAE322CC |
| common-fraction-class-native-impl/src/math/output/common/wrappers/mfrac.cpp | 70F6FB35EE6E24AFB0267B40A2A9C0C948A04B9B956560B6B15DD3194977EBAF |
| common-fraction-class-native-impl/tests/native_common_mfrac.cpp | 28EA161E9E4DB7EAE2133AEA78C952CCAAD46329EBF1C5773689942320950E01 |
| common-radical-class-native-impl/tests/math/common_radical_native.cpp | 35A17966956A83EBA129141A78D5878D8CF9FFFA1D5D899C4F361CC7CD30A04C |
| common-fence-action-native-impl/tests/math/common_fence_action_native.cpp | 3C6A5E8C8AF42CBF138D84CF4E038D05E8FD732D8557723805FDD17BE331F194 |
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

字体证据、Common 原接口与旧类声明已经核查，Tree 实现的构建与测试也已完成。
接下来不要重复这些检查来代替执行文首下一步。若 HEAD 或相关文件实际变化，
只核对相应差异并更新本记录；继续保持一个构建、总并发 4/math 编译池 2。

分批暂存 CMake 不要过滤 zero-context diff 的前部 hunk 后直接应用后部绝对行号；
本轮因此曾把 types 行暂存到 FILE_SET 括号外，工作树构建虽过但暂存版本不正确。
最终提交已恢复正确列表位置，并对最终 HEAD 重跑 configure/build/test；不能用工作树
先前通过的日志推断每个中间暂存提交都能独立配置。
