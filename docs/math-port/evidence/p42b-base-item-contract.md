# P42B BaseItem and StackItemFactory contract

Source: MathJax 4.1.3, fb987178c2d279a99b0db5ea02e751691703955e.
StackItem.ts SHA-256 ac138d321e1efbc54afbd961a4b6bfa24f7d0923eddba1c8d891dee9424602b5;
StackItemFactory.ts SHA-256 229bf0f6bd58642bff86102bb8fb6fd2b9a61d3f52ad31ce738f3f2cefa2e405.
P42A already accepts MmlStack. This packet completes the BaseItem responsibility
and actual StackItemFactory specialization; it does not implement Stack/reducers.

Root owns all interfaces, host bindings, ownership, constructor phases, static
metadata, CMake, actual-original gold, probes and final acceptance. Luna may edit
only base_item.cpp's seventeen methods after the compiled stub gate fails.
No agent edits tests/hosts, builds, commits or accepts its own implementation.

## One identity and explicit Source constructor phases

BaseItem is the existing StackItemContext identity, MmlStack and FactoryNode on
one object. StackItemFactory inherits one AbstractFactory<BaseItem> registry plus
one independent service base; itemFactory() returns that same concrete factory.
The service definition moves into the StackItem Source-owner module to avoid
declaring BaseItem in two named modules. NodeFactory/configuration remain borrowed
edges into the one future ParseOptions/session. MmlStack create always uses
configuration.nodeFactory; reducers' this.factory.create is the stack-item factory.

Factory adopt stores one shared actual BaseItem, including partially initialized
products. reference(raw) upcasts that same owner's StackItemContext; it never
creates a facade. Every class binding explicitly adopts before initialization.
The session outlives its factories, parser, config and all product use; internal
StackItemRef values must not escape that session with borrowed edges in use.

Physical allocation starts BaseItem factory/env/global/properties at Missing.
Root Source phase initializes MmlStack, assigns factory, fresh global, fresh
properties, then calls virtual isOpen once and assigns fresh env if true. Derived
Source fields are still Missing then; bindings initialize them afterward. Do not
call isOpen in a C++ base constructor or preinitialize Source derived fields.
Captured actual FactoryClass metadata provides live errorsBinding; replacement
of the registered kind must not change old products' constructor class identity.

BaseItem.fail/success are shared mutable CheckTuple objects [Null,false/true].
Their identities are shared across instances; checkItem uses BaseItem's static
objects. CheckType also represents Null/Missing required by future reducers.
Replacement lists retain existing NodeValue/StackItemRef products.

## Prop and actual package domain

StackProperty carries canonical Property, existing NodeValue, or a shared
recursive StackPropertyStorage ordinary record. _properties is one rich record;
scalar style/color objects stay the same shared PropertyList handles and are
never promoted/copied. Node-bearing records are created rich from the outset.
get/set preserve prototype data, own shadowing and ordinary record/null __proto__
assignment. assign snapshots own enumerable keys, reads current values in order,
mutates the same target and preserves nested identities. It must not deep-copy,
serialize MML, or add node values to the generic Property module below MML.

Audited callers: BaseMethods.ts:253/328 position/primes and StackItem.ts:509 num
are MML values; CasesConfiguration.ts:101-115 and BaseItems.ts:693-705 use left/right
MML values. BaseMethods.ts:468/484, ColorMethods.ts:66 and BaseItems.ts:650-659 use
shared scalar styles, consumed as MML definitions without node writes to those
objects. Empheq setProperties receives splitOptions EnvList. No enabled caller
promotes a scalar record with MML values, uses an MML node as assignment source or
prototype, or writes __proto__. Node-source/prototype reflection and arbitrary
property descriptors/iterators are explicitly unbound; this is not whole-JS
reflection. Regular shared record/null prototype behavior is nevertheless bound.

Environment/global are canonical scalar property objects. getName returns the
raw StackProperty through virtual getProperty; actual error-name callers supply
scalar string/nullable values. Error tables contain shared PropertyArray string
tuples; extra slots/empty arrays and inherited/falsy entries remain live. Custom
non-array error iterators and non-scalar error substitutions are not this profile.

## Seventeen bodies, exact evaluation order

- kind=base; env get/set direct field; copyEnv=true; isOpen/isClose/isFinal=false.
- getProperty/setProperty/setProperties use the one live properties record;
  setters return this. getName calls virtual getProperty(name).
- isKind compares argument with one virtual kind read.
- checkItem: call item.isKind(over), then this.isOpen if needed; toMml(false),
  item.setProperty(num,result), Clear only after normal setter return. Independently
  call item.isKind(cell), then fresh isOpen, linebreak; truthy returns fail, else
  TexError(Misplaced,Misplaced %1,item.getName()). Then item.isClose, item.kind,
  virtual getErrors for condition; if truthy, freshly read kind/getErrors again,
  capture tuple[0]/[1], read item.getName and throw TexError. Finally isFinal false
  returns success; otherwise virtual item.First -> virtual this.Push -> fail.
- clearEnv snapshots Object.keys(virtual env()) once, then each deletion calls
  virtual env() again; preserve any earlier deletions before failure.
- toString captures kind BEFORE nodes() getter, joins the captured live array
  with '; ' using captured length and per-element current reads; holes/Null are
  blank. C++ compound expressions must not reorder callbacks.
- getErrors captures actual classInfo once, reads its live errors table and key,
  truthy value wins, else freshly reads BaseItem.errors[key]; do not copy tuples.
- addLatexItem defaults only Missing prefix to empty; UTF-16 String.slice with
  JS index truncation/clamping. Empty slice touches no node. Prefix truthiness
  controls concatenation. Each write separately reads node.attributes and the
  current TexConstant.Attr key. Write LATEXITEM first; LATEX only if tex!='}'.
  Never cache attrs across writes or undo the first write on a later failure.

Root hosts bind primitive coercion, UTF-16 slice, live join, actual attribute
access and current metadata. No blanket MML tree recursion budget is added to
these stack operations. Source calls use explicit temporaries for C++ ordering.

## Factory default trap and gate

DefaultStackItems has key base/DummyItem while defaultKind is dummy, but inherited
defaultNodes stays empty. new StackItemFactory() has no registered classes;
create(base/dummy/unknown) reaches the generic absent creator .apply error. The
normal ParseOptions path explicitly supplies configuration.items. Fix any error
mismatch at generic Factory, never with local exception translation/fallback.

The initial 76 actual original records were generated before release; Root has
strengthened them to 84 records for attribute/constructor/order/ownership. Root will
strengthen attribute/constructor/order/ownership witnesses as needed without
changing original results to fit Native. Gate: compiled stubs fail assertions,
completed methods match Source, independent compiling faults are caught, old
MmlStack gate and full core tests pass. Renderer/Folia goals remain open.
