module folia.math.core.mml_tree.mml_document;
import std;
namespace folia::math::mml {
MmlNode::MmlNode(MmlDocument& owner,NodeKind kind):owner_(&owner),kind_(kind){}
}
