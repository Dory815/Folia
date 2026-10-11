/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MmlNodes/mpadded.ts (MathJax 4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA-256: b5aac516c32fb019ac5ace504f86312f3e0849e1faa44006567bac05815c7271
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mpadded;
import folia.math.core.mml_tree.arithmetic_host;
import std;

namespace folia::math::mml {

MmlNode* MmlMpadded::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;

    if (!host::truthy(getProperty("vbox")))
        return AbstractMmlLayoutNode::setTeXclass(previous);

    getPrevClass(previous);
    setTexClass(TEXCLASS.ORD);
    host::arithmeticChild(*this, 0).setTeXclass(nullptr);
    return this;
}

} // namespace folia::math::mml
