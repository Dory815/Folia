# Native execution evidence: Gate98

Gate98 builds and links all 28 targets, including the new raw DynamicFile
observer. All 1,631 captured input hashes remain unchanged during the build.

## Passing tests

The focused dynamic-provider test passes all six assertions. The subsequent
complete Core executable passes 345,656 assertions in 5,324 tests across 171
suites, exit 0. This is the latest fully passing complete Core run.
Its copied executable SHA-256 is
`39566903ca9a5567094abd6db02e109171b00a6728e3507ff1d4cb293da453b5`.
PID 54884 runs from 2026-10-14T15:47:57.637Z to 15:51:01.537Z.

Additional complete runs all exit 0:

| Executable | Assertions | Tests |
| --- | ---: | ---: |
| Foundation | 383 | 58 |
| Host | 465 | 197 |
| Parser | 864 | 56 |
| Regexp | 18,518 | 41 |
| LinkedList | 27 | 7 |
| Font | 179 | 21 |

The three actual instance-extension records match Source exactly. The native
dump exits 0, PID 67312, with copied executable SHA-256
`7052d45cc294b3e64b6180acf194abb61fe3ecaea8925492ce35c3426aeb520f`.
The strict comparer also exits 0. Actual `x+y` SVG remains byte-exact at 2,502
characters, and the completed Scene survives release of the native services.

## Renderer and package comparisons

Complete75 finishes with exit 0 and remains at 52/75 complete exact records.
Registration remains 23/38 packages with 15 warnings. Native returns 75 SVG
results; Source returns 73 SVG results and two errors. Renderer acceptance
and Folia integration remain open.

The copied executable SHA-256 is
`bbd3a545fea07bbb60b723e6feca239d41a8fdc775495d9057a4b4b43c3d8146`.
PID 74088 runs from 2026-10-14T15:47:41.334Z to 15:48:21.690Z.
The complete output SHA-256 is
`29e972a9a32033a33d24510a8d67d5fcd05750470b62c52461da0bce7fa5cd1b`.
The pinned Source SHA-256 is unchanged:
`9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740`.

The table corpus remains 12/14 complete exact records. Label creation and
paint records now agree; the first aligned-label difference moves to the
full-width SVG/container behavior. Colortbl parsing is still incomplete.
The cancel-lines Common boxes and placement calls now agree exactly, but its
SVG and paint observations retain differences. The separate 30-case Menclose
method-trace corpus still fails all 30 comparisons and is not accepted.

Next-six now emits all 20 records, but 0/20 match every field. The textcomp
mapping table contains a lifetime error; Cases and Centernot also retain
separate behavior errors. Font3 emits all 12 records with 10/12 exact; its
remaining differences are the BBM bold option update and textmacros package
record fields. These scores do not authorize new production registration.

The raw DynamicFile observer exits 1 before emitting JSON because replacing
a bound data property with an accessor remains unimplemented in Context.
The four strict records are therefore not accepted. The descriptor ownership
and DynamicFile access boundary are being corrected together.

Follow-up SvgMath percentage-width and Menclose serialization changes are
prepared after this build; they have not yet been compiled or accepted.
Mermaid remains paused. Actual process provenance, raw outputs and strict
comparison reports remain in ignored `build/math-port` output.
