/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/mtd.ts methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mtd;
import folia.math.core.mml_tree.table_host;
import std;

namespace folia::math::mml {

void MmlMtd::verifyChildren(Property options) {
    tree::TreeCallGuard<MmlNode> guard;
    if (this->getParent().nodeOrNull()) {
        const NodeValue parent = this->getParent();
        if (!host::tableNode(parent, "isKind").isKind("mtr")) {
            mError(this->kind() + " can only be a child of an mtr or mlabeledtr", options, true);
            return;
        }
    }
    AbstractMmlNode::verifyChildren(options);
}

NodeValue MmlMtd::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    const Children children = host::tableChildren(*this, "0");
    host::tableChild(children, 0, "setTeXclass").setTeXclass(PropertyNull{});
    return this;
}

}
