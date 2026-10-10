/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/msqrt.ts methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.msqrt;
import folia.math.core.mml_tree.arithmetic_host;
import std;

namespace folia::math::mml {

MmlNode* MmlMsqrt::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    MmlNode& child = host::arithmeticChild(*this, 0);
    child.setTeXclass(nullptr);
    return this;
}

void MmlMsqrt::setChildInheritedAttributes(AttributeList attributes, Property display,
                                           Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    (void)prime;
    MmlNode& child = host::arithmeticChild(*this, 0);
    child.setInheritedAttributes(std::move(attributes), std::move(display),
                                 std::move(level), Property(true));
}

}
