# MathJax 原生移植交接入口

本文件是唯一当前 checkpoint。恢复时先读本文件，再只检查可能使当前结论失效的改动。架构与职责锁见
[native-raii-architecture.md](native-raii-architecture.md)，总验收门槛见
[native-raii-acceptance.md](native-raii-acceptance.md)。实质状态变化后更新本文件；不另建交接笔记。

## 当前 checkpoint：请求期内存门槛诊断中（75/150 已验）

- 无live build/process。Root已在architecture末尾锁定ParseRequest/TeXRequest：原ParseOptions/TeX owner内caller-owned栈上RAII frame，配置/session保留，NodeFactory及表达式数据进入frame；core compile接口不加通用payload/registry。下一动作按锁分派原owner/消费方/fixture迁移，freeze后Root review/build/原gate复验。禁止compile结束clear/retire或放宽CRT；当前request UT未通过，不能接入Folia或写完成。

- 最近完整math基线（加入生命周期观测前）：`canonical-modes-full-math.log` exit0，845 tests / 214872 assertions / 170 suites，无skip。配套parallel4 build `canonical-modes-build-fixed2.log` exit0。
- hosted 75 layout pairs、150最终Scene metrics、150完整SVG byte-equal：1/1279；default86：1/1021；leading：1/20；fonts：1/73。独立Source两child各75零异常，实际EventTarget只注册不dispatch。fixture SHA `5c970e41dc36f7717f1dbf31c576a6c875f445068284fc8c2cd7042e9a17985c`，write/default/no-op过。
- strict `canonical-modes-results.json`仍73equal+2 `source_error_native_success`（Lite缺addEventListener），strict原fixture SHA `31761f077a57e92177b38c44dba0808b78e6616e99e1242f50058aafbe8de8ba`不变。hosted通过不改变strict事实，也不证明浏览器事件行为。
- 本批提交：`c8e53353`架构、`8ce3d53b`共享输入、`060bd58d`共享测试管线、`5743e3fc`mspace默认leading、`71e4abe5`Source cacheID/outline来源、`7f83584a`Source/fixture、`af8c5ede`UT/CMake。独立空cache后缀与缺失glyph不同；bbm-bold感叹号outline来源为MathJaxNewcm/bold。
- generator512MB有界写出完整证据，保留整数键顺序与eager lookup副作用；40shards/80events/24646profile锁过，第二次88outputs mtime不变。旧OOM/初次hosted失败均已被上述结果取代，不要重复诊断。
- 下一批已由Root在architecture末尾锁定并提交`77445af9`：原owner无分配InstanceCount成员+Windows Debug CRT真实normal/client blocks/bytes；复用buildScene（已是scene-only入口），不复制compile管线/不改所有权/不加释放补丁。counter/MML/Common、DOM、UT三owner均freeze，Root已review/format/注册CMake。UT用12个无状态输入，192+768混合请求；错误路径另测，不能循环含宏/label/编号的canonical75断言零增长。
- request build和diagnostic-build均exit0，parallel4。两次focused正常exit1（1test/10asserts，5pass5fail）；无live build/process。日志`request-lifetime-objects-heap.log`（最新含差值）、`...-first.log`（首轮）。活MML/Common/DOM、owner退出和retained Scene均过；CRT门槛未过。12输入第二轮逐项重复，after192==after960：相对x baseline +5 blocks/144 bytes，最大+16/480；不是持续增长，仍不能写请求所有权通过。
- 新证据：ParseOptions.root/nodeLists/mathItem、TeX::mathNode_/latex_、textmacro请求scratch仍在持久对象上。CompileScope只清texhtmlAdaptor；tests/tex_jax.cpp:327+在文档活着时观测字段。common_scene_entry只读结论已收到：保留原配置/factory配置/tag定义/session语义，表达式equation/parsers/root/lists/error/factory借用及输入需caller-owned frame；接口仍待Root锁定，未实施。Counter/原owner观测已提交`54be844a`；未提交候选仅request_lifetime.cpp、heap_snapshot.hpp及CMake一行注册（有意保留失败证据，不冒充green）。未跑新的full。
- 接入只读结论（未实施）：现folia_math没有编入math.ixx/.cpp和C ABI bridge，WinUI仍QuickJS；原Renderer owner仍Context路径。后续保留C ABI UTF16/线程/取消/结果释放接口和WinUI队列/缓存，只更换原owner内部管线，仍通过SVG parser接入，不扩展D2D/额外Scene ABI。math.cpp既有dirty需保留，不可reset覆盖。

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
- 尚未完成：完整Source profile与剩余专用 family覆盖；request ownership 长跑；Folia
  integration；旧对象层完整退出。组件通过不代表这些总门槛通过。
- 已接受组件批次包括 XML/HTML、merror、table direct-MML 8 例、semantics/fenced 11 例、enclosure 86
  例、maction/cache、Unicode/Entities、mhchem、texhtml/Lite 及相关 TeX package。其精确 Source
  hash、聚焦断言、构建日志和限制留在 `native-raii-acceptance.md`；不得仅凭组件验收宣称应用集成。

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
