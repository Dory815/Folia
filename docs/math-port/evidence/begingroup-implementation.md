# Begingroup implementation record

The owned native modules are `begingroup.stack`, `begingroup.methods`, and
`begingroup.configuration`, each importing the canonical `folia.math.input.tex.parser`
module. The independent CJS oracle pins all three upstream TypeScript and
installed CJS hashes and records 13 cases in
`tools/math-port/begingroup-reference/begingroup-source.json`.

| Source owner | Native owner | Ported responsibilities |
| --- | --- | --- |
| `BegingroupStack.ts:104-237` | `stack.cpp` | Class allocation and Context identity; handler references; global map capture; `checkGlobal`; group push/pop; finish/remove/reset/sandbox; ParseOptions lookup. |
| `BegingroupMethods.ts:37-105` | `methods.cpp` | Canonical parse-method table and `Macro` identity; begingroup/endgroup/reset/sandbox/global callbacks; actual allowGlobal includes dispatch; source command-map registration. |
| `BegingroupConfiguration.ts:33-73` | `configuration.cpp` | Configuration record, handlers/options, config callback creating the real stack, preprocess/postprocess callbacks, and the separately evaluated command map. |

The original corpus covers class/method metadata, ordinary parsing, nested
local shadowing, global `def` and `newcommand`, forbidden global use, local
delimiter restoration, global delimiter definition, unmatched endgroup,
reset, sandbox, retry cleanup, and repeated expressions sharing one stack.
Package setup deliberately lists `base`, `begingroup`, then `newcommand`:
the pinned `ParserConfiguration` reverses this list, so Newcommand's maps exist
before BegingroupStack captures its global map references.
The constructor descriptor has source arity one. Its prototype own method
order is `getGlobal`, `checkGlobal`, `push`, `pop`, `finish`, `remove`,
`reset`, `sandbox`; the native initializer installs that order and preserves
`checkGlobal`'s source arity of two.

Observed Source outcomes include local nested shadow expansion in the order
`y`, `x`, `z`; global `def` and `newcommand` expansions surviving the closing
group; an `IllegalGlobal` MML error for a disallowed global declaration; an
undefined-command MML error after local delimiter restoration or group reset;
and a `MissingBegingroup` MML error for an unmatched endgroup. The unmatched
group does not lower the stack below its base. Sandbox sets the base to
`-101`; the retry-open-group row leaves the group at `-101` until the next
preprocessor cleanup. Repeated formulas observe stack levels `-101`, `-101`,
then `-100` on endgroup.

The typed `GlobalDefinitionScope::checkGlobal(PropertyArray, PropertyArray)`
path uses the actual Newcommand service, shared TokenMaps and marker Symbol.
Direct Context invocation of `stack.checkGlobal` remains explicitly
unimplemented: the current runtime has no canonical TokenMap Context class
identity/view. No copied TokenMap facade is installed. Context-backed arrays
can pass through the existing live `ContextPropertyBindings` bridge; this does
not provide the missing TokenMap identities or their virtual `lookup/add/remove`
surface. The package must remain unaccepted until Root runs the native
Begingroup candidate through the integrated build and resolves or records that
reflection boundary.
