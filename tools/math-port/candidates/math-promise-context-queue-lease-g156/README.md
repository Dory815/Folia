# Promise context bindings queue lease candidate

This candidate changes `folia.math.util.promise_context_bindings`, adds focused Promise ownership tests, and adds a real `ParserRuntime` / `FunctionList::asyncExecute` lifetime test. The patch changes no formal files and has not been built or run.

The isolated leak cycle is concrete: `JobQueue::promises_` owns each `PromiseState`; a pending state owns its reaction handlers; the current adapted Context reaction strongly captures `PromiseContextBindings`; and that bindings object strongly stores the same `QueueRef`. This closes `JobQueue -> PromiseState -> Handler -> PromiseContextBindings -> JobQueue`. The candidate makes the bindings queue reference weak and makes methods and adapted reactions lock the real queue for each execution. A pending handler captures only weak bindings. Expiry follows the existing `TypeError: Promise Context services have expired` boundary; no replacement queue or Promise wrapper is manufactured.

The runtime integration test intentionally distinguishes that leak from the normal session roots. `ParserRuntime` owns `queue_`, and `FunctionListContextBindings::State` owns both `asyncQueue` and Promise bindings. Installed FunctionList methods in the live `Context` capture that State. These are expected roots: after `ParserRuntime` shutdown, they remain while the `Context` projection is alive, but the inactive execution lease makes saved FunctionList and Promise methods fail explicitly. Destroying the Context and ParserRuntime releases those roots.

The new integration test uses a real `ParserRuntime`, its `functionLists`, a Context-bound callback returning a Promise payload, and the actual projected `asyncExecute` and `then` methods. It leaves the callback Promise unresolved, retains only external Promise handles and weak queue observation, shuts down the runtime, checks both stale method errors while Context is alive, then destroys the Context. The weak queue must expire despite the pending `FunctionList::AsyncFrame`, its real pending input reaction, the output Promise reaction, and the external Promise handles. This checks the actual service graph without treating the legitimate Context-owned State as a cycle. An independent owner-graph review confirmed that `AsyncFrame` itself holds only a weak queue and that there is no separate frame-to-queue edge.

The direct binding tests also create a real pending projected `.then` reaction and verify queue and bindings expire, retain canonical wrapper identity during a live rejection -> `then` -> `catch` chain, and check explicit errors for late invocation and a reaction that runs after bindings expire.

The patch passed `git apply --check` against the current G159 tree. No build or test was run. After Root accepts the candidate, the focused test target can be verified with:

```powershell
cmake --build build/core --target FoliaMathPromiseOwnerTests
build\core\FoliaMathPromiseOwnerTests.exe
```

The target uses C++23 module scanning and the repository's `Boost::ut_module`; both test translation units include `tests/support/folia_test.hpp` and do not import `std`.




## G168 focused test correction

`promise-context-queue-lease-g168-test-fix.patch` updates only the live-chain test. The previous test attached a rejection handler to `.then` that returned `"recovered"`, then expected the following `.catch` to run on that now-fulfilled chain. Source-compatible Promise behavior is the opposite: the `.then` rejection handler recovers the chain, so `.catch` is skipped. The corrected test covers both paths with the real Context projection and JobQueue: rejection passes through `.then` to `.catch`, and a separate `.then` rejection handler that returns a value fulfills its chain and suppresses the next `.catch`. This addresses the two G168 false assertions without changing Promise implementation or weakening queue/wrapper assertions. The focused patch passes `git apply --check` against the current tree. No build was run.
