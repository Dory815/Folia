# MathJax 原生移植交接入口

恢复或压缩后先实际读本文件，再检查使结论失效的新改动。唯一当前 checkpoint；完整职责、证据和配置事实见 [architecture](native-raii-architecture.md)、[acceptance](native-raii-acceptance.md)、[files](native-raii-files.json)。不另建进度本，不重做已接受广审。

## 当前状态与下一步

- 字体旧消费者/比较器及14绑定源已分5批提交（末次46958781）。Source/data/typed gate保留；18文件及保护11逐字节不变，证据font-direction-retained-check.json；before-removal在font-*-before-removal.json。新Direction改动把现有StretchDirection放回原direction.ixx并从FontData re-export、CMake登记，无新类型/算法/别名。
- 最新parallel4 native **42226 exit0**；9聚焦 **9 tests/26340 assertions**通过，含完整Source SVG、分式/根式/表格/Mo/字体/BBox。WinUI Debug真实worker **56295 exit0/PASS**：join4372ms/8.93s/empty stderr。日志font-direction-{build,worker}.log及font-direction-focused-results.json。本轮未重复full math/full字体data gate；旧整跑96710（870/217144/176/0skip）及上轮leaf聚焦6/78仍留acceptance，不冒充新整跑。未验Release/GUI视觉。
- Direction已提交 **186a1fd7**；canonical已刷新至该代码提交，**index-2026-10-21T09-05-51-0865.json**：654源/650代码、168未配置cpp/ixx、561配置源、248接口与map exact、active未解析imports0。末次断言font-direction-final-check.json，18保留font文件/保护11仍逐字节不变。清单随本checkpoint小批提交。
- **下一动作**：处理保护的旧Context文件：Root先逐diff读9个非媒体dirty，备份原字节/patch并明确account，再分派Luna核实旧对象protocol消费者与真正数学断言。不要直接退Lite/MML provider：仍有MML()/classBinding及旧runtime/test调用者。新CM eager bundle/coverage的40/80为Source provenance，不是native JS模块loader合同。font消费者已清完，不重审旧tuple/ListPatch API。
- 当前无live agent/build/native。找ignored候选先rg --no-ignore；旧remaining-object-graph包含已退路径，只作历史线索。恢复依此checkpoint，不重做已接受广审。

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
