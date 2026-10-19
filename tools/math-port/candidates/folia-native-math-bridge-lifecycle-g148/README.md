# Folia native Math worker lifecycle closure candidate

This is a candidate-only incremental patch on top of the accepted prerequisite sequence; it does not alter the repository's formal files. It closes the native request timeout/session recovery and dispatcher-owner lifetime gaps while preserving the existing app queue, cache, generation, completion revision, SVG parser, and SVG painting consumer.

`MathJaxRenderer::State::Run` gives each request its existing 2-second QuickJS render budget as a `steady_clock` deadline and forwards it with the worker stop token to the G147 native API. The deadline is cooperative; a single native operation without a checkpoint can exceed it. The worker catches typed `Cancelled` and `SessionUnusable` `RenderError`s, discards that worker-owned renderer on the worker thread, and does not cache the first cancellation result. A subsequent request creates a fresh session. No error-message parsing or second cancellation mechanism is added.

The real `EditorSurfaceRenderer::Initialize` completion closure captures a weak pointer to its shared `InvalidationState`; queued dispatcher work locks that state, clears the coalescing bit, checks `active` under the state mutex, increments the shared generation, and invokes the registered invalidation callback. MathJax, SVG-normalizer, and Mermaid completion notifications use the same lifetime-safe closure; their rendering behavior and Mermaid worker/runtime are unchanged. `EditorSurfaceRenderer` destruction remains UI-thread-owned by the WinUI window lifecycle, deactivates the state and clears its callback, and worker shutdown joins after stop is requested. `InitializeForPdf` does not register a dispatcher callback.

The platform unit test directly exercises the exact production helper used by `DispatcherQueue`: inactive-but-still-live state, destroyed owner with an already queued callback, successful callback on the queue-pump thread, generation/coalescing behavior, and enqueue rejection. Worker tests cover active-generation invalidation and same-key document-cache refresh after `Clear`, two independent renderers, worker-owned native sessions, per-request deadline forwarding, cancellation followed by session recreation, native error preservation, stop propagation, worker-thread destruction, and completion callback thread ownership. The WinUI OS dispatcher itself is not initialized by `FoliaWindowsEditorTests`; `winui-lifecycle-harness.ps1` is the concrete app-level smoke entry for the real WinUI close path after the app build.

Apply after the prerequisite patches in this order:

1. `tools/math-port/candidates/math-cancellation-g147/math-cancellation-g147.patch`
2. `tools/math-port/candidates/folia-native-math-bridge-g145/folia-native-math-bridge.patch`
3. `tools/math-port/candidates/folia-native-math-worker-harness-g145/folia-native-math-worker-harness.patch`
4. `lifecycle-closure.patch`

G147 already registers the cancellation module in CMake; do not apply the historical `cancellation-cmake-registration.patch` as a duplicate. This lifecycle patch was checked after G147, bridge, and worker-harness prerequisites. No builds or tests were run for this candidate, and no patch was applied to the formal checkout.

After Root accepts the whole chain, run focused worker tests and the deterministic Windows platform suite:

```powershell
cmd /c build_platform_test.bat "*deadline is passed*"
cmd /c build_platform_test.bat "*clear drops an active generation*"
cmd /c build_platform_test.bat "*separate MathJaxRenderer owners*"
cmd /c build_platform_test.bat "*production math dispatcher helper*"
cmd /c build_platform_test.bat
```

After `powershell -ExecutionPolicy Bypass -File .\setup.ps1` restores the WinUI/NuGet/native-SVG dependencies and the app is built with `powershell -ExecutionPolicy Bypass -File .\build_app.ps1 -Configuration Debug -Platform x64`, run the real WinUI lifecycle smoke (replace the executable path as needed):

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\math-port\candidates\folia-native-math-bridge-lifecycle-g148\winui-lifecycle-harness.ps1 -AppPath .\build\app-winui\bin\x64\Debug\Folia.exe
```

The harness opens a real WinUI window with queued display-math work and posts `WM_CLOSE` after a short delay; it passes only if normal window shutdown finishes with exit code zero. This is a lifecycle smoke, not a deterministic assertion that a particular dispatcher callback was pending at close; the deterministic production-helper test covers the queued-callback lifetime boundary. The native deadline remains cooperative and should be assessed alongside G147 checkpoint tests and the app-level smoke.
