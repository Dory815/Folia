# P19b2 root local acceptance — shared native callable and receivers

Root accepts the bounded native host protocol in
`../callable-receiver-contract.md`. This accepts no TokenMap/MapHandler member,
regular-expression engine, complete TeX parser, Common layout, SVG output or
Folia switch. The complete source count remains **51/246**.

Root froze object/callable/TeX declarations and implementation responsibilities
before assigning Luna work. math_layout changed only FunctionList's two
invocation sites; tex_parser implemented only parse_callable.cpp. Root reviewed
the complete implementation, fixed the active callback storage boundary,
and owns all Source probes, native stimuli, manifests, mutations and acceptance.

One Function identity/body serves ParseMethod and actual FunctionList items.
NativeObject carries identity and lifetime only; priority records derive from
that marker without replacing their accepted fields, storage or iteration.
FunctionBody retains one shared std::function. Capturing a body handle keeps
the active closure alive when a callback replaces its own field and retains
the same mutable closure state across calls. There is no second callback path.
The typed bridge validates receiver and native slots, passes the borrowed
ParserContext/atom/rest, flattens the active ParseResult alternative into any,
and restores accepted results or Source undefined. Property false remains
visible to FunctionList. Invalid native protocols use the existing TypeError;
original callback exceptions and shared result/argument identities propagate.

`callable-receiver-oracle.cjs` executes actual hash-pinned MathJax 4.1.3 CJS
TokenMap, MapHandler and FunctionList. Eight Source probes provide **50
observations**: strict undefined receivers for Character/Macro/Command calls;
EnvironmentMap member receiver, original function/argument identities and
replacement parser; SubHandler's actual first fallback record, shared macro
function identity, live item/priority mutation and removal; exact false through
FunctionList; raw thrown-reason identity; and continuous closure state.
The generated native suite passes **79 assertions in nine tests**, including
eight vector-size assertions and 21 native lifetime/protocol/result witnesses.
Native MapReceiver is a marker used to exercise the accepted call seam. Native
tests supply observed call shapes; they do not implement or accept map dispatch.

The closure witness caught a real boundary error before acceptance: copying
std::function before each invocation cloned mutable capture state. The isolated
baseline reported 78 assertions, with the two second-call assertions failing.
Root replaced callback-value copying with shared FunctionBody handles. The
actual-CJS closure probe and a compiling closure-copy mutant now guard this.

All **12** new isolated mutants compile and exit 1 through failed assertions:
record receiver erased/copied; typed receiver erased; null receiver accepted;
typed false boxed; result erased; error category changed; undefined rejected;
raw bool accepted as a typed result; rest rewritten; exceptions swallowed; and
closure state copied. Results are p19b2-callable-receiver-mutations.json.
Crashes, compile failures and empty baselines count as failures.

Existing Source expectations are unchanged. The complete **403** FunctionList
observations and **29** native assertions pass, with all **26** compiling
mutants rerun. The **128** Token/Macro Source assertions and native protocol
witness pass (**161 assertions in 44 tests**), with all **14** mutants rerun.
Primitive fixtures retain **1223 assertions in three tests**, with all **four**
compiling primitive/priority mutants detected. The early isolated primitive
manifest's missing option_types module and stale priority-storage anchor were
corrected; no primitive implementation or expected value changed.
Each refreshed mutation result is recorded beside this acceptance.

Root ran every oracle in --check mode and the four-member FunctionList ledger
check successfully. `cmd /c build_test.bat 2>&1` completes with exit 0 after
the final source changes; ignored log: build/p19b2-final-full-core.log. Focused
callable build/tests also pass: build/p19b2-callable-final-focused.log.
No platform module or application integration changed. Full G0–G7 remain open.
