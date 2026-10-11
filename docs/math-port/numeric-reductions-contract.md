# P20c complete util/numeric.ts packet

Source MathJax 4.1.3 util/numeric.ts SHA256
a2004a9888f5c85f3cf81126e43d04dfe66b64c55e3b5f7d97366631809d7c15.
Both exported functions sum and max are required. Root owns architecture,
declarations, Source corpus, native tests, manifests, mutation/review/acceptance.
Luna math_layout owns ONLY src/math/util/numeric.cpp and
docs/math-port/evidence/p20c-numeric-reductions-implementation.md.

The native input is a borrowed readonly dense binary64 span. Source's only
importer in the 246-file closure is output/common/Wrappers/mtable.ts (SHA256
a1e060baab84d7b842c811c560ab73f32ca91614e3a5b02f7f1712db73b0100c).
Its call sites use numeric table extents/row cell counts/line-spacing arrays,
concat/map-produced dense arrays, and cWidths converted from string-or-number
to numbers before sum (lines 1210–1223). Numeric H/D arrays initialize with
fill(0) (786–787). The future Common wrapper producer must retain this exact
typed data responsibility; no percent string is silently treated as zero.
Arbitrary JS Array methods/accessors/prototype mutation are outside raw-TeX
native input; this is the explicit native number-array adaptation, not a JS VM.

sum is the Source left fold from positive zero, reading each value in order
with ordinary binary64 addition. Preserve overflow, NaN, signed-zero and
rounding/order. No compensated summation, pairwise/reordered reduction or
float narrowing. max is the Source left Math.max fold from positive zero:
empty/all-negative produce positive zero; a NaN anywhere propagates; infinities
and signed-zero ordering follow JavaScript. std::max alone does not propagate
NaN. No artificial finite-value guards or tolerance-based answers.

C++23 named module/import std; no platform dependencies. Root independently
freezes actual CJS numeric results and exact binary64 input/output records,
with NaN compared as NaN, not requiring payload identity. Include empty,
negative, zeros, subnormal/overflow/infinity, cancellation and seeded mixed
values. Native suite plus compiling assertion-failing mutants and full core
must pass before accepting this Source file. This packet accepts no Common
table layout, regex engine, SVG, native renderer or Folia integration.
