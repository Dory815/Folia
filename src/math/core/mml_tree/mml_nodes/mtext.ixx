// MathJax 4.1.3 MmlNodes/mtext.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mtext;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMtext:public AbstractMmlTokenNode{public:using AbstractMmlTokenNode::AbstractMmlTokenNode;static PropertyList& defaults();static std::vector<std::string>& NONSPACELIKE();void initializeClassFields()override;std::string kind()const override;bool isSpacelike()const override;};
}
