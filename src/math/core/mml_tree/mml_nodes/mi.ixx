// MathJax 4.1.3 MmlNodes/mi.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mi;
export import folia.math.core.mml_tree.mml_node;
export import folia.math.core.mml_tree.leaf_host;
import std;
export namespace folia::math::mml {
class MmlMi:public AbstractMmlTokenNode{public:using AbstractMmlTokenNode::AbstractMmlTokenNode;static PropertyList& defaults();static Pattern& operatorName();static Pattern& singleCharacter();void initializeClassFields()override;std::string kind()const override;void setInheritedAttributes(AttributeList={},Property=false,Property=0,Property=false)override;MmlNode* setTeXclass(MmlNode*)override;};
}
