# P19c — `regress` parser repair experiment

This experiment changes only an ignored copy at
`build/math-port-regexp-repairs`, copied from `.reference/regress` revision
`771e6c8fa1e472ef86c45b5206b37fa4e4494e25`. Before editing,
`src/parse.rs` in the copy and reference had the same SHA-256:
`591243258a598d58e9ed9c4998bff3d75004ceb5dc15bb40e2f35b9c2e813833`.
The `.reference` source remains untouched. The experiment changes only
`build/math-port-regexp-repairs/src/parse.rs`; that path is ignored by Git.

## Repair 1: reserved double punctuation

In `consume_class_set_character` (copied parser lines 1332–1345), the original
lookahead rejected any two adjacent members of the reserved-double-punctuator
set. For example, `!#` was treated like `!!`. The standard's reserved double
punctuators are repeated identical pairs (`!!`, `##`, `&&`, etc.), so the
lookahead now additionally requires equality of the current and next codepoint.

Before:

```rust
if Self::is_class_set_reserved_double_punctuator(cp)
    && let Some(cp) = self.peek()
        && Self::is_class_set_reserved_double_punctuator(cp) {
            return error("Invalid class set character");
        }
```

After:

```rust
if Self::is_class_set_reserved_double_punctuator(cp)
    && let Some(next_cp) = self.peek()
    && cp == next_cp
{
    return error("Invalid class set character");
}
```

## Repair 2: surrogate-pair decoding context

The original `try_escape_unicode_sequence` combines adjacent escaped high and
low surrogates into one Rust scalar unconditionally. Ordinary character
escapes must preserve the legacy UTF-16 units unless the regexp is in Unicode
mode. The call in `consume_character_escape` now passes
`flags.unicode || flags.unicode_sets`, since the `v` flag enables Unicode
semantics even though `Flags` stores `unicode` and `unicode_sets` separately.
When false, the helper returns the high-surrogate unit without consuming the
following `\\u....` escape.

The first version gated all helper calls directly on the regexp flags. The
upstream `run_regexp_named_capture_groups` test failed: that grammar decodes
astral identifier escapes into a Unicode codepoint even without `u`/`v` flags.
The final experiment therefore makes the helper's pair-combination choice an
explicit argument: ordinary character escapes pass the actual `u || v` mode,
while the two named-capture identifier call sites pass `true`. This preserves
the existing named-group grammar and tests, while keeping ordinary legacy
character escapes as separate units. That grammar-specific exception is based
on the source's named-capture parser and the pinned upstream test, not on an
assumption that all regex escapes are Unicode-mode escapes.

Relevant before/after excerpts in `consume_character_escape`:

```rust
// Before
if let Some(c) = self.try_escape_unicode_sequence() {
```

```rust
// After
let unicode_mode = self.flags.unicode || self.flags.unicode_sets;
if let Some(c) = self.try_escape_unicode_sequence(unicode_mode) {
```

Relevant helper behavior after the change:

```rust
if (0xD800..=0xDBFF).contains(&u) {
    if !combine_surrogate_pairs {
        return Some(u as u32);
    }
    // Existing second-surrogate parsing and backtracking follow unchanged.
```

The `\\u{...}` codepoint-escape logic and unrelated parser branches were not
changed.

## Verification and boundary

`cargo test --manifest-path build/math-port-regexp-repairs/Cargo.toml -p regress
--target-dir build/math-port-regexp-repairs-target` passed all 549 reported tests
and doc tests, with one doc test ignored. During iteration, the first
unconditional flag gate caused the named-capture test failure above; adding the
explicit named-identifier decoding context restored the full pass.

This is an isolated upstream package test run, not the root-owned harness or
an engine compatibility claim. The copy is for the parent to compare against
its original and repaired variants. No production source, manifests, fixtures,
or root harness files were changed.

## Root independent replay

Root reviewed the actual parser diff, verified the unchanged reference hash,
and reran the identical 119,218 Node cases through the same native probe against
the repaired copy. `node tools/math-port/regexp-repair-experiment.cjs` verifies
both parser hashes and unchanged API/indexer/Unicode tables/manifest, then
builds a separate ignored probe; the JSON contains case identity and results.
The normalized-v configuration improves from 2,347 to **1,964 differences**;
literal flags improves from 6,847 to 6,464. Each has 383 newly matching cases
and zero previously matching cases that become different in this corpus.
All 333 added-v initialDigit failures are removed, plus 50 targeted cases.

These two repairs are not committed production implementation and do not close
case folding, set algebra/string properties, Unicode dataset, zero-width
surrogate assertions or stateful JS host operations. The candidate remains
unaccepted. The replay requires the ignored repaired checkout described here;
it fails explicitly if either parser hash differs.
