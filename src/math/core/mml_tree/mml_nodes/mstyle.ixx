// MathJax 4.1.3 MmlNodes/mstyle.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mstyle;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMstyle:public AbstractMmlLayoutNode{public:using AbstractMmlLayoutNode::AbstractMmlLayoutNode;static PropertyList& defaults();std::string kind()const override;bool notParent()const override;protected: void inheritAttributes(AttributeList,Property,Property,Property)override; public:
protected:void setChildInheritedAttributes(AttributeList,Property,Property,Property)override;};
}
