# Folia native math bridge candidate

This directory contains a review candidate only. Apply `folia-native-math-bridge.patch` from the repository root only after Root accepts it. It switches the existing `MathJaxRenderer` worker to `folia::math::Renderer`, retaining the request queue, priority, result cache, generation filtering, completion revisions, existing SVG-fragment parser, native SVG normalizer, and Direct2D consumers. Mermaid code and its normalizer deployment are not changed.

The worker lazily constructs one non-copyable renderer on its `std::jthread`, uses it serially for all requests on that worker, and lets it destruct as the worker exits. Disabling rendering clears queued work and advances the generation, requests stop, then joins. An active synchronous native render completes before join; its result is dropped by the existing generation check. A document cache reset does the same generation invalidation without stopping the worker. Completion notifications still use the existing dispatcher callback and the worker is joined before `MathJaxRenderer` state is destroyed.

Formula diagnostics remain in the SVG `data-mjx-error` path parsed by `ParseMathJaxSvgOutput`. `folia::math::RenderError` retains its `name` and `what()` text as an infrastructure error; other standard exceptions retain `what()`, and unknown exceptions receive a fixed infrastructure message. There is no native typed transient failure or cancellation API today, so this candidate does not retry by matching error text and cannot interrupt an active parse/layout/render. Shutdown and PDF cancellation wait for that render to return. This is the main behavior change and must be accepted against real workload latency before shipping.

The WinUI project candidate configures/builds the existing CMake `folia_math` target in `build/core` before app compilation, gives MSVC the target IFC search directory, and links `folia_math.lib`. The application remains a WinUI 3 project and keeps its 32 MiB stack setting. JavaScript bundle/font assets stop being copied by the project; MathJax license notices remain. Mermaid still uses its current `folia_svg_normalizer.dll` path.

## Review and verification

`git apply --check tools/math-port/candidates/folia-native-math-bridge-g145/folia-native-math-bridge.patch` passes against the current checkout. No formal file was changed, no build was run, and this patch has not been applied.

`tests/math/bussproofs_renderer_session.cpp` exercises repeated calls through one public `Renderer`, ordinary formula rendering between calls, independent renderer instances, and this candidate adds an invalid-TeX diagnostic assertion. After Root accepts the candidate and it is applied, the narrow acceptance run should include:

- `cmd /c build_test.bat "*public renderer reuses*" 2>&1` and `cmd /c build_test.bat "*independent public render sessions*" 2>&1` for the public renderer session behavior.
- `cmd /c build_platform_test.bat 2>&1` for the deterministic Windows editor suite.
- After `setup.ps1`, `powershell -ExecutionPolicy Bypass -File .\build_app.ps1 -Configuration Debug -Platform x64` to exercise CMake target construction, IFC discovery, MSVC import, and static linking.
- Focused WinUI worker checks for queued work after `Clear`, an in-flight result after document switch, two documents using independent `MathJaxRenderer` owners, and shutdown during an active synchronous render. The existing platform test executable does not currently host `src/app-winui/media/MathJaxRenderer.cpp`, so these worker checks need a WinUI-side test seam or a controlled app-level harness before this candidate can satisfy the full worker lifecycle acceptance gate.
- Exercise PDF export cancellation with a deliberately long native render and record that cancellation waits for synchronous rendering to return, then completes without a stale callback or owner access.

Mermaid's existing render path should receive a quick regression check during the app build/review because this patch leaves its code, assets, and normalizer calls untouched.
