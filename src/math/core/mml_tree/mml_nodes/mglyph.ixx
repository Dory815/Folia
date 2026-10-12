// Generated MathJax 4.1.3 ts/core/MmlTree/MmlNodes/mglyph.ts; Apache-2.0.
// SHA256 2c2d336067d3970c55f3fcdafb82b55d4339a2c11d69088c8c7eff4a6d953504
export module folia.math.core.mml_tree.mml_nodes.mglyph;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMglyph : public AbstractMmlTokenNode {
public:
using AbstractMmlTokenNode::AbstractMmlTokenNode;
static PropertyList& defaults();
void initializeClassFields()override;
std::string kind()const override;
void verifyAttributes(Property)override;
};
}
