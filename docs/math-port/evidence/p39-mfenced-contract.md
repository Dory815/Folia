# P39 mfenced restoration contract

Root freezes ownership against MathJax 4.1.3 mfenced.ts, SHA-256
64b3f4d89d47765eb83034b480b843b2769d8de12e31d7571c50e5c4bb2bd397.
This supplements the historical dense P16c contract for current tagged arrays.
It is not implementation acceptance. Mermaid remains paused.

Root owns existing declarations/class fields/statics, local primitive helpers,
Source observers, mutation gates, review and acceptance. `mml_verify_nodes` owns
only the four semantic stubs in existing mfenced.cpp: setTeXclass,
setChildInheritedAttributes, addFakeNodes and fakeNode. Agents may not change
helpers/interfaces/other files/gold or build/test. Root explicitly releases edits
only after real-stub evidence and primitive bindings are complete. Use LF.

Retain Source live reads and callback order. Class assignment gets previous class,
then open and slot 0, captures loop length after those calls, reads each separator
and real child live, then close and fallback update. Short-circuit reads and last
callback return are observable. Required member calls retain tagged failures.

Inheritance first calls virtual addFakeNodes, then captures the concat result of
current open/close/separators. Iterate this snapshot, skipping nullish values,
then invoke the qualified base hook on current real children with raw arguments.
Null separators concat as one null element and are skipped; they must not throw
merely on snapshot creation. Null separators indexed access/push still throws.
No fake node is appended to real childNodes.

Fake-node creation gets all three raw attributes in one ordered getList, invokes
string replace without coercion, and removes only space/tab/LF/CR code units.
Create open first. Padding rereads current real length and uses the last UTF-16
code unit. Capture sparse slice(1) after padding; iterate its slots, consuming a
separator character only for a truthy child. Capture separators push receiver
before virtual fakeNode argument evaluation; push its returned value into that
retained array even if the callback rebinds the field. Repeated calls append;
throws retain previous effects. Missing fakeNode arguments alone receive Source
defaults. Capture nullable text setter result as the sole mo child; preserve
factory argument count/carriers, virtual class setter then raw parent setter.

Root binds tagged childAt (including exact null-array index errors), UTF-16 string
units/whitespace, and the native TextNode setter capability. No dummy node or
fallback feature is permitted. Guards use the existing Native resource boundary.
Numeric-slot prototypes are retained; arbitrary JS object reflection is outside
the declared typed native host and is not a whole-source parity claim.

The unchanged original oracle currently reproduces 225 cases / 984 assertions /
509 records. Root supplements actual original observations for exact errors and
known concat/iterator/nullable-setter gaps before Native comparison. Real stubs
must compile and fail; final implementations must pass unchanged gold, independent
compiling semantic faults, the normal build and affected regressions. This is the
remaining explicit MML consumer packet, not the complete TeX/layout/SVG renderer.
