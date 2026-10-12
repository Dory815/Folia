/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/mroot.ts methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mroot;
import folia.math.core.mml_tree.arithmetic_host;
import std;

namespace folia::math::mml {

NodeValue MmlMroot::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    host::arithmeticChild(*this, 0, "setTeXclass").setTeXclass(PropertyNull{});
    host::arithmeticChild(*this, 1, "setTeXclass").setTeXclass(PropertyNull{});
    return this;
}

void MmlMroot::setChildInheritedAttributes(AttributeList attributes,
                                           Property display, Property level,
                                           Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    host::arithmeticChild(*this, 0, "setInheritedAttributes")
        .setInheritedAttributes(attributes, display, level, true);
    host::arithmeticChild(*this, 1, "setInheritedAttributes")
        .setInheritedAttributes(attributes, false, host::add(level, Property(2)), prime);
}

}
