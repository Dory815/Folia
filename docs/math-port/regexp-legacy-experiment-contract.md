# P20b root-frozen legacy canonicalization experiment

This continues the pinned ignored regress 0.12.0 feasibility experiment, not
engine adoption. Source revision 771e6c8fa1e472ef86c45b5206b37fa4e4494e25.
Root retains the previously reviewed general parser fixes. The original clone,
prior experiment, production code, flags, expectations and manifest remain
unchanged. No special cases for token/pattern/character spellings are permitted.

Root owns this contract, ignored-copy setup, library module declaration,
constant data, real-Node expectations, replay, source review and result report.
Luna tex_parser owns ONLY the new ignored copy's src/unicode.rs and src/parse.rs,
and docs/math-port/evidence/p20b-legacy-casefold-implementation.md. No other file,
build, manifest, data, oracle, commit or delegation is authorized by this packet.

Root adds a private legacy_case module. Its immutable MAPPINGS slice contains
every nonidentity BMP UTF16 unit mapping obtained from the pinned Node 24.13.0
Unicode 16/ICU77.1 full uppercase data: if full uppercase UTF16 length differs
from one, retain the source; if source is non-ASCII and that unit is ASCII,
retain the source. All other one-unit mappings are included. 1166 mapped units,
102 full expansions and two ASCII restrictions. canonicalize(unit) binary
searches this complete table, identity otherwise. Targets are verified fixed
points. No RegExp result contributes to the constant data generator.

Implement the exact legacy equivalence relation consistently:

- unicode::fold_code_point(nonunicode) uses legacy_case::canonicalize.
- unfold_uppercase_char computes that same target, includes the input/target,
  and adds every table source with that target. Sort/deduplicate as before.
- Add unicode::add_legacy_icase_code_points(CodePointSet): retain original set,
  form its canonical image, then add all mapping preimages of that image.
  Identity elements, wide ranges and inverted/set consumers remain intact.
  The finite nonidentity table permits exact closure without iterating a
  million codepoints or recognizing example literals. No runtime JS/evaluator.
- make_bracket_class receives the u-or-v mode bit and chooses the existing
  Unicode fold closure or new legacy closure BEFORE optional negation.
- Existing legacy bracket closing chooses the same closure by mode. Its
  result.invert remains unchanged. Literal expansion/runtime backreferences
  must share canonicalize, rather than invent another uppercase path.
- Unicode/v ClassSet, property-escape/complement behavior and existing Unicode
  fold algorithms/data remain unchanged. This packet does not repair Unicode
  version/property data, /v semantics, stateful APIs or budgets.

UTF16/UCS2 input and flag adapter remain the existing experimental harness.
Their outstanding gaps remain reported; correcting legacy matching must not
erase unrelated differences or narrow arbitrary configurable regex syntax.

Root freezes independent real-RegExp witnesses before replay, covering full
uppercase expansions, both directions, literals/singleton/range/negated/mixed
classes, word classes/boundaries, captures/backreferences, Unicode controls,
optimizer predicates and UTF16 starts. Root replays the unchanged 119218-case
corpus and compares regressions against the previous parser-only experiment.
Any difference remains a failure for final engine adoption. Candidate upstream
tests must pass, and existing Source/production artifacts remain unchanged.
