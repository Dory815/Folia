# MathJax 原生移植交接入口

本文件是唯一当前 checkpoint。恢复时先读本文件，再只检查可能使当前结论失效的改动。架构与职责锁见
[native-raii-architecture.md](native-raii-architecture.md)，总验收门槛见
[native-raii-acceptance.md](native-raii-acceptance.md)。实质状态变化后更新本文件；不另建交接笔记。

## 当前 checkpoint：table 新矩阵已验收，准备完整双模式 Source gate

- 本批提交：架构 `51d9e344`、独立Source/fixture `037e543f`、原生UT/CMake `5923227e`；生产代码未改。
- 最新 `build/math-port/table-profile-build.log` exit0，parallel4；full `table-profile-full-math.log` exit0：843 tests / 213573 assertions / 169 suites，无skip。聚焦Source14 UT1/669、table-regression42/4701、default86 UT1/1021过。所有build/native终止，无live handles。
- 真实MathML7场景×none/local：labels左右/含宽度、percentage/nested、ragged/align、horizontal stretch、break exact SVG。每张表最终BBox/TableData/形状/flags/pWidth及完整SVG在请求退出前后均对齐；原direct-MML8、SVG14、数值branch8不变。
- Source default/write/no-op和Root独立default（`table-profile-source-root.log`）过，fixture SHA `b23150d14cb8cdeb08e429428c87b6820f16bffb14eb87613165dad7209c12f8`。实际分支：nested pWidth3.9/父列6；ragged2/1cells+override；stretchy mo/table W3；break各2linebox/逐字符mi。不要退回TeX默认stretchy=false或多字符mi替代断行内容。
- `table-profile-results.json` / `table-profile-native.log`：canonical75正常exit1，73 byte-equal、2 `source_error_native_success`（action-toggle/action-tooltip，严格Lite缺addEventListener），无SVG mismatch/native exception；不算完整75/150通过。
- common_base_emission已交付只读候选`build/math-port/canonical-mode-witness-plan.md`，Root已读；所有Luna freeze，无live handles。保留旧严格Lite证据，拟额外同75输入的none/local150对照，host仅复用已验maction14真实EventTarget注册、不dispatch。尚未授权实现/改变gate。
- 下一具体动作：按计划核对common_scene.cpp现有TeX→Scene helper职责与共享方式，避免复制pipeline或继续堆大测试文件；锁新增Source/fixture/test职责后派Luna。canonical75是原18+profile57，两个独立cache session要同顺序，不把57误称75或重复执行改变session。不要重做已验table/mglyph或改原75输入/expected。可复用common_scene_source_tests（Source）、common_scene_entry（UT）、common_base_emission（writer/plan）、common_rows_emission（table）、scene_values_writer（Scene）；Root独占CMake/build/native/Git。

## 已接受的近期证据

- mglyph已验提交 `0f6fa2e0`/`ed6fd605`/`b0bd1b5d`，原Common算法/charWrapper保留，旧placement移除；13独立Source例退出请求后仍byte-equal。精确hash/计数见acceptance，完整math已被本批full覆盖。
- maction/cache已验提交 `897a0f50`（前序见Git/acceptance）。Resource order 由 Builder 实际生成次序决定；FontCachePlan 临时借用 Scene 并按 scope/Glyph
  地址索引，writer 只消费完整 prefix 列表，不分配 ID。
- Source cache witness
  SHA：`e00941e648aae4ce257f227b89c6f9e76974a4d3ac49c83987ff6708ff5e4fa1`；独立
  `cache-source-root.log` 通过 14 例，Source 原始 SVG 与 Git `175ecabf` 的旧 10 例一致。旧结果与细节见架构文档及验收记录。
- 上游固定为 MathJax 4.1.3、38 个 TeX packages；Source commit
  `fb987178c2d279a99b0db5ea02e751691703955e`，compiled hash
  `cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a`。profile：`tools/math-port/reference-profile.cjs`；范围：`docs/math-port/scope.json`。
