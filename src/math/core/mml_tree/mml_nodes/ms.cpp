/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/ms.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.ms;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
MmlMs::MmlMs(MmlDocument &document) : AbstractMmlTokenNode(document, NodeKind::ms) {}

AttributeSet MmlMs::defaults() {
    return nodeDefaultAttributes(NodeKind::ms);
}

NodeKind MmlMs::kind() const noexcept {
    return NodeKind::ms;
}
} // namespace folia::math::mml
