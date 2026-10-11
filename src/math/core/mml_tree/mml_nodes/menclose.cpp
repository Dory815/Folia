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

MmlNode* MmlMenclose::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;

    MmlNode* const result = host::arithmeticChild(*this, 0).setTeXclass(previous);

    // The source re-evaluates this.childNodes[0] after the first child callback.
    // An empty array yields a nullable update argument; a null array still fails.
    const Children currentChildren = childNodes;
    if (!currentChildren) host::typeError("Cannot read child at index 0 from null children");
    MmlNode* const currentChild = currentChildren->empty()
        ? nullptr
        : (*currentChildren)[0];
    updateTeXclass(currentChild);
    return result;
}

} // namespace folia::math::mml
