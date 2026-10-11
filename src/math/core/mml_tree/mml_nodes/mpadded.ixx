// Generated MathJax 4.1.3 ts/core/MmlTree/MmlNodes/mpadded.ts; Apache-2.0.
// SHA256 b5aac516c32fb019ac5ace504f86312f3e0849e1faa44006567bac05815c7271
export module folia.math.core.mml_tree.mml_nodes.mpadded;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMpadded : public AbstractMmlLayoutNode {
public:
using AbstractMmlLayoutNode::AbstractMmlLayoutNode;
static PropertyList& defaults();
std::string kind()const override;
bool linebreakContainer()const override;
NodeValue setTeXclass(NodeValue)override;
};
}
