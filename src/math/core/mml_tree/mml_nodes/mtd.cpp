/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/mtd.ts methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mtd;
import folia.math.core.mml_tree.table_host;
import std;

namespace folia::math::mml {

void MmlMtd::verifyChildren(PropertyList options) {
    tree::TreeCallGuard<MmlNode> guard;
    if (this->parent && !this->parent->isKind("mtr")) {
        const std::string message = kind() +
            " can only be a child of an mtr or mlabeledtr";
        (void)mError(message, ErrorOptions(options), true);
        return;
    }
    AbstractMmlBaseNode::verifyChildren(options);
}

MmlNode* MmlMtd::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    MmlNode& child = host::tableChild(host::tableChildren(*this), 0);
    child.setTeXclass(nullptr);
    return this;
}

}
