# Ordinary iterator host and Configuration iteration

The iterator oracle executes the locked MathJax 4.1.3 CommonJS
`cjs/input/tex/Configuration.js` (`__values`, lines 2–11; `__read`, lines
13–28) through its actual `ParserConfiguration` constructor and `addFilters`
method. It also evaluates the exact pinned `__values` function text to inspect
the fallback iterator shape. The source hash and CommonJS bundle hash are
recorded in `parser-iterator-source.json`; 21 observations cover dynamic
iterator and `.call` reads, Function.prototype.call metadata/getters/deletion,
plain array-like fallback, close-on-abrupt-loop behavior, null lookup, bounded
tuple destructuring, close-error precedence, and the actual ConfigurationHandler
method metadata, overwrite order, and live key iterator. Every row names a
candidate in the native fixture.

`Context::getIterator` models the transpiled `__values` helper: it performs a
live `Symbol.iterator` Get, invokes the current method through its current
`.call` property, and uses the helper's live array-like `length`/indexed-Get
fallback only when the method is falsy. The canonical Array iterator stores
only the original array identity and cursor, rereads length on each `next`,
and remains completed after its first done result. String iteration preserves
JavaScript code-point iteration across UTF-16 surrogate pairs. Iterator result
objects are actual Context objects.

The `__values` array-like fallback is a plain object with an enumerable own
`next` function and no inherited `Symbol.iterator`; it is not an instance of
the native iterator prototype. Dynamic `.call` lookup is backed by the actual
inherited `Function.prototype.call` function, so inherited getters, an own
`undefined` shadow, and deletion of the inherited method remain observable.

`Context::forEachIterable` follows the emitted `for...of` loop and `finally`:
it dynamically calls `next`, reads `done` before `value`, closes an unfinished
iterator after a body/step failure, and preserves the original error over a
close error. `Context::readIterable` matches the bounded `__read` helper: when
`Symbol.iterator` is falsy it returns the original value for subsequent
indexed destructuring; otherwise it builds an actual array, dynamically calls
`push`, and runs IteratorClose after bounded reads or abrupt completion.

`ParserConfiguration.initializeParserConfiguration` now consumes the result
of its dynamic `packages.slice().reverse()` calls through `forEachIterable`.
Processor-list loops use the same protocol, and each tuple uses
`readIterable(tuple, 2)` before indexed reads, preserving the transpiled
destructuring and close order. The generic host also exposes its source-owned
fields through `ParserConfiguration::sourceField`, a thin read from its sole
canonical Context identity for ParseOptions integration.

`ConfigurationHandler.initializeModule` installs the exported ordinary
Context object with `set`, `get`, and `keys` callbacks. Its `keys` iterator
retains the actual live registry iterator. Registry names are the package
strings used by the enabled source closure; arbitrary non-string Map keys and
non-Configuration Map values remain explicit unsupported boundaries.

The native candidate at `tools/math-port/parser-iterator-reference.cpp` is
outside automatic test discovery and is explicitly linked into the production
library's `FoliaMathHostTests` target. Root ran the combined host suites:
239 assertions in 149 tests passed. This includes a first-yield/second-step
failure whose throwing `return` must preserve the earlier step error.

Root's method-scoped faults include bounded-read closing, no-iterator identity,
live fallback length, prior-error precedence, and loop-body IteratorClose.
All 33 combined Array/Object/iterator faults compiled and failed their native
witnesses, then passed after exact source restoration; results are recorded in
`parser-host-native-faults.json`. This is bounded host acceptance and does not
accept the entire ParserConfiguration, TeX frontend, layout, or SVG profile.
