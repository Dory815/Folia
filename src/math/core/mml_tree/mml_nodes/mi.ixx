// MathJax 4.1.3 MmlNodes/mi.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mi;
export import folia.math.core.mml_tree.mml_node;
export import folia.math.core.mml_tree.leaf_host;
import std;
export namespace folia::math::mml {
class MmlMi:public AbstractMmlTokenNode{public:using AbstractMmlTokenNode::AbstractMmlTokenNode;static PropertyList& defaults();static Pattern& operatorName();static Pattern& singleCharacter();void initializeClassFields()override;std::string kind()const override;protected: void inheritAttributes(AttributeList,Property,Property,Property)override; public:MmlNode* setTeXclass(MmlNode*)override;};
}
