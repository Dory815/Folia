# Shared checkpoints 28 through 32

Root uses one existing Ninja cache, freezes the registered/generated inputs
before each build, and releases the edit window after linking. Gates 28 through
32 each compared 814 input hashes with no changes during compilation. Ignored
build logs and snapshots remain under `build/math-port` and `build/*.log`.

Gate28 built all registered diagnostic targets and FoliaTests, exit 0. Its
ordinary core runner passed 345,517 assertions in 5,291 tests across 168 suites.
ParserTests passed 822 assertions in 58 tests; Begingroup passed 43 assertions
in four tests. Base matched all 37 original observations. Frontend matched 32/37:
five ignore-variant attributes were missing. Enabling the actual package
configuration list exposed a ParseOptions filter-payload mismatch in Bold/AMS.

Gate29 failed before linking on a declaration-order error. No fresh program
results are attributed to it. Gate30 built all targets, exit 0. Its original
comparisons matched Base37/37, Frontend37/37, Bold10/16, AMS23/24 and Boundary33/36.
Its ordinary core process ended with exit 1 and an empty log; assertion counts
are unavailable. Parser/Begin exceptions exposed cross-Context constructor error
records retained through static state. They were not accepted as Source behavior.

Gate31 failed before linking on an incomplete Property ownership-pin edit.
That partial edit was removed by its owner. No fresh program results are
attributed to this gate, and retained public Property lifetime remains open.

Gate32 build ended with exit 1: the library, FoliaTests and 19 diagnostics linked,
but the Boundary candidate failed to compile because a local owner name resolved
to a namespace. Its later correction has not yet been built. Do not attribute
the old Boundary executable to Gate32.

The actual Gate32 FoliaTests process completed with exit 0: 345,517 assertions,
5,291 tests, 168 suites. Fresh frontend original comparisons match 37/37; the
full Bold/ConfigMacros emitted records match 16/16. AMS matches 23/24, with the
tag-reference root math missing display=block. ParserTests still fails three
BaseItems constructor/reflection/error-spread tests (four assertions); its other
suites pass. BoldTests fails one mtext/unmapped-variant expectation despite the
16-row original comparison passing. Begingroup passes 43 assertions/four tests.
These diagnostic failures remain assigned to their owners for Source-based fixes.

The Context error fix allocates actual constructor error records per runtime
module and reads live constructor identities. Static error tables serve only as
initialization templates; they must not remain a second mutable authority.
Remaining reflection failures prevent complete BaseItems class acceptance.

The parser checkpoint runner preserves real program CLI contracts and compares
every emitted case/declared field against independent original corpora. None of
these counts establishes coverage of all 38 enabled packages or output layout.
Native Lite/Common/wrapper/scene/SVG packets remain unregistered or incomplete;
Folia continues to use its existing math rendering path.
