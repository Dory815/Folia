# Native MathJax worker lifecycle harness candidate

This candidate is an incremental patch over `folia-native-math-bridge.patch` and the unaccepted `drafts/math-cancellation-candidate.patch`. It adds the existing app `MathJaxRenderer.cpp` and `MathJaxSvgParser.cpp` to `FoliaWindowsEditorTests`, plus a private factory seam that keeps the actual production queue/cache/generation/revision/worker implementation under test. The injected renderer callable and its session are still constructed, called, and destroyed on the renderer's real `std::jthread`; it does not introduce a second queue or render implementation.

The focused tests cover generation invalidation and retry after `Clear`, completion revision updates and cache contents, independent renderer owners/workers, infrastructure conversion of a native `RenderError`, cooperative worker stop and worker-thread session destruction, completion callback thread ownership, and a queued dispatcher-style callback that retains only a weak UI owner. The queued-dispatch test uses a deterministic local queue because the platform test target does not initialize WinUI. The factory seam is private and is accessible only to the test friend declared in the test translation unit.

Apply order, only after Root accepts every prerequisite:

1. `tools/math-port/drafts/math-cancellation-candidate.patch`
2. `tools/math-port/candidates/folia-native-math-bridge-g145/folia-native-math-bridge.patch`
3. `folia-native-math-worker-harness.patch`
4. `cancellation-cmake-registration.patch`

The separate CMake registration patch adds the cancellation module and implementation that the draft currently adds but does not register. The worker patch adds the real app worker sources to the Windows test target. No builds or tests have been run for this candidate. Mermaid sources and its SVG normalizer deployment are untouched; QuickJS compilation and MathJax JS assets are removed only by the bridge candidate. Mermaid still uses `folia_svg_normalizer.dll` and has no QuickJS API calls.

After prerequisite acceptance/application, run the platform test target with the filters `"*clear drops an active generation*"`, `"*separate MathJaxRenderer owners*"`, `"*native RenderError becomes*"`, `"*destruction cancels active work*"`, and `"*queued dispatcher callback*"`, then run `cmd /c build_platform_test.bat 2>&1`. Build the app after `powershell -ExecutionPolicy Bypass -File .\setup.ps1` using `powershell -ExecutionPolicy Bypass -File .\build_app.ps1 -Configuration Debug -Platform x64`. The core cancellation draft must also pass its focused cancellation tests and the public renderer session tests before applying this bridge.

Lifecycle boundary: the app worker passes its `std::jthread` stop token into the draft renderer's cooperative cancellation overload. Shutdown requests stop before joining and the renderer checks that token at its instrumented parser/layout/scene/serialization checkpoints. `Clear` advances the existing generation; an already-running request finishes or observes cancellation, and its result is rejected by the existing generation comparison. The cancellation draft must be accepted and its checkpoint coverage validated before shutdown latency can be considered bounded. This worker candidate does not assign a per-request deadline, so request timeout behavior remains open against the former QuickJS two-second interrupt limit.

Known dispatcher risk: the current `EditorSurfaceRenderer::Initialize` math completion path posts a lambda capturing raw `this`. Clearing the worker callback prevents later notifications, but it cannot revoke a lambda already posted to `DispatcherQueue`. The local queued-callback test covers weak-owner behavior as a deterministic harness model; it does not initialize WinUI or prove the production lambda safe. This lifetime issue is reported to Root and should be resolved in an accepted app-side follow-up before treating the dispatcher lifecycle gate as closed.
