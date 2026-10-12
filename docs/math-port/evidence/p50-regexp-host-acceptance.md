# P50 native FindTeX scanner RegExp acceptance

Root froze the portable interface, unimplemented host, CMake ownership and 34
independent original observations in 7cdd2cd. The stub compiled and failed those
observations. The completed host is 2a53b9d, using the existing vendored SRELL
through custom code-unit traits, without changing that dependency. This is a
language-host adaptation for the actual FindTeX scanner profile, not a MathJax
parser algorithm or a general replacement for all JavaScript RegExp behavior.

The existing SRELL UTF-16 traits always combine surrogate pairs. Root and a
readonly Luna audit identified the exact difference from FindTeX's non-u global
expressions. The adapter uses utf_traits_core<char16_t>, retaining one-unit
matching, positions and cursor updates. Another real caller difference is
quotePattern's valid escaped colon/hyphen: native compilation emits equivalent
literal character escapes rather than rejecting configured delimiter strings.

Validation:

- All 34 original observations pass. They use actual JavaScript RegExp.exec,
  including patterns produced by pinned original FindTeX and quotePattern.
  TS/CJS hashes and the SRELL hash are locked in the Source manifest.
- Four native assertions verify explicit unsupported-flag failures and flag
  validation order. No unsupported flag silently changes matching semantics.
- Seven independent faults compile and fail original assertions: scalar
  stepping, omitted identity-escape adaptation, ignored cursor, failure retaining
  the cursor, missing captures treated as empty, zero-width matches advancing,
  and relative instead of absolute positions. Restored baseline passes.
- Readonly Luna review found no blocking mismatch in this declared profile and
  confirmed the scope restriction. No agent edited the host or its gold.
- Complete core build/test passes, exit 0 (build/p50-full-core.log), alongside
  the P49 and prior core/math regressions. Oracle --check reproduces the gold.

pattern() retains the supplied compilation text; it is a diagnostic native
getter, not a claim of JavaScript RegExp.source normalization. Match captures
preserve missing versus empty strings. Input and all offsets are UTF-16. Plain,
global and sticky numeric-cursor behavior are independently witnessed.

Other flags, complete Annex-B/Unicode expression grammar, arbitrary coercion,
native Context exec/lastIndex binding and actual FindTeX algorithms remain future
gates. No full TeX, mathematical layout, SVG rendering or Folia integration is
accepted here. Mermaid remains paused; the full MathJax goal remains active.
