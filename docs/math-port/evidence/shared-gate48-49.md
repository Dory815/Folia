# First linked output closure: Gates48 and 49

Gate48 exits 1 with all 1,029 input hashes unchanged. Its only remaining
compiler failure is CommonMmultiscripts' attempted BBox-to-LineBBox
construction. The Source accumulator is a LineBBox. Keeping that actual type
also avoids slicing its line-leading/indent/start/end/originalL data.

Gate49 fixes that declaration and builds all 13 selected targets, exit 0.
Actual build session 41665 terminates successfully; all 1,029 input hashes
remain unchanged. This is the first linked current Common/Lite/Scene/SVG
closure, not acceptance of complete rendering behavior.

Root executes the newly linked binaries, recording their hashes and actual
process exit codes:

| Runner | Assertions | Tests | Suites | Exit |
| --- | ---: | ---: | ---: | ---: |
| FoliaTests | 345,607 | 5,314 | 170 | 0 |
| FoliaMathHostTests | 458 | 196 | 10 | 0 |
| FoliaMathParserTests | 864 | 56 | 7 | 0 |
| FoliaMathRegexpTests | 18,518 | 41 | 3 | 0 |
| FoliaMathLinkedListTests | 27 | 7 | 1 | 0 |

The complete core execution is PID 54828, from 09:22:47 through 09:25:40 UTC,
with SHA256
`9cbdf20b40470380db7e474007ca98d77a42fd03bfac1dd502607b28f40237c5`.
Its five diagnostic runner records are in ignored
`build/math-port/diagnostic-exits-49-root.json`; full core output is
`build/math-core-gate-49.log`.

The fresh production Unicode observer matches all 37 exact original records,
including pattern source spelling and UTF-16 indices. The independently
re-executed original output still has SHA256
`e6c01910ecb588816a0b1c99f7746b7a9b21bd8abeb1cd84f93a3b7163e5d634`.
Five fresh parser corpora match 150/150 whole records (37 Base, 37 Frontend,
16 Bold/ConfigMacros, 24 AMS and 36 AMS Boundary). The actual LinkedList dump
also matches the independently re-executed 13-record Source corpus exactly;
its executable hash is
`62b7882e9545c9f99b18043d58e0e4f8777136471ef58f9471a7692ba617937a`.

The new foundation executable exits 1. Its three actual Lite projection tests
pass 18 assertions, but both completed-Scene ownership/precision tests throw
`std::bad_function_call` while serializing through a directly constructed,
unbound LiteAdaptor. This is observed runtime failure, not a passing Scene
checkpoint. The executable SHA256 is
`0635557362abad63feb1444e661da5bb105606c071a8dc9a0963fc09a6a78364`;
output is `build/math-port/FoundationTests-49-root.log`.

Root locates unguarded LiteParser serializer calls to the empty Context string
callback. Owners save a correction after release. Root also changes the Scene
fixture to use the actual LiteModule/Context adaptor constructor, preserving
all ownership, geometry and repeated-serialization assertions. LiteModule
now requires the real shared RegExp service rather than constructing another
object registry. Those changes require fresh compilation and execution.

Source virtual constructor phases, canonical wrapper Context binding before
initialization, real LayoutJob lifetime leases, remaining wrapper kinds,
FontData class reflection and the full 75-case production render comparison
remain required. Folia integration waits for that acceptance; Mermaid stays
paused.
