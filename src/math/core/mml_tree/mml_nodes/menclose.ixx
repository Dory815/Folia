// Generated MathJax 4.1.3 ts/core/MmlTree/MmlNodes/menclose.ts; Apache-2.0.
// SHA256 ffa4ab507744cb4f190312ca8dcd692474c333bee6e19c86709554aa9dad4333
export module folia.math.core.mml_tree.mml_nodes.menclose;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMenclose : public AbstractMmlNode {
public:
using AbstractMmlNode::AbstractMmlNode;
static PropertyList& defaults();
void initializeClassFields()override;
std::string kind()const override;
double arity()const override;
bool linebreakContainer()const override;
NodeValue setTeXclass(NodeValue)override;
};
}
