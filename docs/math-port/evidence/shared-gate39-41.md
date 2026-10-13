# Shared native checkpoints: Gates 39 through 41

Gate39 exits 1: the actual MmlFactory reference lacked its module import and
two inherited create overloads were ambiguous. Gate40 builds all 12 selected
targets, but Core, Host, Parser and all five parser observers abort before main.
Root's GDB first-throw trace and LLVM symbolization locate `bad_optional_access`
in `derivedErrors`, invoked by `OpenItem::errors` dynamic initialization before
`BaseItem::errors` existed. Initializing derived tables in the Source module
registration order fixes this concrete failure for Gate41.

All three builds keep their 826 frozen registered/generated input hashes
unchanged. Gate41's selected targets freshly compile/link and its build exits 0.

| Fresh Gate41 observation | Result |
| --- | --- |
| HostTests | Exit 0; 458 assertions / 196 tests / 10 suites |
| ParserTests | Exit 0; 864 assertions / 56 tests / 7 suites |
| RegexpTests | Exit 0; 18,518 assertions / 41 tests / 3 suites |
| LinkedListTests | Exit 0; 27 assertions / 7 tests / 1 suite |
| Five whole parser corpora | 150/150 records match |
| Whole LinkedList corpus | 13/13 records match |
| Actual Unicode RegExp observer | Exit 0; 36/37 strict original records match |
| Full core | Exit 1; one assertion fails in the Unicode codepoint/index test |

The core's failed suite has 35 assertions / 13 tests, with one failure. Its
other 169 suites pass 345,572 assertions / 5,301 tests. Those passing-suite
counts do not describe a passing complete run; Gate36 remains the last complete
passing core checkpoint.

Unicode progresses from 27/37 in Gate38 to 34/37 in Gate40 and 36/37 in Gate41.
The remaining Gate41 tag-split record has identical parts; its emitted `.source`
leaves two slashes unescaped. The owner has a subsequent observer getter repair,
which requires a fresh build. Original records remain untouched and comparisons
still include exact source spelling, error messages and every field.

Ignored reproduction evidence includes `build/math-font-package-gate-41.log`,
`build/math-port/gate-41-inputs-before.json`, `gate-41-input-changes.json`,
`diagnostic-exits-41-root.json`, `core-run-state.json`,
`parser-checkpoint-41-root-fresh/summary.json`,
`unicode-checkpoint-41-root-fresh/summary.json` and
`build/math-core-gate-41.log`. The G40 first-throw and symbolization logs are
`build/math-port/gate-40-host-firstthrow-gdb.log` and
`build/math-port/gate-40-host-firstthrow-symbols.log`.

Gate41 core executable SHA-256:
`bdfedc0b40bcdb369d9b488534154ee8a487afee9f4082be0e477a76de456f38`.
Unicode observer executable SHA-256:
`5bf26f327dd1099dff85d937f911338fac4b53c1377d0d7243eb3f5ac69fb91e`.
Its unchanged original baseline SHA-256:
`e6c01910ecb588816a0b1c99f7746b7a9b21bd8abeb1cd84f93a3b7163e5d634`.

Common, Lite, completed Scene and SVG output are still uncompiled drafts at
this checkpoint. No complete native SVG path or Folia replacement is accepted.
Mermaid remains paused.
