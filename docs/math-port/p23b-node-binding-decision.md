# P23b Root decision — lossless node boundaries before the atomic migration

Status: architectural decision supported by original Source observations.
Concrete declaration/red-test freeze and implementation dispatch are still
required. This document authorizes no agent to edit production algorithms.
P23a's accepted ChildArray is a prerequisite; the active FactoryChildren alias
and every consumer still need replacement together. No dense/sparse runtime
selector, pointer compatibility overload or mirrored parent field is allowed.

## Source evidence and responsibilities

Root independently replayed actual pinned Source:

| Corpus | Cases | Actual thrown errors | SHA-256 (LF) |
|---|---:|---:|---|
| Existing Node/NodeUtil sparse operations | 111 | 27 | `0a91ef40e88691fc5ae57f1406a635c4cab925d86650c1c1923fd07cb3c2e743` |
| New actual MML classes, copy/inferred/arity/scripts/callbacks | 299 | 99 | `b9717d5f5213ad400617fb680f92d3f744cae0b7f7b2182f86a194b205367ab4` |
| New direct/delegated MML return values and getter rebinds | 32 | 9 | `b9e7aefa6c3d0910acd775acb2a63652e1c20287d314f431b9de33dc71ab7ebf` |

The 299 cases execute the original MmlFactory registry/classes, not a mock
reimplementation. All 41 loaded original CJS files are SHA-pinned in the fixture;
the 32 return cases verify the same loaded Source prerequisites. No native
algorithm contributes an expected value. Array own presence, tagged values,
node identity, parents, newly allocated products, captured arrays, callback order
and exact thrown error are recorded. Partial failed operations remain observable.

The superscript-first examples use actual MmlMsubsup plus original NodeUtil;
they are a stronger node-level probe than the earlier generic shaped array, but
still do not execute the TeX parser. Parser/Common/SVG/Folia acceptance is open.
There is no passing native counterpart for these new corpora yet.

The return corpus observes ten Undefined and five Null returns. In particular
Layout/Base core() directly returns slot zero, whereas coreMO() dereferences it.
Original MmlMrow.core() may return a newly rebound slot after its isEmbellished
getter. Original MmlMstyle.notParent returns Undefined/Null for a falsey first
slot, despite its TypeScript boolean annotation. A blanket required-node read,
pointer-null projection or native bool return would change these behaviors.

## Sole storage and concrete signature decisions

FactoryChildren<Product> becomes shared_ptr<ChildArray<Product>>. Node's
childNodes field is initialized to one fresh ChildArray. All creation/casting,
wrapper optional handles and TreeJoinGuard identities use that same type.
The P23a ChildValue tags and borrowed factory-owned Product identity stay fixed.

The following are the chosen typed boundaries, to be written into the existing
declaration modules in the upcoming red-test freeze:

```cpp
template<class Product> class Node {
    ChildValue<Product> parent_ = PropertyNull{}; // sole parent storage
public:
    virtual ChildValue<Product> getParent() const;
    virtual void setParent(ChildValue<Product> value);
    virtual ChildValue<Product> appendChild(ChildValue<Product> child);
    virtual ChildValue<Product> replaceChild(ChildValue<Product> replacement,
                                           ChildValue<Product> previous);
    virtual ChildValue<Product> removeChild(ChildValue<Product> child);
    virtual std::optional<std::size_t> childIndex(ChildValue<Product> child) const;
};
using NodeValue = tree::ChildValue<MmlNode>; // in MML's existing declaration module
// MmlNode and every override:
virtual NodeValue Parent() const;
virtual NodeValue core();
virtual NodeValue coreMO();
virtual Property notParent() const; // Boolean, Missing, or PropertyNull
```

