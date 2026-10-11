# P21b — legacy braced Unicode escape audit

## Scope and evidence

This is a read-only analysis of the P20d ignored-copy experiment. It proposes a
generic parser boundary for the remaining mismatches; it does not change the
candidate or accept the regex engine.

The P20d result is pinned to Node `v24.13.0`, Unicode `16.0`, ICU `77.1`.
`docs/math-port/evidence/p20d-lookahead-regexp-repair-result.json` reports 18
remaining mismatches in the `lookahead` corpus's `v_implies_unicode` run
(4,014/4,032 equal, zero regressions). All 18 are in
`surrogate-speculative-escape`. The corpus adapter's `v_implies_unicode` mode
sets Unicode mode only when the `v` flag is present; these 18 use flags `""` or
`"i"`, so they exercise legacy mode. The distinct patterns are ordinary
concatenation and repeated groups containing `\\uD800\\u{41}` or
`\\uDBFF\\u{41}`, and character classes combining those high-surrogate
escapes with braced escapes and ranges. Two additional class cases use
`\\uDBFF\\u{DC00}`. The tested subjects contain a surrogate followed by `A`
or another surrogate. In each mismatch the candidate consumes a legacy
`\\u{...}` sequence as one code point and reports a match where Node reports
none. These are syntax-mode errors, not failures of P20d's failed-low-surrogate
rollback repair.

Relevant source identities:

| Source | SHA-256 | Relevant methods |
|---|---|---|
| P20d ignored candidate `build/math-port-regexp-lookahead-source/src/parse.rs` | `3c2a911d8b65389d1a657f371784777393b2d374f7b4de4b0c3c5f2f31f2967e` | `consume_character_escape` (1501–1604), `try_escape_unicode_sequence` (1768–1864), `try_consume_named_capture_group_name` (1866–1934) |
| Pinned `.reference/regress/src/parse.rs` | `591243258a598d58e9ed9c4998bff3d75004ceb5dc15bb40e2f35b9c2e813833` | `consume_character_escape` (1487–1590), `try_escape_unicode_sequence` (1750–1837), `try_consume_named_capture_group_name` (1839–1905) |

The P20d contract is `docs/math-port/regexp-lookahead-experiment-contract.md`.
Its repair snapshots the cursor before the speculative second `\\u` prefix,
so an unsuccessful surrogate-pair trial restores the whole prefix. That change
is independent of whether braced escapes are permitted in the current grammar.

## Call graph and mode distinction

In the candidate, `consume_character_escape` reads the escape introducer and,
for `u`, computes `unicode_mode = flags.unicode || flags.unicode_sets` at
1551–1553 before calling `try_escape_unicode_sequence(unicode_mode)`. If that
call fails, it uses the legacy identity-escape path when `!flags.unicode`
(1555–1558); in Unicode mode it reports an invalid Unicode escape. The helper
currently names its Boolean `combine_surrogate_pairs`, but its braced branch
at 1771–1808 does not consult that Boolean: it accepts `\\u{...}` in legacy
mode too. Thus `\\uD800` is kept as a UTF-16 unit when the Boolean is false,
but the immediately following `\\u{41}` is still converted to `A`. That is
the generic cause of the 18 mismatches.

All ordinary character-escape routes converge on `consume_character_escape`:

| Candidate call site | Parsing context |
|---|---|
| 1028 | Character escape within a traditional character class |
| 1315 | Character escape in a Unicode-set class set character |
| 1342 | Character escape in a Unicode-set class-set escape |
| 1703 | Atom-escape fallback after the named-backreference probe |
| 1761 | General atom-escape fallback |

The atom parser dispatches through `consume_atom_escape` (definition 1606),
and class parsing has a separate path through `consume_class_set_character`
(1325). None of these callers should contain independent braced-escape rules;
they should all inherit the same ordinary character-escape mode decision.

Named capture identifiers deliberately take a different route. The group
parser calls `try_consume_named_capture_group_name` at 671; the named
backreference path calls it at 1714, and another capture/group path calls it
at 2007. Within that method, the first identifier unit calls
`try_escape_unicode_sequence(true)` at 1877 and continuation units call it
again at 1902. Those calls do not pass the regexp's u/v mode. Node 24.13.0
confirms `new RegExp('(?<\\\\u{61}>a)')` succeeds without a flag and exposes
group key `a`; the `\\u{61}` identifier escape must therefore remain
available independently of ordinary-pattern Unicode mode. The same method
uses `true` to combine escaped surrogate pairs before checking Unicode
identifier start/continue membership.

The pinned upstream parser has the same structural separation but no Boolean
parameter: `consume_character_escape` calls `try_escape_unicode_sequence()` at
1538 and named-capture identifier parsing calls it at 1849 and 1872. Its helper
also begins by accepting `{...}` unconditionally (1750–1790). The P20d
candidate's new parameter controls pair combination, not braced syntax. A
mode gate must not silently change the named-identifier grammar or the
P20d rollback behavior.

There is a separate lexical requirement in the same helper: ECMAScript's
`HexDigit` grammar excludes `+`, but the candidate delegates digit validation
to Rust `from_str_radix` at all three parse sites: braced code points (1786),
the first `Hex4Digits` value (1812), and the speculative second surrogate's
`Hex4Digits` value (1843). The loops currently collect general source
characters before those conversions. Node 24.13.0 rejects `\\u+041` and
`\\u{+41}` in Unicode mode, and rejects `(?<\\u{+61}>a)` as a named capture.
The identifier route remains mode-independent, but still requires strict
Unicode-escape digit syntax. A generalized repair should validate every
collected unit against ASCII `0`–`9`, `A`–`F`, or `a`–`f` before radix
conversion, preserving each site's existing rollback/error behavior; this
must not be folded into a mode-specific code-point special case.

## Generic repair boundary for root

The ordinary escape path needs two independent pieces of context: whether the
current grammar permits braced code-point escapes, and whether a parsed
four-hex high surrogate should speculatively combine with a following escaped
low surrogate. Today one Boolean represents only the latter. A generic repair
should keep those decisions distinct: the ordinary character escape passes
the current u/v Unicode mode for braced syntax, while named-capture identifier
escapes retain their identifier-grammar allowance and surrogate combining.
In legacy ordinary mode, a rejected braced trial must leave the cursor at the
position after the consumed `\\u`, allowing `consume_character_escape` to take
its existing identity-escape fallback and leave `{...}` for the enclosing
grammar. In u/v mode, valid `\\u{...}` remains a code-point escape and malformed
forms continue through the existing Unicode-mode error path. Hex4 handling,
P20d's pre-prefix rollback snapshot, and the two identifier call sites must
remain independently tested.

This is a diagnosis and boundary only. The source calls above and exact edge
semantics should be turned into new Node witnesses before any candidate edit;
the existing P20d corpus is sufficient to locate the 18 failures but is not a
full acceptance suite for the proposed mode separation.
