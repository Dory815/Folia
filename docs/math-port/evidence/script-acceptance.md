# Root audit of P10 script nodes and string utilities

Status: local root acceptance passed under the explicit typed host scope. This packet covers
all members of msubsup.ts, munderover.ts and util/string.ts: six concrete MML
classes, eight string helper functions, 43 inventoried members and 35 executable
members. p10-members.json contains AST-discovered member hashes and native
targets. p10-source-lock.json independently pins clone TS and resolved CJS.

Root froze interfaces, per-file ownership, constructor static bindings and
1,270 initial actual-CJS assertions in commit 5240599 before dispatch. Agents
could write only their assigned implementation and ledger; root alone owns
source review, shared services, expectations, builds, mutations and acceptance.
Each node remains a concrete subclass of the accepted MML hierarchy. The
complete production factory, mo, parser and renderer remain unfinished.

## Reviewed semantics and corrections

msubsup captures a child-array handle with live slots, evaluates receiver and
arguments in source order, keeps attribute record identity, performs primitive
level+1 and preserves truthy raw prime while skipping virtual sub getters.
Msub/Msup inherit that hook and retain their distinct virtual indices/arity.

munderover performs base inheritance before force calculation, then reads
ACCENTS from its captured actual constructor independently of current registry.
Munder inherits its parent's shared static binding; Mover owns the reversed
array. Replacement does not retarget earlier captures; in-place writes stay
observable, and slot names are reread after child callbacks. getScriptlevel uses
numeric increment only on its source branch, preserving raw levels otherwise.
setInheritedAccent reads current children and separate current attribute getters,
tests explicit presence before dereferencing null children, preserves NaN strict
inequality and reinherits with fresh empty records/original display and prime.

Root found and corrected an eager over() argument evaluation in the initial
agent body, which defeated truthy-prime short circuiting. A virtual getter that
changes the base slot now verifies both short circuit and receiver capture.
Root also required source Number.toString for finite RangeError suffixes; native
to_chars general formatting was insufficient near JS decimal/exponent cutoffs.

String helpers operate in UTF-16, preserving lone surrogates and code-point
iteration. All whitespace operations reuse the accepted ECMA table. Regex
quoting, empty trim/split result, bare/braced Unicode escape prefix matching,
generic escape errors, unmatched line terminators, non-rescanned replacements
and first-code-point uppercase entities follow the source, including error text.
unicodeString's typed numeric-span binding exposes invalid doubles but does not
expose arbitrary JS iterators/getters, proxies or VM argument-stack capacity.

## Validation and harness corrections

The final current fixtures have 1,683 actual-CJS assertions in 17 cases, plus
six native assertions exercising real 512-deep hook overflow and subsequent
recovery: 1,689 assertions in 18 cases. Focused and full core suites pass.
Initial fixture compilation exposed root generator errors in exponent literal
emission, string/record comparison bindings and namespace selection; those
were fixed in the generator, without deriving expectations from native output.

Deep Windows Debug hook calls exhausted the default process stack before the
512-call guard. The test process and isolated mutation executor now reserve
16 MiB; real nested hooks reach the shared guard and recover. The native module
does not configure caller stacks. G7 requires equivalent math worker stack
configuration and validation before Folia can switch. These tests do not prove
the current app/thread integration and are not injected counter simulations.

An initial array-retarget mutation exposed unsafe trace indexing in the root
test harness when the deliberate defect removed expected callbacks. Root added
shape checks before reading trace slots, so structural failures remain assertion
failures and never cause test-only undefined behavior. The mutation then compiled
and failed assertions as required. The final complete isolated run detected all
28 deliberate regressions, each compiling and exiting 1 through failed tests.
No crash or timeout counts as detection. Working sources and fixture bytes were
checked unchanged at completion. Results/logs remain ignored under
build/math-port-script-mutations; script-mutation-test.cjs reproduces the run.

## Scope remains open

The operator dependency audit separately records ordinary-object lookup,
definition aliases, range-return identity and LinebreakVisitor startup mutation.
It is not acceptance of OperatorDictionary or LinebreakVisitor implementations.
No complete TeX parser, font service, Common/scene layout, SVG backend or Folia
replacement follows from this packet. Full G0–G7 remain closed.
Twenty-four of 246 inventoried source files now have local acceptance;
the other 222 and external dependencies still require review and implementation.
