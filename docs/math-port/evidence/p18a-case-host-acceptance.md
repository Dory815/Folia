# Root P18a — general native Unicode lowercase primitive

This is a host string operation required by maction's actual toLowerCase call,
not a new MathJax source-file implementation or renderer gate. Root owns its
interface, implementation, generated tables, independent source observations,
tests and mutations. No JavaScript interpreter runs in the native operation.

Authority is the pinned reference runtime: Node 24.13.0, Unicode 16.0,
ICU 77.1. The table generator observes every code point through actual default
String.toLowerCase and Unicode Cased/Case_Ignorable properties. It emits 1,460
changed mappings and 617 property ranges. Payload SHA256:
127fe1e81da77464d5a476855efd2db40700587da69e880cfdef4a475d98b23e.
--check regenerates the exact source. Native binary searches consume these
immutable compiled tables. Full mappings retain expansions and supplementary
characters; the generic UTF16 decoder retains lone surrogates. Final sigma
checks the original input on both sides, skipping Case_Ignorable before testing
Cased, including characters belonging to both properties. Locale tailoring is
not part of default source toLowerCase.

The independent oracle does not read the tables or native algorithm. It executes
actual String.toLowerCase for all 1,114,112 isolated code points and three
context forms for every code point (3,342,336 observations), compressed into
1,162 context ranges. Another 474 exact strings exercise long ignorable runs,
mixed scripts, both-property precedence, expansions, supplementary characters,
lone surrogates and deterministic random streams. All 477 native assertions
pass. Both generators reproduce their outputs under --check.

All 14 isolated deliberate mutants compile and fail unchanged assertions with
exit 1: omitted mapping, supplementary offset, surrogate replacement, missing
final sigma, both cased predicates, both ignorable scans, incorrect precedence,
truncated expansion, omitted supplementary mapping, missing ASCII cased data,
binary-search equality and first-range boundary. The runner restores isolated
copies and byte-checks working source/fixtures. Results are
p18a-case-host-mutations.json; per-mutant logs remain in ignored build output.

Full core build/test passed (build/p18a-core.log, exit 0). The build also compiles
the pending action/registry bodies, whose independent source acceptance remains
separate. This prerequisite does not increase 47/246 accepted source files or
pass complete TeX/Common/SVG or Folia integration gates.
