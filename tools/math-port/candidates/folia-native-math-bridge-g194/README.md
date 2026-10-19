# Folia native Math bridge candidate (G194)

This is a review candidate only. Its complete change is [folia-native-math-bridge-g194.patch](folia-native-math-bridge-g194.patch); it replaces the earlier G171/G193 candidates as a single patch. It has not been applied to the formal tree, built, or tested.

The app's `MathJaxRenderer` still owns the existing per-instance request queue, cache, generation, completion revisions, and SVG consumer. Each enabled renderer has one worker; that worker constructs, uses, and destroys its own `folia::math::Renderer`. Every native call receives the stop token and a `steady_clock` deadline two seconds after that request begins, matching the old QuickJS render budget. The deadline is cooperative and cannot interrupt a native operation until it reaches a cancellation checkpoint.

`Cancelled` and `SessionUnusable` remain typed native errors. A cancelled request is not cached and does not fall back to QuickJS. Because the public API does not say whether `Cancelled` occurred before mutation or during rendering, the worker retains its session until a later call reports `SessionUnusable`; it then drops that session on the worker thread. The `SessionUnusable` request is also left uncached. A subsequent request creates a fresh renderer on that same worker. Other `RenderError`s retain their native name/message in the existing infrastructure-result path. A non-`RenderError` exception becomes a generic native-renderer infrastructure error.

Successful native output is the complete SVG string. It goes through the existing `ParseMathJaxSvgOutput`, SVG normalization, render-cache and Direct2D path. No app-side layout or second SVG representation is introduced. Invalid TeX remains represented by the renderer's existing error result and current UI handling.

The WinUI project imports `folia.math` from the CMake-generated IFC directory and links the matching `folia_math.lib`, while continuing to compile the app's core/platform modules and native SVG parser/normalizer through the existing project. The candidate's `build_app.ps1` uses the existing `build/core` tree for the selected Ninja configuration and CRT, reconfiguring that same tree when those settings change. `Folia.vcxproj` always points at that one tree. Debug, Release, dynamic CRT, and self-contained static CRT app builds must therefore run serially; they reuse and reconfigure `build/core` rather than creating per-mode CMake trees or copying libraries/IFCs. The WinUI output remains under `build/app-winui`.

Before configuring/building math, `build_app.ps1` imports the selected Visual Studio installation's `vcvars64.bat` environment, matching the repository's test scripts. It checks that the existing CMake cache's `CMAKE_CXX_COMPILER` is the `cl.exe` selected by that environment. A mismatch stops with a diagnostic instead of building IFCs that the WinUI compiler cannot consume. The current `build/core` cache identifies the VS 18 `cl.exe`; the project requests C++23 modules with that toolset. `build_platform_test.bat` also configures and builds `build/core`, so do not run it concurrently with `build_app.ps1` or switch configurations while either is active.

QuickJS calls, bundle/font loading, asset-copy entries, and build/link steps are removed from the MathJax path. The settings text now describes the native renderer. QuickJS vendor sources and their notices remain in the repository. `MermaidRenderer`, Mermaid assets, its runtime, and its rendering/build integration are unchanged. The `EditorSurfaceRenderer` completion adapter is shared by math, SVG normalization, and Mermaid: its lifecycle is made safe for all three, while Mermaid rendering semantics are preserved.

For interactive WinUI rendering, `MainWindow` owns `EditorSurfaceRenderer` and registers its invalidation callback on the UI thread. Worker completion only posts to that panel's `DispatcherQueue`; the queued closure locks the weak shared invalidation state, checks that it is active, advances the document generation, and invokes the callback on the UI pump. Renderer teardown deactivates the state and clears the callback. The deterministic platform test executes this exact production helper with a queued closure after teardown. PDF export initializes a worker-local `EditorSurfaceRenderer` without a dispatcher callback, so it does not post into UI objects. The real WinUI smoke test opens a document with display math, waits for worker activity, then closes the actual window normally.

The platform test target compiles the actual app-side worker and SVG parser, links `folia_math`, and uses the repository's `tests/support/folia_test.hpp` / `Boost::ut_module` setup. The worker tests cover cache clear during active work, fresh same-key generation and revision, independent renderer workers, cancellation/session replacement, per-request two-second deadlines, native errors, callback thread identity, and destruction with active work. Dispatcher tests cover coalescing, generation change on the pump thread, failed enqueue, deactivation, and a queued callback after its state owner has been destroyed.

After the native math gates pass and Root chooses to integrate the bridge, use this serial acceptance sequence from the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\setup.ps1
powershell -ExecutionPolicy Bypass -File .\build_app.ps1 -Configuration Debug
cmd /c build_platform_test.bat "*clear drops an active generation*"
cmd /c build_platform_test.bat "*separate MathJaxRenderer owners*"
cmd /c build_platform_test.bat "*typed cancellation detects an unusable session*"
cmd /c build_platform_test.bat "*native RenderError becomes a cached infrastructure result*"
cmd /c build_platform_test.bat "*production dispatcher closure*"
cmd /c build_platform_test.bat "*already queued production callback*"
cmd /c build_platform_test.bat 2>&1
powershell -ExecutionPolicy Bypass -File .\tools\math-port\candidates\folia-native-math-bridge-g194\winui-lifecycle-harness.ps1 -AppPath .\build\app-winui\bin\x64\Debug\Folia.exe
```

To validate Release or `-SelfContained`, run that app build and the desired platform checks only after the previous build/test process has finished; the single CMake tree is reconfigured in place. After the app has been built in that mode, pass its corresponding `build/app-winui/bin/x64/<Configuration>/Folia.exe` to the lifecycle harness. Restore dependencies only in the accepted integration setup, not while reviewing this candidate.
