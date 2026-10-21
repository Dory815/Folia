# MathJax 原生移植交接入口

本文件是唯一当前 checkpoint。恢复时先读本文件，再只检查可能使当前结论失效的改动。架构与职责锁见
[native-raii-architecture.md](native-raii-architecture.md)，总验收门槛见
[native-raii-acceptance.md](native-raii-acceptance.md)。实质状态变化后更新本文件；不另建交接笔记。

## 当前 checkpoint：mglyph 已验收，下一批 table profile

- 本批提交：架构 `ca8ce3ef`、Scene/writer `0f6fa2e0`、原 SVG owner 与旧 placement 清退 `ed6fd605`、独立 Source/UT `b0bd1b5d`。
- 最新 build `build/math-port/mglyph-build-final.log` exit0，parallel4；full `mglyph-full-math.log` exit0：842 tests / 212904 assertions / 168 suites，无skip，包含既有default86与组件回归。
- 聚焦 Scene3/17、writer3/5、owner+catalog4/26、Source13 UT1/222均过。13例为8 images、3 legacy index、2 invalid verification merror，后两者不计image成功；完整SVG在请求退出前后均byte-equal。
- Source13 default/write/no-op及Root独立default（`mglyph-source-root.log`）通过；fixture SHA `ef8fe550e9d54839050ffa896556f7d8adc555b02f184ed827a476d05d31e1ab`。BBox namespace与测试parent role假设已修，生产/fixture未迎合修改。Image属于标准mglyph metadata group，不能要求其父Group为image_glyph role。
- `mglyph-results.json` / `mglyph-native.log`：canonical75正常exit1，73 byte-equal、2 `source_error_native_success`（action-toggle/action-tooltip，严格Lite缺addEventListener），无SVG mismatch/native exception；不算完整75/150通过。
- 所有build/native结束，无live handles；Luna均freeze。可复用owner：scene_values_writer（Scene）、common_scene_entry（SVG owner）、common_base_emission（writer）、common_scene_source_tests（独立Source/UT）、common_rows_emission（table）。Root独占CMake/build/native/Git。
- 下一具体动作：读 ignored `build/math-port/table-profile-next.md`，只核对新增table覆盖，再锁定职责/独立Source矩阵后分给Luna。已有direct-MML8、table SVG14和数值branch8保持原样；新覆盖重点labels左右/含宽度、percentage/nested、ragged/align、horizontal stretch、break exact SVG。不据旧完成消息重开已验调查。

## 已接受的近期证据

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
- 尚未完成：table 完整 profile；剩余专用 family；canonical 75 layout /
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
