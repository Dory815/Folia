# Root template inheritance acceptance

Accepted exactly AbstractMmlNode::inheritAttributesFrom and the unconditional
AbstractMmlEmptyNode override. The Root-frozen get/isSet virtual slots preserve
the existing Attributes method bodies; the two consumer bodies were implemented
by separate Luna agents under the pre-implementation P29 contract. Root reviewed
their diffs and independently generated all acceptance evidence.

All 9,344 actual original complete records pass, including 104 original errors,
raw value identities, fresh bags/pairs across repeated calls, retained/rebound
attribute objects, exact virtual read order and effects preceding exceptions.
Source JSON SHA256 remains
4fbceb2e3fda0716ac4ca72bb772613327f1376624c1fbd90d1085031a2fc20c.

Thirteen compiling faults are effective: base no-op; repeated attributes read;
reversed display/level reads; unconditional mathsize; bypassed isSet/get virtual
slots; boolean-coerced prime; preserved falsey prime; premature prime read;
wrong pair origin; shared default bag; bypassed receiver inheritance setter;
and Empty inspecting its argument. All fail tests with exit 1, with final
working implementation/fixture byte guards passing. No original expectation
was changed to make a Native result pass.

The inheritance regression gate passes 1,765 assertions in 17 tests and its
prime-type fault. Original primitive --check reproduces 1,223 checks. The actual
normal core runner filtered to inherit_from rebuilds and passes 9,344 assertions
in one test. A separate attributes_reference run passes 445 assertions in the
single executed Attributes test; its suite reports three registered tests with
two skipped. That count must not be reported as three executed tests.

Ignored evidence SHA256:

| Evidence | SHA256 |
|---|---|
| build/math-port-mml-inherit-from/results.json | 7d0e0b97e19fc32060a20615bb758314d322a2beb03f101c8cbf646654cbd404 |
| build/p29-inherit-from-core.log | 975af535ece1ced7a5d8c956d90269c4a99443275ef7eea0fda2a3863e95c010 |
| build/p29-attributes-core.log | 286c7477d9ed61d2f5594640a7295e92836a25efcd66000fac3908607175804b |

The previous full core runtime is still unaccepted. Remaining MML verification,
replacement/error construction and other consumers, full TeX/parser/layout,
SVG/Direct2D and Folia remain required. Neither a whole-file tally nor the
historical 54/246 figure advances from these two method bodies.
