# Retry utility typed coverage boundary

`tools/math-port/retries-original-oracle.cjs` runs the locked original CJS for
`util/Retries.ts`, and `retries-original-source.json` contains five direct source
observations: retry error shape, normal return, retry then success, direct error,
and rejected retry promise. The generator checks the pinned and installed TS
bytes and the installed CJS hash.

`src/math/util/retries.ixx/.cpp` provides the matching typed API over the existing
`async::Promise` and `async::JobQueue`: `RetryError`, `retryAfter(PromiseRef)`,
`retryAfter(QueueRef)` for a resolved promise, and `handleRetriesFor`.

`ParserRuntime` now owns the session `QueueRef`, and
`TextMacrosMethods.CheckAutoload` uses `retryAfter(runtime.queue())` to carry
an actual queue-bound resolved promise. The Source global `Promise.resolve()`
and JavaScript-visible `Error.retry` property remain host integration work. Legacy
`MathJax.Callback.After` is intentionally unsupported.

`tools/math-port/parser-family-reference/retries-native-candidate.cpp` contains
six Boost.UT cases over actual `async::Promise` and `async::JobQueue`
objects, aligned to the five Source observations: RetryError identity/message,
synchronous success, retry and replay after resolution, direct error rejection,
and rejected retry promise without replay. A sixth case covers RetryError from a
returned Promise, including retaining the retry closure until the retry resolves.
Root's dedicated `FoliaMathRetryTests` run passed with 24 assertions across six
tests. This is typed native coverage; it does not claim JavaScript Promise
reflection or legacy callback support.
