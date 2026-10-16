# Gates194–196: constructor fixes, Label roots and a teardown regression

Gate194 integrates the dormant-environment guard, corrects the two BaseItems
fixture lookups, adds a raw named Lite identity diagnostic, and integrates
private Renderer Label-root retirement. It also adds Context String.split /
RegExp splitting and corrects TexParser currentCS initialization plus
TextMacros callback metadata. All changes retain the existing native owners.

Gate194 fails compilation on duplicate local getter names in the new split
test. Gate195 corrects those names and several fixture receiver/argument and
coercion assumptions, then fails on Boost.UT's UTF-16 container printer.
Gate196 makes those comparisons explicitly boolean; all 60 targets compile.
Each gate captures 2,125 unchanged inputs. Gate196 manifest SHA-256:
`d363818ec3201a89fd4b43dceaa2da24bbbad41fabc8ac0cf10bb0240a8da4c5`.

Gate196 runtime evaluation is terminal and failed. Parser, Regexp and
LinkedList pass. Host and public API exit with Windows status 3221225477;
the public SVG capture also exits abnormally and supplies no accepted
150-case comparison. Common layout and paired Scene captures time out, so
there is no accepted layout/route result. Nine of 25 package captures
complete; four of 20 comparisons pass. All three Renderer lifetime drivers
fail. Core stops at a Microsoft Visual C++ runtime-error dialog and is
explicitly terminated after checking its executable path and window title;
the record is `build/math-port/core-runtime-dialog-termination-g196.json`.
Termination is retained as a failed observation, not a passing test.

The supplementary diagnostic `build/math-port/renderer-stage-g196.cpp`
links the actual Gate196 library/IFCs without copying them. Its unbuffered
log shows constructor success and completed x, x+1, fraction, tagged-label
and reference renders, then an abnormal exit after `before destructor`.
The new RegExpBindings destructor calls clear(), which accesses Context
records through NativeIdentityBindings. The actual service graph can be
released by Context-owned callbacks while Context's record storage itself
is being destroyed. The teardown contract needs a correction before another
runtime acceptance claim. Source split witnesses alone do not test this
native owner lifetime.

The Label candidate has passed static independent review, including map and
prototype Label roots, unchanged intrinsic descriptor/prototype guards and
zero Source-getter calls. Its strengthened real-Renderer test normalizes
the same formula before each persistent-state comparison and checks the
named body identity without hard-coded allocation IDs. Gate196's teardown
failure prevents accepting its runtime results. The public NativeRenderSession
does not automatically retire these private Renderer roots.

Separate review also confirms untracked plain Context request objects still
retain graphs. The proposed retired-record heap-shell release is withdrawn:
Context define/erase can hold Record references across reentrant callbacks.
See `tools/math-port/drafts/g194-context-storage-audit/audit.md`. Stable
registry counts do not establish bounded Context storage or total memory.

The full MML candidate now has one shared operation body, but foreign factory
construction and foreign-child/callback lifetime boundaries remain under
implementation and review. Six Stack methods are candidate-only. No Folia
integration is accepted; the application still uses math QuickJS. Mermaid
remains paused.
