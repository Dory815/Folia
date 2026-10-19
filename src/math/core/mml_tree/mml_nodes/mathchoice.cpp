module folia.math.core.mml_tree.mml_nodes.mathchoice;
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
MathChoice::MathChoice(MmlDocument &document)
    : AbstractMmlBaseNode(document, NodeKind::math_choice) {}

AttributeSet MathChoice::defaults() {
    return nodeDefaultAttributes(NodeKind::math_choice);
}

NodeKind MathChoice::kind() const noexcept {
    return NodeKind::math_choice;
}

double MathChoice::arity() const {
    return 4.0;
}

bool MathChoice::notParent() const {
    return true;
}

void MathChoice::setInheritedAttributes(AttributeList const &attributes, bool display, double level,
                                        bool prime) {
    const double selected = display ? 0.0 : std::clamp(level, 0.0, 2.0) + 1.0;
    MmlNode *branch = nullptr;
    if (std::isfinite(selected) && selected >= 0.0 && std::floor(selected) == selected) {
        const auto index = static_cast<std::size_t>(selected);
        if (index < children().size())
            branch = children()[index];
    }
    if (!branch) {
        branch = &document().factory().make(NodeKind::mrow);
    }

    MmlNode *parent = Parent();
    if (!parent)
        throw std::logic_error("MathChoice inheritance requires a parent");
    parent->replaceChild(*branch, *this);
    branch->setInheritedAttributes(attributes, display, level, prime);
}
} // namespace folia::math::mml
