module folia.math.core.mml_tree.mml_nodes.mroot;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
MmlMroot::MmlMroot(MmlDocument &document) : AbstractMmlNode(document, NodeKind::mroot) {}

AttributeSet MmlMroot::defaults() {
    return nodeDefaultAttributes(NodeKind::mroot);
}

NodeKind MmlMroot::kind() const noexcept {
    return NodeKind::mroot;
}

double MmlMroot::arity() const {
    return 2;
}

bool MmlMroot::linebreakContainer() const {
    return true;
}

MmlNode *MmlMroot::setTeXclass(MmlNode *previous) {
    getPrevClass(previous);
    if (children().size() < 2)
        throw std::out_of_range("mroot requires two children");
    children()[0]->setTeXclass(nullptr);
    children()[1]->setTeXclass(nullptr);
    return this;
}

void MmlMroot::setChildInheritedAttributes(AttributeList const &attributes, bool display,
                                           double level, bool prime) {
    if (children().size() < 2)
        throw std::out_of_range("mroot requires two children");
    children()[0]->setInheritedAttributes(attributes, display, level, true);
    children()[1]->setInheritedAttributes(attributes, false, level + 2, prime);
}
} // namespace folia::math::mml
