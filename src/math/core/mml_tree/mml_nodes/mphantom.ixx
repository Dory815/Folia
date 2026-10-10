// MathJax 4.1.3 MmlNodes/mphantom.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mphantom;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMphantom:public AbstractMmlLayoutNode{public:using AbstractMmlLayoutNode::AbstractMmlLayoutNode;static PropertyList& defaults();void initializeClassFields()override;std::string kind()const override;};
}
