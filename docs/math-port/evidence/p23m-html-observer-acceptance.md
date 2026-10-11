# Root HtmlNode observer acceptance

Root reviewed and accepts only the P23m test-binding repair: the children factory
now constructs ChildArray/NodeValue entries, and nine native parent writes call
setParent. Original JS parent mutations, XML services, virtual callbacks, host
aliasing and expected assertions remain unchanged.

The original oracle and the Root frozen profile both reproduce: 434 assertions,
75 cases, 52 expected-value declarations and 570 ordered profile records. Profile
SHA-256 remains `c61f83c6717ec204dff4514315d0c7657a5eb1cc09b086ad4e611c8d743380b9`.
Root separately reviewed unchanged direct UTF-16/copy expectations.

`node tools/math-port/html-observer-compile.cjs` compiles this observer against
the full implementation graph and all 90 module interfaces. It executes no
semantic tests. Its `compile-result.json` SHA-256 is
`9d3aeacaf088134b8fec70ba29d87650bc944592ad3dbfe0eef0ae650029dd02`;
its successful baseline build log SHA-256 is
`be848d1e7ca0fdfef69632d60e6c9770336d10514132d41f809c6d9a8caef0b4`.

The subsequent `cmd /c build_test.bat` remains a failed repository gate before
test execution. HtmlNode no longer fails compilation; remaining observer
bindings must be repaired separately. This packet does not certify sparse
algorithm runtime equivalence, increase the 54/246 historical whole-file tally,
or accept Math/Folia integration.
