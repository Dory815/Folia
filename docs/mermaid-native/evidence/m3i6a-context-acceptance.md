# Root M3i6a shared context ownership acceptance

Accepted the ten mechanical production changes listed in
m3i6a-context-consolidation-contract.md. The owner is PipelineContext;
GradientProgram retains returned/stages/context. Common sampling stages consume
that owner directly. PatternContext borrows it immutably alongside the original
pixel borrow; ResourceContext borrows it mutably alongside destination/mask/AA
run resources. Root's field/call-site review finds no retained old persistent
fields, copied context adapter, old signature fallback or source-pixel copy.
The sampling register comment was corrected by Root after all guarded runs.

Root migrated only four native observer transports to the new signatures.
All existing expected values and numeric assertion loops remain unchanged;
`migrate-context-observers.cjs --check` reproduces those transports. The independent
default observer still executes actual original Context::default, all fifteen
groups, SHA `d02ab0b72988be7722e914495990e20e2d80b9f17e2dda8a2c4ca77e1f03fe28`.

The new shared-write witness uses **48 existing original observations** (16 highp
conical mask writes and 16 lowp/16 highp uniform reads) through one live owner,
followed by borrowed pattern/resource views. It checks owner identity and visible
state with **226 assertions**; the separate Source default test has 16 assertions.
Root initially selected sampling capture index one, but original sampling records
have exactly one capture per stage and singleton cases use index zero. That
transport failure was rejected and corrected without editing Source or production.

The five affected targets pass all original comparison suites: gradient programs
13988, colors1296, powers8010; sampling5733 high/945 low and unsupported672;
resources2959 plus terminal418 and unsupported685; pattern7056/pixmap1694 plus
unsupported459; and the two context tests. **Fourteen compiling semantic mutations**
are all detected by assertion failures: corrupted Source default, lost producer
transform/radial writes, high/low default transform readers, lost conical mask
writes/default mask reads, high/low default uniform/coverage reads, and default
pattern X/Y limits/sampler readers. Original source, tests and golds are byte
guarded. Results: `build/mermaid-context-ownership-mutations/results.json`, SHA
`54b4ae9cf84cd712f079cf32d0f23a803fa68b02528389993eaa1d078cf99919`.

The four gradient and two sampling mutation anchors affected by the public API
move were adapted to the new owner. Each of those six existing compiling mutations
was rerun and detected. Other historical mutants are not claimed as rerun.
Actual Source generators reproduce unchanged default, gradient, sampling,
resource, terminal, blend, pattern, tall-pattern and pixmap golds.

The full explicit native matrix passes **20 targets / 61 tests**, including eleven
actual Direct2D WARP tests. It runs locked/offline, without default features, with
direct2d and pinned Arial at C:/Windows/Fonts/arial.ttf. Log:
`build/m3i6a-final-native-matrix.log`, SHA
`040dadedb8653f8f91fbbae91b5bf9c1a7fe18693c35a85bd1b2b1393fea9e45`.

This is an ownership consolidation, not a new algorithm red port. Rendering
function acceptance remains **34/70**. Pattern producer, full executor, the three
separate blitter builder/context paths, complete shared scene paint/text and final
SVG/Direct2D comparisons remain open. Folia has not been connected to this work;
the user's full acceptance-before-integration condition still applies.
