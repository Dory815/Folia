# MapHandler module ownership

The pinned `MapHandler.ts` owns one module-private `maps` collection. Native
`map_handler.cpp` currently places that collection in a function-static
variable even though every native render session creates its own Context and
ParserRuntime. This is an architectural gap. A later session can replace names
used by an earlier session's late map lookup and retain its callbacks globally.

Root's required design:

- `map_handler.{ixx,cpp}` owns an instance registry and the existing Source
  register/get algorithms. There is no process-global registry, current-session
  selector, thread-local selector, or alternate implementation.
- `ParserRuntime` owns that registry as part of its one loaded module
  environment, like its existing ConfigurationHandler. Multiple TeX instances
  in that runtime share it. Separate runtimes have distinct registries.
- `TokenMapHost::create` and map initialization receive the actual registry
  explicitly. A token map need not retain the registry after registration;
  avoid a registry-to-map-to-registry ownership cycle.
- `SubHandler` and `SubHandlers` receive the same registry explicitly. Their
  late add/lookup paths and direct package callers use that instance.
- Package initialization, NotItem, Begingroup and SetOptions route through the
  runtime's actual registry. Update callers and native observers together;
  there is no implicit global default for standalone tests.
- Preserve Source map ordering, replacement, SameValueZero behavior, callback
  identities, priorities and warnings. Keep all locked Source observations.

The implementation owner is Luna `tag_setoptions`. Root coordinates shared
parser/package edits and builds. Implementation begins after Gate109 reaches a
terminal state; existing package owners must coordinate their affected calls.

Acceptance requires focused existing MapHandler tests plus an interleaved
two-runtime observation: register different maps under the same name, add them
to handlers after both registrations, call A/B/A/B, and check actual callback
and map identities in their original Contexts. Exercise late NotItem and
Begingroup lookup, same-runtime sharing, replacement, and release of registry
entries when their owning environment ends. A successful full renderer
comparison and the appropriate Core/Parser suites remain required afterward.
