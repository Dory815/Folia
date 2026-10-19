module folia.math.core.mml_tree.mml_nodes.tex_atom;
import folia.math.core.mml_tree.node_defaults;
import folia.math.core.mml_tree.operator_class;
import std;

namespace folia::math::mml {
MmlTeXAtom::MmlTeXAtom(MmlDocument &document) : AbstractMmlBaseNode(document, NodeKind::tex_atom) {}

AttributeSet MmlTeXAtom::defaults() {
    return nodeDefaultAttributes(NodeKind::tex_atom);
}

NodeKind MmlTeXAtom::kind() const noexcept {
    return NodeKind::tex_atom;
}

double MmlTeXAtom::arity() const {
    return -1.0;
}

bool MmlTeXAtom::notParent() const {
    return true;
}

MmlNode *MmlTeXAtom::setTeXclass(MmlNode *previous) {
    if (!children().empty())
        (void)children()[0]->setTeXclass(nullptr);
    return adjustOperatorTeXclass(*this, previous);
}
} // namespace folia::math::mml
