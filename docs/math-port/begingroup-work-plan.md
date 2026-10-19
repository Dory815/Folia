# Begingroup package ownership and acceptance

This is a required package in the unchanged Folia TeX profile and supplies
Newcommand's real global-definition service. Root owns shared contracts,
CMake, builds and acceptance; the assigned Luna owns the complete package.

Port the pinned `.reference/MathJax-src/ts/input/tex/begingroup` files separately:
`BegingroupStack.ts` to `src/math/input/tex/begingroup/stack.ixx/.cpp`,
`BegingroupMethods.ts` to `methods.ixx/.cpp`, and `BegingroupConfiguration.ts`
to `configuration.ixx/.cpp`. Use standalone modules importing the actual parser
and required Newcommand utility constants. Keep original method, class-field,
map, priority, callback and initialization responsibilities in these files.
Preserve every member and pin cloned TS and installed CJS hashes in a ledger.

The actual `BegingroupStack` implements the internal `GlobalDefinitionScope`
native protocol exported by parser:types. Its `checkGlobal(PropertyArray
tokens, PropertyArray maps)` consumes the same live array handles and returns
the actual `TokenMapRef` values. This replaces Source dynamic method dispatch
at the C++ boundary, not the Source stack or its algorithms. Newcommand remains
independent of the concrete package; it resolves the actual object through
`ParserRuntime::globalDefinitionScopeIdentities`. Configuration stores that
same object's Context identity in the real packageData `begingroup.stack`.
Use `ParserRuntime::begingroupMethods` for the one canonical method table.
Source `MARKER` is a fresh actual Context Symbol per stack, retained as
`Property(options::SymbolRef)` in the existing TokenLookup. Do not substitute
a string, fake token or object sentinel. Root is validating that Symbol bridge.

Preserve Source group insertion/removal, global map aliases, priorities and
top/base state, input tokens.shift mutation, marker insertion/removal order,
reset/sandbox behavior, allowed global commands, retry preprocessing cleanup,
finish behavior, and partial mutations on malformed groups or callbacks.
Use the actual TokenMap/MapHandler and parser services. A marker or global
implementation requiring an unsupported shared operation must identify that
operation; approximations and swallowed errors do not pass.

Coordinate directly with the Newcommand owner on the shared service signature.
Expose `BegingroupConfiguration::initializeModule(ParserRuntime&)`. Root will
register it in the loaded original package graph without changing package
selection. Owned tools, native fixtures and evidence use `begingroup-`.

Provide frozen actual CJS original observations, runnable actual native
fixtures and strict comparisons of complete MML, macro/map identities and
state. Exercise nested/local/global definitions, local shadow removal,
delimiter/environment definitions, protected commands, unmatched endgroup,
reset, sandbox, retries and repeated formulas using one real runtime.
Root compiles and runs native candidates and checks meaningful semantic
faults. The package stays unaccepted until that gate passes; remaining
packages, common layout, SVG and Folia integration retain their full scope.
