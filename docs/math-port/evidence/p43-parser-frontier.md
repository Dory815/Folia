# P43 parser frontier after P42B

This is a Source audit/next boundary, not an implementation release. Root must
freeze exact interfaces and original observations before more Luna body work.
Mermaid remains paused. The native TeX/SVG/Folia goal remains active.

## Stack.ts

Pinned SHA-256 36be7583b14111961367e9e3f1dd5911456d235b096a2fcee62f438d37c7e11a.
Implement the actual Stack class, retaining existing BaseItem/StackItemRef and
MML NodeValue identities. Distinguish the parser stack array from each item's
MML list and existing MML child arrays; no substitute parse graph.

- Constructor makes global={isInner}, creates start with that exact global,
  truthy supplied env replaces start.env, then reads current start.env.
- Push skips falsey inputs, wraps only real MML nodes through factory mml,
  writes item.global before current Top.checkItem, keeps tuple/list identities,
  falsey success skips, truthy replacement performs Pop then recursive Push.
  Null-kind suppresses append only; environment processing still runs. Read
  env/copyEnv/env again in Source order, retaining prior effects on failure.
- Pop removes first, then reads isOpen. Non-open delete env is a no-op for the
  inherited BaseItem accessor; retain backing env. Restore current stack env
  from live Top.env or fresh {}. Errors occur after removal where Source does.
- Top defaults only Missing to 1; too-short returns Null, otherwise ordinary
  numeric array lookup may return Missing for zero/fractional/NaN/negative.
  Prev reads Top once, then virtual First or Pop by truthiness. height is live
  length; toString joins live slots with captured initial length.

Actual Top(n) caller: braket/BraketMethods.ts uses height-env.braketItem. Prev(true)
appears in BaseMethods and TexParser. Freeze constructor/global/env identities,
recursive reduction and callback mutations, current env copies, post-pop errors,
Null/Missing Top edges and live join witnesses, not only simple stack examples.

## Complete base reducer dependency frontier

BaseItems.ts SHA-256 0c0d8448f3577be6642c95a412682310362819d2d383ab83b817d2888e4e5f0c.
BaseConfiguration ITEMS registers 26 keys: start, stop, open, close, null, prime,
subsup, over, left, middle, right, break, begin, end, style, position, cell, mml,
fn, not, nonscript, dots, array, eqnarray, equation, mstyle. This configured table
is distinct from the factory's DefaultStackItems/base metadata.

Simple lifecycle/state/style/position and script reducers use the accepted
BaseItem, NodeUtil, TexError, MML and stack factories. Fn also uses entities
ApplyFunction; Not uses MapHandler/CharacterMap not_remap; Nonscript uses the
actual ParseOptions node-list registration. Start.finalize and Equation require
real Tags; they cannot form a working parser with placeholder tags.

Over.fixedFence and Left.fenced require real ParseUtil. Array+EqnArray is a cohesive
subsystem: checkItem/createMml/handleFrame/StartEntry/getEntry/EndEntry/EndRow/
EndTable/checkLines/addRowSpacing, plus eqnarray overrides/extendArray/addIndentshift.
It needs actual ParseUtil.fenced/addArgs/fixInitialMO, UnitUtil.dimen2em/em, Tags
start/getTag/clearTag/end/enTag and the actual TexParser's mutable string/i.
Do not fake parser input/index or implement a separate array parser. Source
constructor phases remain necessary for all subclasses.

Freeze complete service interfaces on the one actual ParseOptions/TexParser/
configuration/Tags implementation, then complete Stack and the full configured
base item registry. Individual reducer acceptance may be divided for ownership,
but cannot substitute for complete base/AMS/enabled-package TeX -> MML parity.
Fonts/common wrappers/shared layout, SVG and Folia remain required afterward.
