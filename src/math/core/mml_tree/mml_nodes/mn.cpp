/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mn.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mn;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
MmlMn::MmlMn(MmlDocument &document) : AbstractMmlTokenNode(document, NodeKind::mn) {}

AttributeSet MmlMn::defaults() {
    return nodeDefaultAttributes(NodeKind::mn);
}

NodeKind MmlMn::kind() const noexcept {
    return NodeKind::mn;
}
} // namespace folia::math::mml
