# MathJax 原生移植交接入口

恢复或压缩后先实际读本文件，再检查使结论失效的新改动。唯一当前 checkpoint；完整职责、证据和配置事实见 [architecture](native-raii-architecture.md)、[acceptance](native-raii-acceptance.md)、[files](native-raii-files.json)。不另建进度本，不重做已接受广审。

## 当前状态与下一步

- 最近代码提交 **6f7cc0e7**：退旧 reference_leaf/native driver，Source oracle 输出改 ignored；其数学断言先保留到两个原测试文件（**5cf687cb**），无生产算法改动。Source display 探针6观察通过，聚焦 **6 tests/78 assertions**通过。最后 parallel4 build **85509 exit0**，相对聚焦后 test/DLL hash不变、保护11字节不变；日志 `build/math-port/mml-leaf-retirement-build*`。
- 上述新测试前的完整math **96710 exit0：870 tests/217144 assertions/176 suites/0 skips**；WinUI Debug真实worker **64450 exit0/PASS**。之后未重复full/worker；未验Release/GUI视觉。证据在acceptance及 `retire-scene-{full-math,worker-verify}.log`。
- canonical已刷新至 **6f7cc0e7**、**index-2026-10-21T08-41-46-0351.json**：668源/664代码、183未配置cpp/ixx、560配置源、247接口与map exact、保护11字节不变；Root断言在 `latest-retirement-inventory-check.json`。未配置不等于死代码。旧Scene/SVG重复布局与Common factory对象链均已分批退出；真实数学留原owner。
- **下一步**：Root完整读font_data_source及active字体合同，读三个Luna实读分包报告与CJS companion。首批已退该旧test+3旧JSON observer+2比较器，证据font-fontDataConsumers-before-removal.json，未改active代码/UT；继续退lifecycle、provider消费者小批，再退font绑定。immutable FontRepository没有旧charOptions/tuple/ListPatch mutator合同，不为旧observer恢复它们；Source/data/typed font gate保留。
- 当前无live agent/build/native。Direction职责窄审通过：保留原direction.ixx，接着将现有StretchDirection enum从font_data放回该owner并export-import/CMake登记，不删除或恢复旧字符串别名。Lite/MML旧provider仍有保护消费者及MML()/classBinding调用者，不能按无显式import先删。找ignored交付先rg --no-ignore；旧graph仅历史线索。

## 已接受架构与执行约束

TeX → request-owned MML → 原Common共享布局/placement → immutable Scene → SVG → Folia原SVG路径。保留MathJax模块职责/数学算法；不模拟JS对象，不建平行native/new/typed/placement层，不堆释放补丁。Scene无MML/DOM/请求指针和回调。未来D2D共用Scene，无像素门槛；Mermaid暂停。

Renderer/C ABI与worker已接入（9个C exports），QuickJS数学调用/链接退出，worker保留queue/cache/generation/cancel。既有Source SVG/几何、字体、UTF8/16+C ABI及192+768请求回基线已接受，细节留acceptance；strictLite为73 equal+2个Source缺addEventListener错误，不宣称事件dispatch。不要按旧反射字段/descriptor trace重建C++ API。

Root独占Git/build/configure/native；同一时间一个build、parallel≤4。异常退出停广测、有界诊断；console探针链接folia_console_test_runtime。小批提交、不reset、不覆盖无关工作；生成物/日志放ignored。保留active factory_types、js_number/js_numeric_input和原Common算法，不能按名称清理。

## 11条保护dirty

最新逐字节不变，基线 `build/math-port/scene-batch-before.json`。保护不是永久保留死代码：处置前Root先逐diff明确account，备份原字节与patch，再单独决定；不能顺手覆盖/暂存。媒体两项normalized blob同HEAD，仅CRLF/stat状态。

- src/app-winui/media/MathJaxSvgParser.cpp
- src/app-winui/media/SvgNormalizer.cpp
- src/math/adaptors/lite/lite_module.cpp
- src/math/adaptors/lite/lite_module.ixx
- src/math/core/mml_tree/mml_context_bindings.cpp
- src/math/core/mml_tree/mml_context_bindings.ixx
- src/math/core/mml_tree/mml_nodes/mtable_bindings.cpp
- src/math/core/mml_tree/mml_nodes/semantics_bindings.cpp
- tests/math/context_object_record_retirement.cpp
- tests/math/lite_array_view_request_retirement_g259.cpp
- tests/math/mml_private_renderer_allocation_domain_g227.cpp
