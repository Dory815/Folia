module folia.math.core.mml_tree.mml_nodes.mphantom;
import folia.math.core.mml_tree.node_defaults;

namespace folia::math::mml {
MmlMphantom::MmlMphantom(MmlDocument &document)
    : AbstractMmlLayoutNode(document, NodeKind::mphantom) {}

AttributeSet MmlMphantom::defaults() {
    return nodeDefaultAttributes(NodeKind::mphantom);
}

NodeKind MmlMphantom::kind() const noexcept {
    return NodeKind::mphantom;
}
} // namespace folia::math::mml
