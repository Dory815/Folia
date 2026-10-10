// MathJax 4.1.3 MmlNodes/mn.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mn;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMn:public AbstractMmlTokenNode{public:using AbstractMmlTokenNode::AbstractMmlTokenNode;static PropertyList& defaults();void initializeClassFields()override;std::string kind()const override;};
}
