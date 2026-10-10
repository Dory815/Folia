/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mtext.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mtext;
import folia.math.core.mml_tree.leaf_host;
import std;

namespace folia::math::mml {
void MmlMtext::initializeClassFields() {
    initializeTexClass(static_cast<double>(TEXCLASS.ORD));
}

std::string MmlMtext::kind() const {
    return "mtext";
}

bool MmlMtext::isSpacelike() const {
    tree::TreeCallGuard<MmlNode> guard;
    if (!host::onlySpaces(getText())) return false;
    const auto nodeAttributes = attributes();
    if (!nodeAttributes) host::typeError("Cannot read hasOneOf on null MML attributes");
    return !nodeAttributes->hasOneOf(NONSPACELIKE());
}
}