- Source witness 使用 Node heap 512 MB、cwd `C:/Windows`，每个进程一次 `createReference`。默认 Luna，必要时仅
  GPT-6.1 Sol。

## 已接受边界与未完成目标

- 总目标：TeX → request-owned MML → 共享 Common layout/placement → immutable Scene → SVG → 现有
  Folia SVG 路径，替换math QuickJS。Mermaid暂停；无D2D像素gate，未来后端共用数学布局。SVG 不重算数学布局；Scene 不持有 MML、DOM、请求指针或回调。完整架构原则、既定修复和 Source
  细节以架构文档为准，避免在本文件复制历史。
- Common→Scene 使用 em/y-up；`SceneContainers.nodes` 是本 wrapper 的输出，`content` 是 children 的
  parents；SceneNodes 槽位可为空，不压缩。孩子由各自 wrapper 的 `placeScene` 放置；缺失 segment 保留槽位。完整接口见架构文档
  Common-to-Scene 段。
- Mglyph 保留 `CommonMglyph::getParameters/computeBBox` 和 legacy index `charWrapper`
  的现有布局与所有权。Source image 用一个 Image Scene 值，不加载/解码 URI，不从图像内在尺寸推导布局；图像追加到第一个 standard content
  root。Source 矩阵、属性顺序及空值边界以架构文档 mglyph lock 为准。
- 尚未完成：完整Source profile与剩余专用 family覆盖；canonical 75 layout /
  双模式 150 SVG byte comparison；request ownership 长跑；Folia
  integration；旧对象层完整退出。组件通过不代表这些总门槛通过。
- 已接受组件批次包括 XML/HTML、merror、table direct-MML 8 例、semantics/fenced 11 例、enclosure 86
  例、maction/cache、Unicode/Entities、mhchem、texhtml/Lite 及相关 TeX package。其精确 Source
  hash、聚焦断言、构建日志和限制留在 `native-raii-acceptance.md`；不得仅凭批次验收宣称完整 75/150 或应用集成。

## 工作规则

- 保留 MathJax 原 owner 和已验算法；不建平行 `native/new/typed` 实现，不模拟 JS 对象，不添加请求释放补丁。Source fixture
  独立生成，不能为迎合 C++ 改 expected。
- Root 独占 CMake、build/native tests、Git；同一时间只跑一个 build，parallel 不超过 4。交接前记录实际 build/test
  结果、日志路径、live handle 和下一具体动作。
- C++23 modules 使用 `import std;`。Windows console tests 链接
  `folia_console_test_runtime`；手工构建需加入
  `tools/test-support/windows_console_runtime.cpp`。异常进程退出后停止广泛测试，先做有界诊断。
- 标准构建：

```bat
call "%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake --build build/core --parallel 4 --target FoliaMathTests
```

- 聚焦测试使用 positional pattern：`build\core\FoliaMathTests.exe "*name*"`。日志及生成物放 ignored
  `build/math-port`。小批、明确路径提交；不 reset worktree 或改写历史。

## 保护的既有 dirty 路径

不得顺手修改、暂存或提交以下 13 条路径：

- `src/app-winui/media/MathJaxSvgParser.cpp`
- `src/app-winui/media/SvgNormalizer.cpp`
- `src/math/adaptors/lite/lite_module.cpp`
- `src/math/adaptors/lite/lite_module.ixx`
- `src/math/core/mml_tree/mml_context_bindings.cpp`
- `src/math/core/mml_tree/mml_context_bindings.ixx`
- `src/math/core/mml_tree/mml_nodes/mtable_bindings.cpp`
- `src/math/core/mml_tree/mml_nodes/semantics_bindings.cpp`
- `src/math/math.cpp`
- `tests/math/context_object_record_retirement.cpp`
- `tests/math/lite_array_view_request_retirement_g259.cpp`
- `tests/math/mml_private_renderer_allocation_domain_g227.cpp`
- `tools/math-port/native-raii-inventory.cjs`
