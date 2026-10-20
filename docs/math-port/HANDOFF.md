# MathJax 原生移植交接入口

本文件只记录当前执行所需信息，不是完整验收报告。恢复时先读本文件，再核对相关当前改动。架构与验收细节分别见 [native-raii-architecture.md](native-raii-architecture.md) 和 [native-raii-acceptance.md](native-raii-acceptance.md)。有实质决定或验证结果后更新本文件。

## 目标与固定输入

完成 MathJax 4.1.3 原生 C++/RAII 路径：固定 38 个 TeX 包完成 TeX→MML→共享 Common 布局→Scene→SVG，并由 Folia 替换 math QuickJS。Mermaid 暂停；没有 D2D 像素 gate；整体尚未完成。

- Source：`.reference/MathJax-src`
- Source commit：`fb987178c2d279a99b0db5ea02e751691703955e`
- Full38 compiled-source hash：`cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a`
- Hash profile：`tools/math-port/reference-profile.cjs`

## 最近接受基线

最近功能基线 `5838b3f5` 接受 Physics 整包（此前 Bussproofs 已完成）。`build/math-port/physics-package-full.log` exit 0：692 tests、201734 assertions、130 suites，无失败或跳过。已验包不重做；不得据此声称完整移植完成。

## 当前唯一断点：Unicode + Entities

正在进行；尚未构建、验收或提交。Root 负责 CMake、native build/test 和 Git。Luna `/root/unicode_finish` 负责原 Unicode configuration pair、unicode 测试/Source 工具；`/root/entities_tables` 负责 entities 生成器/Source 工具/fixture；`/root/entities_tests` 只负责 entities 测试。无活动 build。先完成这些交付再构建。

已冻结的架构：私有 `UnicodeCommandMap : CommandMap` 持有 codepoint→font 缓存；`MapHandler` 代表独立 Source 模块 registry，同 registry 的多个 TeX 实例共享缓存且 reset 保留，registry 析构释放缓存。`build/math-port/unicode-cache-scope-source.cjs` 的 Source probe 已验证共享与 reset。Unicode marker 是 `TexNodeProperties` 的 optional bool。

Entities 留在原 util owner，使用 typed string tables、loaded shard set 和同步本地 shard 安装；`entities::numeric(view)` 复用 `string_util::unicodeString`。Root 已改 entities 原 pair、Unicode marker 及 merge/false 测试。独立 Source witness `tools/math-port/entities-source.cjs` 与 `tests/math/fixtures/entities.inc` 已建立。

当前 Source witness 已运行：base 406、29 shards、2128 rows、21 numeric cases、19 completed translations。未解决点：3 个 shard entry 与 base 重复；旧 `entities-data-generate.cjs` 的 diff 生成方式会丢掉重复项的覆盖语义。下一步捕获实际 Source `add` 参数以保留完整 shard，再验证覆盖及 remove 后 reinstall。不得改 Source 期望来迎合 C++。

完成当前断点的顺序：

1. 完成 Unicode/Entities 独立 Source bytecheck。
2. 正式接入 CMake targets。
3. 冻结输入，单次一个 build，总并发 4。
4. 跑聚焦 math tests，再跑完整 math tests。
5. 分批提交明确路径。
6. 再移植 mhchem、texhtml；随后完成 Common→Scene 发射、SVG 对照、Folia 集成及旧对象层退出。

Common 测量产生的辅助 MML 节点由每次请求的 `MmlDocument` 持有；同一 document 的重复测量可增长至 document 析构。最终 renderer 每 request 新建 document，且不得把它留在 session 或 Scene 中。

## 工作约束与常用检查

保留 MathJax 原 owner/职责。不得建立 `native2`、`new`、`typed` 并行实现，不模拟 JS 对象，不加释放补丁。Source fixture 独立从 pinned Source 生成。Node Source 使用 heap 512、cwd `C:/Windows`、每次只 `createReference` 一次。

默认模型 Luna；必要时仅用 GPT-6.1 Sol。一次只运行一个 build，总并发最多 4。Windows console tests/probes 链接 `folia_console_test_runtime`；手工构建包含 `tools/test-support/windows_console_runtime.cpp`。CRT/abort 输出到 stderr。进程异常退出后停止广泛测试，先做有界聚焦定位。

标准 VS 18 开发环境构建：

```bat
call "%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake --build build/core --parallel 4 --target FoliaMathTests
```

聚焦测试使用 positional pattern：

```bat
build\core\FoliaMathTests.exe "*name*"
```

日志放在 `build/math-port`。

## 保护的既有 dirty 路径

以下变更是既有工作，不得顺手编辑、暂存或提交：

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

不得 reset worktree 或改写历史。提交时只显式选择本次目标路径，并保留无关工作。

