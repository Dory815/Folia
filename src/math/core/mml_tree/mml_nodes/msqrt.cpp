/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/msqrt.ts methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.msqrt;
import folia.math.core.mml_tree.arithmetic_host;
import std;

namespace folia::math::mml {

NodeValue MmlMsqrt::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    host::arithmeticChild(*this,0,"setTeXclass").setTeXclass(PropertyNull{});
    return this;
}

void MmlMsqrt::setChildInheritedAttributes(AttributeList attributes, Property display,
                                           Property level, Property /*prime*/) {
    tree::TreeCallGuard<MmlNode> guard;
    host::arithmeticChild(*this,0,"setInheritedAttributes")
        .setInheritedAttributes(attributes,display,level,true);
}

}
