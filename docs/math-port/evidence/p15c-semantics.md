# P15c source mapping: semantics nodes

Pinned upstream: MathJax 4.1.3 `ts/core/MmlTree/MmlNodes/semantics.ts`, SHA-256 `e609dfc3d7510a996a87804e3929cd4f53a5aa060003e351f45f1fc5dcfb6e96`. The source’s direct imports are `PropertyList`, `AbstractMmlNode`, and `AbstractMmlBaseNode`. The C++ declarations and generated default/accessor/class-field bindings are in `src/math/core/mml_tree/mml_nodes/semantics.ixx` and `semantics_bindings.cpp`; this packet implements only the non-generated empty hook.

## Member-by-member correspondence

| Upstream source member | Upstream lines | Native owner / mapping |
|---|---:|---|
| `MmlSemantics extends AbstractMmlBaseNode` | 33 | `MmlSemantics : AbstractMmlBaseNode` in `semantics.ixx`. |
| `MmlSemantics.defaults` | 37–40 | Root-generated `MmlSemantics::defaults()` shallow-copies `AbstractMmlBaseNode::defaults()` and adds null `definitionUrl` and `encoding`. |
| `MmlSemantics.kind` | 44–46 | Root-generated getter returns `semantics`. |
| `MmlSemantics.arity` | 53–55 | Root-generated getter returns 1 (minimum arity under inherited generic verification). |
| `MmlSemantics.notParent` | 62–64 | Root-generated getter returns true. |
| `MmlAnnotationXML extends AbstractMmlNode` | 71 | `MmlAnnotationXML : AbstractMmlNode` in `semantics.ixx`. |
| `MmlAnnotationXML.defaults` | 77–84 | Root-generated fresh shallow copy of `AbstractMmlNode::defaults()` plus `definitionUrl:null`, `encoding:null`, `cd:'mathmlkeys'`, `name:''`, and `src:null`. |
| `MmlAnnotationXML.kind` | 89–91 | Root-generated getter returns `annotation-xml`. |
| `MmlAnnotationXML.setChildInheritedAttributes` | 99–100 | Implemented in `semantics.cpp` as a genuinely empty function body. Arguments are unnamed and ignored. There is no superclass call, `TreeCallGuard`, normalization, child traversal, error, or state change. |
| `MmlAnnotation extends MmlAnnotationXML` | 104 | `MmlAnnotation : MmlAnnotationXML` in `semantics.ixx`. |
| `MmlAnnotation.defaults` | 110–112 | Root-generated fresh shallow copy of `MmlAnnotationXML::defaults()`; it is a distinct defaults object. |
| `MmlAnnotation.properties` | 117–119 | Root-generated class-field phase replaces `properties` with a new `PropertyList` containing `isChars:true`; it does not merge prior properties. |
| `MmlAnnotation.kind` | 124–126 | Root-generated getter returns `annotation`. |

The upstream comments naming `MmlMroot` are stale descriptions; executable class declarations and members above define the mapping. No semantics class in this file overrides verification or TeX-class behavior. Those behaviors remain inherited from the specified base class. In particular, only the annotation-XML child-inheritance hook suppresses inherited propagation; the native body intentionally does not walk children.

## Empty hook fidelity

The source body at `semantics.ts:99–100` has no statements. The only native definition added by this packet, `MmlAnnotationXML::setChildInheritedAttributes` in `semantics.cpp`, likewise has an empty body and ignores the four parameters. It does not add even a call guard, since that would introduce behavior absent upstream. This correspondence is direct and does not imply that semantics factory registration, output wrappers, layout, or HTML integration are implemented here.

Root froze the independent actual-CJS oracle before this implementation packet. It contains constructor, class identity, inherited-value and hook cases; this report does not claim that those tests were run or that the packet is accepted. Root owns independent review, builds, mutations, and acceptance.
