// MathJax 4.1.3 MmlNodes/merror.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.merror;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMerror:public AbstractMmlNode{public:using AbstractMmlNode::AbstractMmlNode;static PropertyList& defaults();void initializeClassFields()override;std::string kind()const override;double arity()const override;bool linebreakContainer()const override;};
}
