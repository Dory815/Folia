module folia.math.core.mml_tree.mml_nodes.malignmark;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
AttributeSet MmlMalignmark::defaults() {
    return nodeDefaultAttributes(NodeKind::malignmark);
}

NodeKind MmlMalignmark::kind() const noexcept {
    return NodeKind::malignmark;
}

double MmlMalignmark::arity() const {
    return 0.0;
}

bool MmlMalignmark::isSpacelike() const {
    return true;
}
} // namespace folia::math::mml
