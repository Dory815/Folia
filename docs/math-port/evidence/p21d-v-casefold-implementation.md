# P21d implementation evidence — v-mode case folding

This is an isolated candidate change, not an acceptance claim. The root-frozen
contract is `docs/math-port/regexp-v-casefold-experiment-contract.md`.

## Files and provenance

The only source edits are in the ignored P21d copy:

| File | SHA-256 after edit | Frozen predecessor SHA-256 (LF) |
|---|---|---|
| `build/math-port-regexp-v-casefold-source/src/parse.rs` | `db9a0588975f072671dfecbe309b156bb480dff6bb842e495b5c6b5a42c3e54a` | `34a075eaf65c01f5c4f3fbe527018f7573cf3dc034072b419bcab82abcbd864a` |
| `build/math-port-regexp-v-casefold-source/src/unicode.rs` | `ead02aef680de46559032fc6b19f926bb431ad5c87e6b774899a530debcac62b` | `4aebcc04392ddf6820b74bc470e3e9bfc7ea7bf486e0b203e3f589136c741635` |

The P21d property data table remains unchanged at
`475cb03589f0deb859b9a2ccfb34e3f33026a1e89500524024ce575a135e5daf`.

## Method and branch mapping

`unicode.rs` adds the private `PropertyCaseFolding::{Unicode, Intrinsic}` tag
next to `PropertyEscapeKind` (lines 344–358). `unicode_property_from_str`
(376–420) tags general-category, script, script-extension, ordinary binary,
and fallback general-category sets as Unicode folding policy. Only catalog
entries represented by `UnicodePropertyBinary::Ascii` or `Any` get Intrinsic
policy. StringSet values retain their previous representation and have no
new folding metadata. The actual range construction and the property catalog
are unchanged.

In `parse.rs`, `ClassSet::node` (76–89) no longer closes the complete point set
after set algebra; it still emits the same bracket and leaves string
alternatives on their prior `into_node(icase)` path. `union_operand`
(92–113), `intersect_operand` (115–175), and `subtract_operand` (177–231)
close literal singleton operands before applying their operation when `icase`
is true. Already-prepared class and character-class operands are consumed as
prepared. Both range-construction sites in `consume_class_set_expression`
(1067–1209; range branches around 1110–1127 and 1156–1174) first compare raw
endpoints, construct the original inclusive range, then close that range
before adding it to the set. Thus closure cannot rescue or change an invalid
raw range.

`codepoints_from_class_set` (344–353) closes the positive d/s/w point set
before applying D/S/W complement. `close_property_case_set` (356–364) closes
only when `icase` is active and the catalog policy is Unicode. The existing
top-level d/D/s/S/w/W helper `make_bracket_class` (369–388) already closes its
positive set before complement and receives Unicode mode for both u and v;
that path remains unchanged. Nested
character escapes and properties use these helpers in
`consume_class_set_operand` (roughly 1288–1382): positive and negative
d/D/s/S/w/W are prepared consistently; `\p` is closed according to metadata;
`\P` closes its positive property set and only then complements it. Nested
sets are parsed before they are passed as operands, so their own point
operations are prepared before any outer negation.

The top-level property branch in `consume_atom` (1715–1754) applies the same
policy for v mode: permitted closure first, then P complement, emitted as a
non-inverted bracket. The existing u-mode branch remains the old logic: it
does not use the new metadata and retains the prior complement-before-closure
order for `\P` under `iu`; non-ignore-case u behavior is unchanged. The
legacy non-Unicode parser path is untouched. The legacy Unicode whitespace
and word-class paths outside v-set parsing are also not globally rewritten.

## Scope and validation status

No matching, IR, compiler, UTF-16, string-set, q-escape, WSpace-alias,
property-table, flag-adapter, or production integration code was changed.
The contract's pre-generated actual-Node corpus and all six validation groups
remain for root-owned replay and review. I did not run builds, tests, corpus
generation, or replay, and this report does not claim semantic acceptance.
