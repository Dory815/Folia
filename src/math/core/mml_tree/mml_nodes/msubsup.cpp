// Native port of MathJax 4.1.3 MmlNodes/msubsup.ts; licensed under Apache-2.0.
module folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
MmlMsubsup::MmlMsubsup(MmlDocument &document) : MmlMsubsup(document, NodeKind::msubsup) {}

MmlMsubsup::MmlMsubsup(MmlDocument &document, NodeKind kind)
    : AbstractMmlBaseNode(document, kind) {}

AttributeSet MmlMsubsup::defaults() {
    return nodeDefaultAttributes(NodeKind::msubsup);
}

NodeKind MmlMsubsup::kind() const noexcept {
    return NodeKind::msubsup;
}

double MmlMsubsup::arity() const {
    return 3;
}

std::size_t MmlMsubsup::base() const {
    return 0;
}

std::size_t MmlMsubsup::sub() const {
    return 1;
}

std::size_t MmlMsubsup::sup() const {
    return 2;
}

void MmlMsubsup::setChildInheritedAttributes(AttributeList const &attributes, bool display,
                                             double level, bool prime) {
    const auto nodes = children();
    nodes[base()]->setInheritedAttributes(attributes, display, level, prime);
    nodes[sub()]->setInheritedAttributes(attributes, false, level + 1, prime || sub() == 1);
    if (sup() >= nodes.size() || !nodes[sup()])
        return;
    nodes[sup()]->setInheritedAttributes(attributes, false, level + 1, prime || sub() == 2);
}

MmlMsub::MmlMsub(MmlDocument &document) : MmlMsubsup(document, NodeKind::msub) {}

AttributeSet MmlMsub::defaults() {
    return nodeDefaultAttributes(NodeKind::msub);
}

NodeKind MmlMsub::kind() const noexcept {
    return NodeKind::msub;
}

double MmlMsub::arity() const {
    return 2;
}

MmlMsup::MmlMsup(MmlDocument &document) : MmlMsubsup(document, NodeKind::msup) {}

AttributeSet MmlMsup::defaults() {
    return nodeDefaultAttributes(NodeKind::msup);
}

NodeKind MmlMsup::kind() const noexcept {
    return NodeKind::msup;
}

double MmlMsup::arity() const {
    return 2;
}

std::size_t MmlMsup::sup() const {
    return 1;
}

std::size_t MmlMsup::sub() const {
    return 2;
}
} // namespace folia::math::mml
