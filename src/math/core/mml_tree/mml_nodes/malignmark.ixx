// Generated MathJax 4.1.3 ts/core/MmlTree/MmlNodes/malignmark.ts; Apache-2.0.
// SHA256 f35d8b9edcf1afce67f74eef1dc1c04434f9caea944b3f3fd439d1aaf484cb61
export module folia.math.core.mml_tree.mml_nodes.malignmark;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMalignmark : public AbstractMmlNode {
public:
using AbstractMmlNode::AbstractMmlNode;
static PropertyList& defaults();
std::string kind()const override;
double arity()const override;
bool isSpacelike()const override;
};
}
