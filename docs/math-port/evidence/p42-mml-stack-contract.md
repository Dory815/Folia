# P42A MmlStack contract

Source StackItem.ts SHA-256
ac138d321e1efbc54afbd961a4b6bfa24f7d0923eddba1c8d891dee9424602b5,
MathJax 4.1.3 pinned commit fb987178c2d279a99b0db5ea02e751691703955e.
This packet implements the entire MmlStack responsibility, not BaseItem,
StackItemFactory, Stack or the concrete reducers in the same parser pipeline.

Root owns all interfaces, generic storage/host helpers, CMake, oracle, probes,
expected data and acceptance. Luna mml_row_mi may edit only stack_item.cpp's
eleven semantic bodies after compiled stubs fail the actual original gold.
Review agents are read-only. Agents do not build, change tests or accept work.

## State, ownership and services

The per-item node list is the existing ChildArray<MmlNode> shared handle.
Its entries are the existing MML factory-owned NodeValue identities. Constructor
retains the supplied handle; nodes() returns it. startStr is UTF-16; startI and
stopI are binary64, initially zero. Clear rebinds to a fresh empty array.
No second AST, VM, parser, MML storage or renderer is introduced.

StackItemFactoryService is the borrowed configuration field of the future real
StackItemFactory. ParseOptions will implement StackItemFactoryConfiguration and
NodeFactoryConfiguration on its one instance; nodeFactoryReference reads its
one authoritative NodeFactory field. There is no dummy ParseOptions or copied
configuration state. Missing and explicit Null are distinct at every service
edge; the session owns all factories/configuration/products throughout calls.

The generic ChildArray now stores node-valued named properties separately from
indexed slots. Canonical 0..4294967294 index strings route to indexed storage;
other numeric keys (including negative/fractional/NaN/Infinity) are ordinary
named slots and do not grow length or enter indexed slice/pop. Named data lookup
walks the same array prototype chain. This is not an empty-First special case.
Scalar length and built-in Array/Object methods are separate typed operations;
the node-property API is used only for numeric-computed keys. Arbitrary JS
descriptors, overridden Array methods and intrinsic node-valued lookups are not
part of the enabled package profile or a whole-JavaScript reflection claim.

## Semantic bodies

- Push appends all supplied tagged values to the current raw nodes_ array;
  empty Push still resolves its array receiver. Pop forwards native pop.
- First get/set capture raw nodes_ BEFORE virtual Size, compute Size()-1,
  convert to the canonical JS property key and access that captured array.
  A Null receiver throws only after Size/key evaluation; setter errors say
  setting rather than reading. Empty First writes named -1, keeping length zero.
- Last get/set use raw nodes_ index zero without Size. Empty write grows length.
- Peek defaults only nullish count to one; resolve/capture slice receiver BEFORE
  virtual Size and numeric coercion of count. Use Size()-count with JS
  ToIntegerOrInfinity and relative-index clamping, preserving sparse holes and
  inherited indexed slots in a new shallow array. Enabled package calls are
  Peek() and Peek(2); wider primitive cases are direct boundary witnesses.
- Size reads raw nodes_.length. Clear allocates a fresh array.
- toMml defaults Missing inferred to true only; checks raw length==1 and falsey
  forceRow, then returns virtual First. Otherwise call virtual create with node,
  inferredMrow/mrow chosen by truthiness, the exact current array and fresh {}.
- create calls virtual factory(), reads its current configuration, reads that
  configuration's current NodeFactory and forwards exact kind/rest/result.
  Do not cache edges or translate service errors at a different boundary.

Source methods directly access private nodes_; only derived users use virtual
nodes(). Keep those responsibilities distinct. Reads/calls must use explicit
temporaries wherever C++ argument evaluation would lose Source order.

## Acceptance

The Root oracle executes pinned original compiled methods with hash-checked TS
and locked CJS. The 74 complete observations compare return states, array holes,
aliases, virtual dispatch, ordering, partial effects and exact errors. Required
gate: compiled stub fails assertions (not compiler errors), completed body passes
all original records, independent compiling semantic faults are detected, and
the complete normal core build/test passes. No public TeX/SVG/Folia claim follows.
