// Generated MathJax 4.1.3 ts/core/MmlTree/MmlNodes/maligngroup.ts; Apache-2.0.
// SHA256 4d8ecdae93aaff85887ec4eb09c8821811866a9e9b56e037ac61d0b6d81bf2b1
export module folia.math.core.mml_tree.mml_nodes.maligngroup;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMaligngroup : public AbstractMmlLayoutNode {
public:
using AbstractMmlLayoutNode::AbstractMmlLayoutNode;
static PropertyList& defaults();
std::string kind()const override;
bool isSpacelike()const override;
protected:
void setChildInheritedAttributes(AttributeList,Property,Property,Property)override;
};
}
