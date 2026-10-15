# Shared build after Gate110

The latest successful shared compile is Gate113. No later production
registration changes have yet been built or accepted as an integrated renderer.

Gate111 selected 46 targets and captured 1,901 inputs. It terminated with
exit 1; all inputs remained unchanged. The errors were explicit registry
arguments missing from Physics/Mathtools helper parameters, plus TexHtml and
HTMLDomStrings calls that did not match the current native APIs. Their Luna
owners corrected the responsible callers and module imports.

Gate112 selected the same 46 targets and captured 1,904 inputs. It terminated
with exit 1 and zero changed inputs. The production packet compiled, but
seven observer/test translation units failed: stale NodeFactory conversion
arguments, incomplete explicit registry fixture wiring, incorrect Property
namespace/UTF-16 conversions, ambiguous empty MML arguments, and Context
calls that passed a Value where an ObjectRef was required. Those owners are
correcting the actual fixtures and calls without changing Source outputs.

The text-layout identity failure has been localized to
`TextParser::copyLists`. `ParseOptions::addNode` creates ordinary Context
arrays containing canonical MML node identities. The previous copyLists
incorrectly resolved the array itself as a native ChildArray. Its candidate
correction uses the existing Context iterable protocol and resolves yielded
nodes individually, preserving the Object.entries snapshot, partial effects,
and clearing lists only after successful completion. Focused draft checks
exercise ordinary arrays, live iterator methods and propagated exceptions.
These changes require a new successful shared compile and official captures.

The 358-module dependency audit reports no missing imports, cycles, duplicate
interfaces or Scene imports of backend-specific modules. That audit does not
establish behavioral equivalence. Luna owns the implementation and test
packets; the bounded Sol callable-boundary review is complete.

Gate113 compiles all 46 targets with 1,904 unchanged captured inputs. Core
passes 345,659 assertions / 5,325 tests / 171 suites. Host, Regexp and LinkedList
pass; Parser still fails. The earlier three Scene layering failures pass;
new TexHtml/HTMLDomStrings/XML tests fail. Full renderer comparison is 53/75
(73 layouts, two errors), public SVG comparison is 8/9 and actual registration
remains 23/38. Aligned now returns SVG and differs in its generated transform.

Official package comparisons: Colortbl 29/29 and its map match; Physics 61/63
and 9/11 maps; next-six 16/20; font packages 13/16; Mathtools 35/42 and 3/5 maps,
with its package-order supplement byte-exact at 2/2. TagSetOptions emits all
23 records but initially differs in observer field order. Bussproofs, Map
sessions and TexHtml observers fail fixture initialization. The parser
projection observer is stopped by its recorded 60-second timeout. Complete
Array.unshift and Common getBBox comparisons pass, as does getBBox lifetime.

Mhchem's first official G113 package capture exactly matches all 220 cases,
63 replacement stages, six rules, two maps and 14 mutations. Its dependency
comparison passes all 7,960 records. A repeat again shows the single Unicode
difference, but same-executable raw-byte evidence identifies a capture error:
decoding stdout Buffers individually splits U+2062 into two U+FFFD characters.
Replacing just those bytes reproduces the differing capture byte-for-byte;
the raw native output is correct. This is not an MML/parser Unicode failure.
The capture code now retains raw bytes, hashes them and decodes once after
process completion. A regression checks every byte boundary and bytewise
delivery of mixed UTF-8, including U+2062 (73 checks), plus invalid-byte
reporting. New observations must use the corrected capture helper.

After G113, Root enrolls Colortbl and Mhchem in session initialization as a
candidate for the next shared build. That does not switch Folia. Further
isolated Luna corrections remain candidates until a fresh shared checkpoint.
The missing handler graph is a separate complete binding packet documented
in `../canonical-handler-graph-plan.md`.
