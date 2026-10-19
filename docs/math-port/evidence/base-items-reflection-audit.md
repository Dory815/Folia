# BaseItems pinned reflection audit

This additive audit reads the pinned MathJax 4.1.3 CommonJS runtime. It does not modify the frozen `base-items-original.json` corpus. The source hashes checked by `tools/math-port/base-items-reflection-audit.cjs` are TS `0c0d8448f3577be6642c95a412682310362819d2d383ab83b817d2888e4e5f0c` and CJS `5310d0429b7b62d405ae9b93c8d21ad17b787e5eb3fd1f01439c62f2094f7d1a`.

Run with `node tools/math-port/base-items-reflection-audit.cjs`. It instantiates all 26 exported classes through the real `StackItemFactory` and records constructor arity, prototype descriptor/member chain, instance own keys and own-field value shapes, class error ownership, and Source kind/open/close/final/copyEnv values. The ignored result is `build/math-port/base-items-reflection-audit.json`.

| Source class | CJS constructor arity | kind | Own subclass fields | Own prototype members |
|---|---:|---|---|---|| StartItem | 2 | start | — | kind, isOpen, checkItem |
| StopItem | 0 | stop | — | kind, isClose |
| OpenItem | 0 | open | — | kind, isOpen, checkItem |
| CloseItem | 0 | close | — | kind, isClose |
| NullItem | 0 | null | — | kind |
| PrimeItem | 0 | prime | — | kind, checkItem |
| SubsupItem | 0 | subsup | — | kind, checkItem |
| OverItem | 1 | over | — | kind, isClose, checkItem, toString |
| LeftItem | 2 | left | — | kind, isOpen, checkItem |
| Middle | 3 | middle | — | kind, isClose |
| RightItem | 3 | right | — | kind, isClose |
| BreakItem | 3 | break | — | kind, checkItem |
| BeginItem | 0 | begin | — | kind, isOpen, checkItem |
| EndItem | 0 | end | — | kind, isClose |
| StyleItem | 0 | style | — | kind, checkItem |
| PositionItem | 0 | position | — | kind, checkItem |
| CellItem | 0 | cell | — | kind, isClose |
| MmlItem | 0 | mml | — | isFinal, kind |
| FnItem | 0 | fn | — | kind, checkItem |
| NotItem | 0 | not | remap | kind, checkItem |
| NonscriptItem | 0 | nonscript | — | kind, checkItem |
| DotsItem | 0 | dots | — | kind, checkItem |
| ArrayItem | 0 | array | table, row, frame, hfill, arraydef, cstart, cend, cextra, atEnd, ralign, breakAlign, templateSubs | kind, isOpen, copyEnv, checkItem, createMml, handleFrame, StartEntry, getEntry, EndEntry, EndRow, EndTable, checkLines, addRowSpacing |
| EqnArrayItem | 1 | eqnarray | table, row, frame, hfill, arraydef, cstart, cend, cextra, atEnd, ralign, breakAlign, templateSubs, maxrow | kind, EndEntry, EndRow, EndTable, extendArray, addIndentshift |
| MstyleItem | 3 | mstyle | attrList | kind, checkItem |
| EquationItem | 1 | equation | — | kind, isOpen, checkItem |
## Native evidence status

`base-items-native-candidate.cpp::observeRegisteredItems` constructs each of the same 26 classes through the actual `ParseOptions.itemFactory`, but records only kind/flags/size/env/global/name, identity, constructor length, prototype kind getter, and own-errors status. `base-items-constructor-identity-native-tests.cpp` checks per-instance ArrayItem storage freshness and MstyleItem's supplied attribute identity. These are partial field/reflection comparisons, not a strict comparison of the full CJS own-field table above.

The frozen original corpus contains 37 real parser observations and 37 direct Source observations. The native direct-method fixtures exercise constructor arguments, reductions, arrays, Prime/Subsup/Over, Position/Style, Not, and error ordering. Gate27's strict parser comparison matched all 37 rows; the earlier `array-row-controls` mismatch was traced to HFill storing stack depth instead of `ArrayItem::Size()` and fixed in `BaseMethods::HFill`.

Remaining reflection gap: there is no full native emitted dump compared field-by-field with the CJS constructor/prototype/instance records, including descriptor flags and all own storage identities. The list above is actual Source execution; do not interpret the native subset as full reflection parity.
