# Folia native Math bridge candidate (G171)

Candidate only. This patch is not applied to the repository's formal app sources. It is intended for review after the native math API and behavior gates pass and Root explicitly chooses to integrate Folia.

`MathJaxRenderer` keeps its existing per-instance priority queue, request cache, generation invalidation, completion revisions, and SVG parser/normalizer consumer. Each `MathJaxRenderer` worker owns one `folia::math::Renderer` session from construction through destruction. Its native call receives the worker stop token and a `steady_clock` deadline set to the existing two-second QuickJS render budget immediately before the render call. The deadline is cooperative; a native operation without a checkpoint can run past it. Typed `Cancelled` errors remain transient and do not trigger a QuickJS fallback. If the next call reports `SessionUnusable`, the worker destroys that session on its own thread and leaves the transient request uncached so the UI can request it again using a fresh session. Other native errors become the existing infrastructure-result form and follow the existing SVG/UI path.

The app build links the CMake-produced `folia_math.lib` and searches the matching `CMakeFiles/folia_math.dir` IFC directory. `build_app.ps1` builds `folia_math` first, using separate Ninja build trees for Debug/Release and dynamic/self-contained CRT choices. The WinUI project continues to compile its existing core and platform modules itself. The math source import uses the CMake IFCs; it does not duplicate the math source tree inside the WinUI project. `MathJaxSvgParser.cpp`, SVG normalization, and Direct2D consumption remain in the app path. The patch removes only the MathJax QuickJS bundle/font assets and QuickJS build/link target from Folia; the QuickJS vendor files remain untouched. Mermaid files, runtime, and build integration are not changed.

`EditorSurfaceRenderer` uses a concrete shared `EmbeddedInvalidationState` and `QueueEmbeddedCompletion` helper for its dispatcher callback. The worker callback and queued UI callback capture only a weak state pointer. The queued callback checks `active` under the state mutex, increments the shared document generation, then invokes the registered UI invalidation callback. Destruction deactivates and clears the callback; a queued closure cannot retain or access a destroyed renderer. The WinUI window owns the interactive renderer on the UI thread. `InitializeForPdf()` installs no dispatcher callback, so its worker-thread-local renderer has no queued UI closure.

The CMake platform test target adds the real app-side `MathJaxRenderer.cpp` and `MathJaxSvgParser.cpp`, links `folia_math`, and picks up the new tests through its existing `tests/platform/*.cpp` glob. Worker tests use the actual `MathJaxRenderer` queue/cache/thread implementation with an injected native-call seam; they cover clear during an in-flight generation, fresh same-key completion/revision, independent renderer workers, stop propagation and worker-thread session destruction, per-request deadlines, `Cancelled` followed by `SessionUnusable` and session replacement, native error caching, and worker callback thread identity. Dispatcher tests use the exact production state/helper and cover coalescing, active generation updates on the pump thread, enqueue rejection, deactivation, and a queued callback after state destruction. No build or tests were run for this candidate.

Apply the single patch only after Root approves the bridge:

```powershell
git apply --check tools/math-port/candidates/folia-native-math-bridge-g171/folia-native-math-bridge-g171.patch
git apply tools/math-port/candidates/folia-native-math-bridge-g171/folia-native-math-bridge-g171.patch
```

Before the app build, restore the normal WinUI dependencies:

```powershell
powershell -ExecutionPolicy Bypass -File .\setup.ps1
```

Then the candidate build script builds the matching CMake math target and invokes MSBuild. After Root applies the patch, validate Debug and Release; also validate `-SelfContained` if that deployment mode is used. The current repository CMake configuration requires Ninja, CMake 4.x, Node for generated math data, and a Visual Studio compiler compatible with the IFC producer.

Focused platform checks after integration:

```powershell
cmd /c build_platform_test.bat "*clear drops an active generation*"
cmd /c build_platform_test.bat "*separate MathJaxRenderer owners*"
cmd /c build_platform_test.bat "*typed cancellation detects an unusable session*"
cmd /c build_platform_test.bat "*production dispatcher closure*"
cmd /c build_platform_test.bat "*already queued production callback*"
cmd /c build_platform_test.bat 2>&1
```

WinUI lifecycle smoke after app build:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\math-port\candidates\folia-native-math-bridge-g171\winui-lifecycle-harness.ps1 -AppPath .\build\app-winui\bin\x64\Debug\Folia.exe
```

The WinUI smoke opens an actual document with display math, allows the real worker to enqueue work, then posts `WM_CLOSE` to the real window and requires normal process exit. The deterministic platform tests cover the exact shared dispatcher helper without starting a WinUI dispatcher.
