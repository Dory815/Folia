# Gates178–180: Factory and Lite integration, initialization regression

Gate178 integrates the generic Factory Context module with real base owners,
StackItem live `_nodes`/`_properties` and current string-conversion semantics,
MML `toString`, and Lite canonical descriptor projection. Lite custom fields
move once into the canonical Context object; subsequent reads, writes,
deletion and key enumeration share that owner. Existing parent fields follow
Source insertion order. The lifetime fixture prepares UTF-16 before its stop
thread and always executes recovery calls.

Gate178 fails compilation at StackItem conversion/helper interfaces and a
Factory test's exception-name access. Gate179 compiles the production library
but fails the new StackItem test TU at an invalid module import, Value versus
ObjectRef arguments, and a missing getter fixture. Corrections keep the test
assertions and add the actual getter being counted. Both gates close with all
2,096 captured inputs unchanged. No runtime acceptance is claimed for either.

Gate180 compiles all 59 targets. Its manifest SHA-256 is
`d3d4e457a537f87ba62c35df244255575f20ae82aaf786851b98b4f44e389977`.
All 2,096 inputs remain unchanged through the completed evaluation.
Public API initialization fails with
`Generic Factory Context bindings must be initialized before Common constructors`.
The public SVG and paired Scene captures also fail. There is no accepted
complete SVG or Scene comparison for this gate; Gate177 remains the last
passing full SVG/error comparison (150/150).

The actual service graph registers Common constructors before the SVG class
registration calls `bindFactoryMethods`. The new generic binder requires
those real capabilities earlier. The fix must install that same binder before
constructor registration and preserve its identities when SVG registration
later supplies the same runtime. It must not substitute a fallback owner.

Core, Host, Parser and Foundation fail. Regexp and LinkedList pass. Five of
25 package captures succeed; one of 20 checkpoint comparisons passes. Besides
the initialization error, focused Factory tests have unresolved boolean
failures and a released-runtime exception-contract difference; these need
separate diagnosis after initialization works.

Both additional Renderer diagnostic drivers leave uncaught initialization
exceptions and display Microsoft Visual C++ Runtime Library windows. Root
verifies their exact workspace executable paths and terminates those two
test processes (PIDs 43444 and 40840). Their captured exit codes are
4294967295, not passing diagnostics or normal test completions. A separate
record preserves why they were terminated. After evaluation closure, both
drivers gain top-level exception reporting to stderr and return 1; that
correction still needs the next build.

The complete Gate180 evaluation closes with exit 1 and zero post-evaluation
input changes. Folia still uses math QuickJS; Mermaid remains paused. The
full Tree/MML constructor and owner-retirement packets remain unintegrated
candidates and are not covered by this build.
