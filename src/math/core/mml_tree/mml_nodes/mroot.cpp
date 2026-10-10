/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/mroot.ts methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mroot;
import folia.math.core.mml_tree.arithmetic_host;
import std;

namespace folia::math::mml {

MmlNode* MmlMroot::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    MmlNode& base = host::arithmeticChild(*this, 0);
    base.setTeXclass(nullptr);

    // Resolve child 1 after child 0's virtual call; that callback may have
    // replaced the source childNodes array.
    MmlNode& index = host::arithmeticChild(*this, 1);
    index.setTeXclass(nullptr);
    return this;
}

void MmlMroot::setChildInheritedAttributes(AttributeList attributes,
                                           Property display, Property level,
                                           Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    MmlNode& base = host::arithmeticChild(*this, 0);
    base.setInheritedAttributes(attributes, display, level, Property(true));

    // JS evaluates the child-1 receiver before evaluating `level + 2`.
    MmlNode& index = host::arithmeticChild(*this, 1);
    const Property indexLevel = host::add(level, Property(2));
    index.setInheritedAttributes(attributes, Property(false), indexLevel, prime);
}

}
