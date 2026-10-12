# P43 actual parser Stack contract

Root freezes input/tex/Stack.ts against MathJax 4.1.3, pinned source
36be7583b14111961367e9e3f1dd5911456d235b096a2fcee62f438d37c7e11a.
The locked compiled CJS is the independent observer, loaded through the existing
simple-mml-reference -> arithmetic-reference -> mo-pattern lock validation.

## Responsibilities and identity

- stack.ixx declares the actual Source Stack with a private vector of existing
  StackItemRef owners, borrowed current factory, shared scalar environment and
  global records. It does not add an item graph or mirror MML lists.
- stack_bindings.cpp only invokes the Source constructor phase on Stack.
- stack.cpp is Luna's sole write boundary: constructor phase, environment
  getter/setter, Push, Pop, Top, Prev, height and toString. No test or host edits.
- stack_host.ixx supplies Root-owned item/nullish reads, the same actual factory
  and product owner, existing NodeUtil.isNode, CheckTuple/CheckList identities,
  array spread snapshot, current-target Object.assign, ordinary numeric lookup,
  inherited env deletion and captured-length/live-slot join operations.
- Root owns the oracle, generated tests, fault injection and acceptance. The
  fixture uses real generic StackItemFactory and BaseItem/MML identities with
  explicit virtual observations. Test-only Start/Mml/Probe classes are observer
  instruments, not production reducers or a substitute configured registry.

## Source order

Push visits the captured rest array, skips falsey inputs, wraps only actual MML
nodes using mml, writes the current global before current Top.checkItem, reads
the shared tuple into local bindings, and skips falsey success. A truthy list
(including an empty list) pops and recursively pushes a fresh spread snapshot;
the original input item is discarded. null-kind skips append only. Environment
condition, copyEnv, receiver env, current Stack.env, assignment and final env
read occur separately in that order. Prior append/global/assignment effects
survive exceptions. No catch/fallback/alternate parser is allowed.

Pop removes the item before isOpen. delete item.env is a no-op for the inherited
BaseItem accessor: it must not invoke setEnv or clear backing storage. Read live
remaining Top.env after any isOpen callback; empty result gets a fresh record.
Top defaults only Missing, compares current length with numeric n, then reads
current length again before the second numeric conversion/subtraction and an
ordinary index read. Out-of-range comparison returns literal Null; invalid
numeric index returns Missing. Prev reads virtual Top exactly once and selects
virtual First versus Pop by truthiness. toString uses private stack length and
live slots while virtual item conversions may mutate it.

## Typed package boundaries

Real enabled parser environments are existing shared scalar PropertyList
records. Inherited keys are ignored by Object.assign, nested records remain
aliases and the inherited __proto__ behavior comes from PropertyList. Nullish
source is ignored; nullish receiver rejects. Assignment of nonempty properties
to primitive boxes is unbound, not silently rewritten. No enabled caller uses
primitive environment boxes. Source numeric arguments use the accepted Property
number conversion; arbitrary evaluated JS functions/descriptors/iterators are
not native bindings. The private stack has no synthetic sparse array/prototype
API. Live mutations occur through actual Push/Pop callbacks.

The future actual ParseOptions owns Stack/factory/configuration lifetimes. No
complete TeX command or SVG/Folia claim follows from this packet. Complete real
base reducers, tags/ParseUtil/UnitUtil, TeXParser, packages, font/layout and output
still follow the frontier in p43-parser-frontier.md. Mermaid remains paused.
