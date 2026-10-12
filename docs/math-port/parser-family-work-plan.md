# Sustained parser-family implementation assignments

The earlier small-method delegation did not make effective use of three Luna
workers. The next batch assigns complete Source responsibilities, not prewritten
method templates. Root owns cross-module architecture, integration and final
acceptance; each worker owns its design, full method implementation and a
Source-method coverage ledger. The shared production declaration graph is P53.

Architecture remains actual TeX -> TexParser/Stack -> actual MML -> common
output layout -> independent SVG/Direct2D backends. No evaluated JS, fake parser,
flattened pseudo-MML or renderer-specific parser/layout shortcuts are accepted.
One ParserRuntime/Context represents a loaded module environment; actual
configuration/tag registries and canonical identities survive formula sessions.

Concurrent exclusive ownership:

1. Configuration/runtime worker: complete Configuration and ParserConfiguration
   in configuration.cpp, ParserRuntime lifetime and class metadata in
   parser_runtime.cpp, corresponding configuration/runtime interface corrections.
2. Tags worker: complete actual Tags/AbstractTags/NoTags/AllTags/TagsFactory in
   tags.cpp, actual ColumnParser in column_parser.cpp, their interface corrections
   and a portable URI encoder if Source formatUrl requires one.
3. TexParser/ParseUtil worker: complete actual TexParser, ParseUtil and
   KeyValueDef bodies and module initialization, alongside independent executable observers against the
   pinned actual Configuration/Tags/ColumnParser classes; coverage and identity/
   getter/priority/lifetime/error observations, original gold and native fixture
   design. Observe real original peers; never use fake TexParser or TeX to make
   construction easy. Native tests cannot substitute mocks for missing production
   classes. If a peer is missing, record the actual dependency and test seam.

Workers may design local helpers and required Source-facing interfaces. Changes
to another owner's file must be negotiated through Root. Do not change existing
accepted bodies to fake unresolved dependencies. Missing peer implementations
are tracked as remaining work, not replaced by accepting stubs. The new complete
implementation units are enabled together after the real class graph compiles
and links. Until then, their unimplemented peers prevent acceptance.

Root resolved the first shared seams: TagsFactory::initializeModule receives
the two actual built-in TagsClassInfo descriptors after runtime identity buckets
and constructor objects exist. Its allocation constructor registers nothing.
ColumnState's ordinary fields remain one Context record accessed by its thin
native reference; no parallel C++ field values are synchronized. TexParser's
instanceIdentity is its actual allocation handle, bound once by the eventual
make implementation before Source initialization, so ColumnState.parser can
point to the same object. The handle is not a substitute parser implementation.

Root alone owns shared CMake/build targets and runs the combined builds, avoiding
parallel mutation/build interference and duplicate generated trees. Workers may
run their independent Node observers and source/coverage checks. Candidate native
fixtures stay under tools/math-port/parser-family-reference until their actual
peer dependencies are available; then Root enables them in the normal core target.
Workers do not stop after a plan or one method: continue through the full assigned
class/file responsibilities, original observer coverage and dependency report.

The configuration worker also owns the complete ordinary Array host packet
(slice, reverse, includes, push, pop, join, fill and find), its frozen original
observations and corresponding native cases. Follow-on ownership is the generic
iterator protocol and its real Configuration consumers; package-specific loops
are not a substitute for the Source iterator helpers. Root owns the shared
native construct dispatch and runtime integration. Tags owns its class
prototype registration hook as part of the actual Tags module, preserving
AbstractTags/NoTags/AllTags inheritance and emitted-CJS descriptors.

The local Codex configuration requests eight child workers. The current
conversation still advertises four total concurrency slots, so the three child
workers run these sustained responsibilities concurrently. Do not claim that a
configuration edit has changed this conversation's tool allocation, and do not
create unrelated threads or shell-launched agents to evade that allocation.

Acceptance requires actual complete declared method bodies, preserved Source
field/operation order and graph identity, independent pinned-original outputs,
relevant intentional-error detection and a combined build/core test. Original
observations are frozen before comparing against native outputs. No result is
accepted solely because an agent reports completion or existing narrow tests
pass. TeX/package parsing, fonts, common output layout, SVG comparison and Folia
integration remain the full objective; this batch does not redefine completion.
