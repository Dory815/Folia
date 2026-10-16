# Gates197–198: passing core tests and restored Renderer lifetime

Both gates compile all 60 targets. Gate197 preserves 2,126 inputs through
evaluation; Gate198 preserves 2,127. Neither evaluation changes its registered
inputs or replaces a tested executable. Gate198 input manifest SHA-256:
`a8eb088a2ef8aaa0b8a6701f09a57448aa19aca571eba165de2401db60b55928`.

| Check | Gate198 actual result |
| --- | --- |
| Core / Host / Parser / Foundation / Regexp / LinkedList | All pass |
| Complete public SVG/errors | 150/150 match |
| API / Common layout | 9/9 and 75/75 match |
| Paired Scene routes | 146 equal successful Scenes and four equal error pairs; Scene-only materialize/XML calls zero |
| Package captures / checkpoint comparisons | 25/25 complete; 18/20 pass |
| Earlier owner-count / request-lifetime drivers | Both exit 0 |
| Strong mixed owner-retirement driver | Exits 0 |
| Standalone real Renderer stage diagnostic | Five conversions and destructor complete; exit 0 |

Gate197 removes Context access from RegExpBindings destruction. Destruction
invalidates the binding lifetime and releases its local native-field owners;
explicit clear() remains a Context-live operation. The actual Renderer service
graph now tears down normally. Public SVG/API, layout and all three lifetime
drivers pass, but Core retains four failed tests: a getter-only Symbol.split
fixture assignment, two native binding error-class assertions, and an invalid
binding-lifetime fixture that released its last binding owner too early.

Gate198 restores the native resolve() identity-error contract and confines
Source TypeError conversion to unbranded/expired receiver resolution. The
shared matching body runs outside that catch, preserving exceptions raised
by a valid receiver's lastIndex coercion. Focused tests verify the exact native
exception subtype, error name and unchanged raw lastIndex. Three separate
ownership tests cover Context holding the final binding owner, bindings dying
before a live Context, and externally held bindings outliving their Context.
The split fixture defines an own Symbol.split data property instead of
assigning through its getter-only inherited descriptor. Core now exits 0.

The strong Renderer driver exercises 64 mixed array/fraction/Scene iterations,
durable macros and labels, repeated unresolved references and malformed input,
cancellation, and a returned frozen Scene used after Renderer destruction.
Its actual named Lite and parser owners return to the normalized persistent
state; all 24 parser identity families remain stable between equivalent
requests. Source Label placeholders no longer accumulate as native owners.

This result does not establish bounded total Context storage. The owner-count
log reports 4,332 Context object slots warm, 6,217 after durable state setup,
12,457 after 32 iterations and 18,697 after 64. Explicitly retired records still
leave heap shells and slots, and untracked ordinary records can retain graphs.
The withdrawn shell-release candidate is not applied. Safe operation leases
and ordinary request-object ownership remain separate work.

The two failing comparisons are tex-next-six and tex-font-packages. Their
actual Source/native data reports remain unmodified. The MML shared algorithm,
live constructor/factory projection, native Symbol property storage and six
Stack-method packages are still candidates; no passing profile claim substitutes
for their independent review and native execution. The 150 public records are
75 inputs repeated in continuous and fresh-session groups, not 150 independent
feature cases. Selected text-mode BBoldx SVG coverage is being added separately.

Evidence is under `build/math-port/`, including `diagnostic-exits-198-root.json`,
the Foundation provenance, `checkpoint-comparisons-g198.json`, each lifetime
driver's log/provenance, `renderer-stage-g198.log.stderr`, and the paired Scene
route audit. Public SVG comparison SHA-256:
`ac183fbd9cc3b93e36c2fbe03ba57b8fbb11dfff32ebcb0d97f0da81c4ce9877`;
API comparison SHA-256:
`22ce4ade41daf0b24b65c4454b6c34a149e0cfb19abcfc328a2d71fa7b75d6ad`.
The aggregate evaluation remains failed because two package comparisons fail.
Folia still uses its math QuickJS worker. Mermaid remains paused.