Self/fresh-product returns may keep raw Product* when the Source operation
cannot directly return a nullish child value (factory construction, copy, and
Node-only fallback helpers). They must explicitly lift into NodeValue when
implementing a tagged virtual return. Node-valued input/return boundaries beyond
this list, including setTeXclass's prior/result binding, need the complete Source
call-domain inventory before declarations freeze; do not infer a pointer-only
result merely from a TS annotation. Direct child helper Get returns NodeValue;
helpers may require a node only where the original caller dereferences it.

Root has frozen, compiled real red stubs, implemented and independently accepted
`Product* ChildValue::requireNodeForWrite(std::string_view member) const`.
Its seven Source-derived assertions and five compiling semantic mutations are
recorded in `evidence/p23b-child-write-acceptance.md`. It emits the original `Cannot set properties of
undefined/null (setting 'parent')` error at a parent write receiver. Existing
requireNode remains the read-receiver error. The value assigned to parent may
legally be Null or Undefined; a nullish **receiver** causes the setting error.
Never reject setParent(Null) as an invalid assignment.

The sole parent slot is private so internal reads/writes cannot bypass the
uniform typed binding. Default getParent/setParent implement ordinary Source
data fields with no callback. Controlled native subclass overrides model bound
Source accessors used by the observer; every production parent access uses the
same binding. There is no auxiliary raw parent or test-only callback store.
This is not a generic JS property descriptor/Proxy implementation.

Pinned MathJax classes define ordinary lower-case parent and childNodes fields.
Capitalized Parent and notParent are real virtual logical-ancestry getters.
Root's Source111 final case and ten new MML cases deliberately install a parent
descriptor in the observer to test capture/write timing. Those cases are real
Source observations; they do not mean ordinary production parent writes run a
callback. Their acceptance must neither silently disappear nor be mistaken for
an arbitrary generic descriptor-engine requirement.

## Preserve each original algorithm

Generic append pushes before setting parent. replace calls virtual childIndex,
then writes into the current array, sets new parent, then reads/possibly clears
old parent. remove calls virtual childIndex, splices the current array, then
clears parent. Empty append returns its argument and Empty replace returns the
old argument, retaining their tags. Source TypeError class, message and partial
array/parent state are acceptance outputs, not just exception presence.

Generic copy skips falsey values. MML copy deliberately pushes explicit Null
for every falsey yielded value. NodeUtil.copyChildren reads every source index
and setChild writes it to the target, converting a source hole to present
Undefined. These three conversions are different and must remain so.

Source for-of captures the array handle and uses live length, returning Undefined
for a hole. Source forEach captures length and checks live Has/Get, skipping
absent properties. Direct index loops either capture an array or reread the
receiver property according to their original expression. Arity truncation uses
sparse slice and a new handle; growth dispatches append. MML infinite-arity
inferred flattening uses forEach; finite conversion uses explicit row creation
and live setChildren. Never replace all of them with a dense range loop.

## Upcoming execution and acceptance

Root must first freeze every affected signature, parent access, iteration policy
and actual/native observer, including NodeUtil's full-file responsibility. Its
original mutable attrs set, properties/attributes, operator-dictionary reads and
virtual receiver dispatch cannot be replaced by four convenient test helpers.
Do not claim NodeUtil file acceptance from its child subcorpus alone.

Root then compiles real red stubs in a closed module graph and allocates disjoint
implementation files with explicit Source/member maps. Tests/golds/manifests,
builds, mutations, file inventory and acceptance remain Root-owned. All 31
existing child consumers plus affected overrides, parent consumers, NodeUtil and
the 19 native dense test observers/generators must migrate in one active graph.
No old overload can be retained solely to make those observers compile.

Acceptance requires all Source111/MML299/return32 observations, unchanged dense
behavior expectations through migrated native observers, compiling semantic
mutations, lifetime/identity review, then the complete core build/tests. A host
packet alone advances no whole MathJax Source-file tally and no parser, Common,
SVG, Direct2D or Folia gate. The accepted counts remain 54/246 for math and
34/70 renderer functions for Mermaid; those count local evidence only.
