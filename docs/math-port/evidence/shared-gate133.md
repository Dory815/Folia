# Gates132–133: processMath projection and fixture corrections

Gate132 captures 1,965 unchanged inputs and ends with namespace errors in
two new test fixtures. Gate133 corrects those names and builds all 51 targets
with the same 1,965 inputs unchanged. Complete Core, Host, Parser, Regexp
and LinkedList pass. The four new Core failures reported at Gate131 are
resolved without changing their intended behavior checks.

Lite fontFamily now ignores its node argument in both Source branches.
The Foundation XML fallback test passes, and the standalone XML capture
completes. Its strict comparison still fails: two added nativeChecks envelope
fields violate the locked schemas, and two foreignObject namespace differences
remain. The supplemental native checks must move to UTs, retaining their
assertions; the Source output and comparer remain unchanged.

SVG.prototype.processMath now resolves the actual Jax, wrapper and Lite DOM
parent and invokes the existing SVG algorithm. Its pinned CJS reflection and
native metadata UT agree. The full tooltip record reaches the same Source
TypeError, with the same 23 layout calls and 21 paint records. Its layout root
and wrapper snapshot are absent because the nested Context invocation does
not notify the existing layout observer. That shared observation boundary
still needs correction. Full rendering therefore remains 73/75, with 37/38
registered packages and the Bussproofs warning. No acceptance is claimed.

Foundation has one failing newly added test: it incorrectly expects texttip
to follow the non-text tooltip branch. Source texttip creates an SVG title;
the existing complete texttip SVG comparison continues to pass. The test must
exercise the actual mathtip input for the Source error and preserve texttip's
successful SVG behavior.

Public API remains 9/9. Fonts remain 13/16 and next-six 18/20, first differing
at the standalone Jax adaptor fixture, with the parser/Stack alias still
pending. Other previously passing package and owner comparisons pass.
Stack, parser, factory fields, XML creator, lifetime and cancellation packets
remain under review. Folia still uses its math QuickJS worker; Mermaid is paused.

All evaluation processes are terminal. The comparer audits passed before
subsequent edits for the next gate. Ignored evidence includes the Gate133
input manifest and unchanged-input report, diagnostic-exits-133-root.json,
package-captures-g133.json and checkpoint-comparisons-g133.json.
The full native record hash is
`e5118fb8d22c7ea40f6a9beeb4422ddc9a10b015e41887ae470e2b7bdd84bd15`;
the full Source hash remains
`9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740`.
