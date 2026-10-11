# P16c `MmlMfenced` method port

The implementation is in `src/math/core/mml_tree/mml_nodes/mfenced.cpp` and
follows the pinned MathJax 4.1.3 source
`.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mfenced.ts` (SHA-256
`64b3f4d89d47765eb83034b480b843b2769d8de12e31d7571c50e5c4bb2bd397`). The
root owns the class declaration, defaults and field initialization. This
packet implements only the four methods assigned in the frozen contract.

`setTeXclass` maps source lines 80–102. It calls `getPrevClass`, then reads and
processes the current open pointer and child zero, captures the then-current
child count, and performs separately-live separator and child reads for each
remaining index. It processes the current close pointer last, then applies the
source `open || childNodes[0] || close` selection for `updateTeXclass` if a
delimiter is missing. A null child list at each source indexed/length access
raises a native TypeError; absent slots and null child pointers are skipped.
Tree callbacks can replace the child list between these reads, while loop
length remains captured after the first child callback.

`setChildInheritedAttributes` maps source lines 111–124. It calls the virtual
`addFakeNodes`, captures a snapshot of open, close, and separator pointers
before the first inheritance callback, visits those nonnull fake nodes with the
original attribute handle and raw scalar properties, then calls the qualified
`AbstractMmlNode` implementation. The base pass therefore observes the live
real-child list after fake-node callbacks. If the separator array handle is
null, the native concat snapshot includes a null entry, matching
`[open, close].concat(null)`.

`addFakeNodes` maps source lines 129–172. It captures the current attributes
object and performs one `getList` for `open`, `close`, and `separators`. Each
value must be a native string because the source directly calls `.replace`;
the C++ method raises TypeError for other Property alternatives rather than
stringifying them. Cleaning is over UTF-16 code units and removes only space,
tab, LF, and CR. All three cleaned values are prepared before factory calls.
Open creation precedes separator work; padding re-reads the current child-list
length per loop test; the `slice(1)` child snapshot follows padding. Null
children do not consume a separator unit. For each separator, the source first
evaluates the `this.separators.push` receiver. The native method captures and
validates that handle before reading the separator character or calling virtual
`fakeNode`, so a null handle throws before factory or callback side effects. It
then appends the result to that same captured handle, even if the callback
rebinds the field. Close creation occurs last. No old fake state is cleared, so
repeated calls retain delimiters and append separators; errors retain earlier
effects.

`fakeNode` maps source lines 180–190. Raw Missing defaults are preserved at
virtual call sites; the base method alone changes Missing properties to a
fresh empty PropertyList and Missing TeX class to null. It creates `text`,
calls the created TextNode's virtual `setText`, passes that returned pointer
as the one `mo` child, and calls the resulting node's virtual TeX-class setter
before assigning its parent. A PropertyList-backed properties value is passed
to the factory as its record view; other raw Property values remain raw in the
factory argument so its binding reports unsupported constructor input. This
retains the constructor/factory boundary without adding a second conversion
policy here.

The implementation uses `std::u16string` to preserve MathJax string length,
`charAt`, and indexing behavior in UTF-16 units, including astral pairs and
lone surrogates. It uses the accepted `tree::TreeCallGuard` native recursion
bound. The only host adaptation is checked TypeError behavior for null native
handles and wrong typed values; it does not add normalization or substitute
fallback nodes.

No build, fixture generation, or tests were run in this assignment, as the
root explicitly reserved those operations and all acceptance decisions.
