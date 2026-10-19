module folia.math.core.mml_tree.mml_nodes.merror;
import folia.math.core.mml_tree.node_defaults;

namespace folia::math::mml {
MmlMerror::MmlMerror(MmlDocument &document) : AbstractMmlNode(document, NodeKind::merror) {}

AttributeSet MmlMerror::defaults() {
    return nodeDefaultAttributes(NodeKind::merror);
}

NodeKind MmlMerror::kind() const noexcept {
    return NodeKind::merror;
}

double MmlMerror::arity() const {
    return -1;
}

bool MmlMerror::linebreakContainer() const {
    return true;
}
} // namespace folia::math::mml
