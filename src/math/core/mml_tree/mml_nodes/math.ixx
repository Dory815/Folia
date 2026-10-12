// MathJax 4.1.3 MmlNodes/math.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.math;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMath:public AbstractMmlLayoutNode{public:using AbstractMmlLayoutNode::AbstractMmlLayoutNode;static PropertyList& defaults();std::string kind()const override;bool linebreakContainer()const override;std::string linebreakAlign()const override;void verifyTree(Property options=Missing{})override;
protected:void setChildInheritedAttributes(AttributeList,Property,Property,Property)override;};
}
