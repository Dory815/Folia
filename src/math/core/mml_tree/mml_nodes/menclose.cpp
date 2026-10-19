/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MmlNodes/menclose.ts (MathJax 4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA-256: ffa4ab507744cb4f190312ca8dcd692474c333bee6e19c86709554aa9dad4333
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.menclose;
import folia.math.core.mml_tree.arithmetic_host;
import std;

namespace folia::math::mml {

NodeValue MmlMenclose::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    previous = host::arithmeticChild(*this, 0, "setTeXclass").setTeXclass(previous);

    // The upstream method reads childNodes[0] again after the callback, so
    // retain the live-array receiver only for this fresh slot lookup.
    const auto children = this->childArrayOwner();
    if (!children) host::typeError("Cannot read properties of null (reading '0')");
    updateTeXclass(children->get(0));
    return previous;
}

} // namespace folia::math::mml
