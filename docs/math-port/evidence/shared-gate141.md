# Gates138–141: Stack regressions restored; renderer acceptance remains open

Gate138 builds the current Stack, parser fields, computed instance-field
initializers, FunctionRef Source metadata and Object.prototype conversion
changes. One test helper resolves `options` to Boost.UT's type instead of the
math namespace. Gate139 fixes that compile error, but its input audit rejects
an accidental edit to an existing factory test while building. The original
test is recovered from its earlier complete tool output, with exact SHA-256
`d7d8931d75db17dc7fbc71df1e4062a79631949e15448ac5b88e944d8acedf60`.
The candidate factory test remains separate. Gate139 is not accepted evidence.

Gate140 builds all 53 targets with 1,987 captured inputs unchanged. The six
Gate137 Stack regressions are gone. Core has one obsolete concat expectation;
Host has five obsolete conversion/join expectations, including that shared
concat test. Independent Node observations confirm plain-object numeric
conversion yields NaN and these array operations treat the resulting bound
as zero. Join retains its captured length, including the separator after an
element shrinks the array. No production result is changed to satisfy them.

Gate141 updates those expectations and adds the shrinking-join test. The new
StackOwners fixture also receives the Source Alpha constructor length and the
typed Property argument required by StartItem. All 53 targets compile with
1,989 captured inputs unchanged. Complete Core, Host, Parser, Foundation,
Regexp and LinkedList pass. The real subclass-construction tests exercise
inherited setters, rejected setters and a nonwritable reused receiver: the
computed initializer is evaluated once at its field position, and assignment
goes once through Context rather than pre-writing the native owner.

Object.prototype.toString/valueOf now provide ordinary object conversion,
primitive boxing, string code-unit properties and observed Symbol.toStringTag
lookup. Symbol's fallback brand and the primitive prototype records have
independent Node/native tests. This is not a claim of complete JavaScript
intrinsic coverage; Map/RegExp and other unmodeled brands remain a boundary.

The original Bussproofs, utility, expanded and session comparisons pass again;
expanded coverage is 21/21. The session observation has the same Source/native
SHA-256, `c8003603a905fbea8412c61831b4764a671b47670f5f6c6e1a20b9e5236a21ac`.
ParserFields passes exact array replacement, OpenBrace/CloseBrace, old aliases
and fresh Parse ownership. The new StackOwners capture still fails: its
fixture discards the TeX compile result and then queries the unset MathItem
root. A candidate using the actual MathItem.compile/root field, together with
the complete Alpha SourceSurface, is prepared but not yet applied or built.

Full rendering stays 74/75, with 37/38 actual package registrations and the
unchanged missing Bussproofs registration. API is 9/9; complete texttip SVG is
exact. Physics, Colortbl, Mathtools, Tags, Mhchem, Attributes, parser projection,
map sessions and host/layout comparisons pass. Fonts stay 13/16, now first
exposing BboldxConfig's callback name. Next-six stays 18/20, now first exposing
the actual configurations PrioritizedList instead of an empty carrier. Two
standalone XML namespace differences remain. The full native observation is
byte-identical to Gate134:
`795255bf0b7f754b17b9a49ae7a107873925825a6572e8b03a8a09ac34b23881`.

All Gate141 evaluation processes are terminal and their input/binary audits
pass. The last diagnostic finishes at 2026-10-15T16:32:40.103Z. A supplemental
Alpha/StartItem Source witness is edited at 16:32:51.587Z, after those processes
end but before Root explicitly releases the freeze. No compiled source or
executed comparison changes. This later edit is recorded separately in
`build/math-port/gate-141-post-evaluation-changes.json` and must be included in
the next frozen snapshot. Existing draft filenames can be registered inputs;
their directory name is not permission to edit them during a gate.

Evidence is under build/math-port: Gate141 input/audit manifests, diagnostic
exits, package captures, comparison report and executable provenance. Runners
reuse build/core executables with before/after hashes; no per-gate executable
copy is created. Build completion now keeps the state marked frozen until
evaluation is explicitly closed.

Factory fields, live XML/Lite ownership, FunctionList replacement, remaining
callback/configuration projection, lifecycle and cancellation candidates are
not enrolled here. This checkpoint does not accept the renderer or integrate
it into Folia. The math QuickJS worker remains; Mermaid is paused.
