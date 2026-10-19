module folia.math.core.mml_tree.mml_nodes.mtd;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
AttributeSet MmlMtd::defaults() {
    return nodeDefaultAttributes(NodeKind::mtd);
}

NodeKind MmlMtd::kind() const noexcept {
    return NodeKind::mtd;
}

double MmlMtd::arity() const {
    return -1.0;
}

bool MmlMtd::linebreakContainer() const {
    return true;
}

std::string MmlMtd::linebreakAlign() const {
    return "columnalign";
}

void MmlMtd::verifyChildren(VerifyOptions const &options) {
    if (auto *parentNode = parent(); parentNode && !parentNode->isKind(NodeKind::mtr) &&
                                     !parentNode->isKind(NodeKind::mlabeledtr)) {
        (void)mError(u"mtd can only be a child of an mtr or mlabeledtr", options, true);
        return;
    }
    AbstractMmlNode::verifyChildren(options);
}

MmlNode *MmlMtd::setTeXclass(MmlNode *previous) {
    getPrevClass(previous);
    if (children().empty() || !children().front())
        throw std::logic_error("mtd requires its inferred row child before TeX-class traversal");
    children().front()->setTeXclass(nullptr);
    return this;
}
} // namespace folia::math::mml
