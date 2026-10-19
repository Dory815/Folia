module folia.math.core.mml_tree.mml_nodes.msqrt;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
MmlMsqrt::MmlMsqrt(MmlDocument &document) : AbstractMmlNode(document, NodeKind::msqrt) {}

AttributeSet MmlMsqrt::defaults() {
    return nodeDefaultAttributes(NodeKind::msqrt);
}

NodeKind MmlMsqrt::kind() const noexcept {
    return NodeKind::msqrt;
}

double MmlMsqrt::arity() const {
    return -1;
}

bool MmlMsqrt::linebreakContainer() const {
    return true;
}

MmlNode *MmlMsqrt::setTeXclass(MmlNode *previous) {
    getPrevClass(previous);
    if (children().empty())
        throw std::out_of_range("msqrt requires its inferred row");
    children()[0]->setTeXclass(nullptr);
    return this;
}

void MmlMsqrt::setChildInheritedAttributes(AttributeList const &attributes, bool display,
                                           double level, bool) {
    if (children().empty())
        throw std::out_of_range("msqrt requires its inferred row");
    children()[0]->setInheritedAttributes(attributes, display, level, true);
}
} // namespace folia::math::mml
