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
    // P23f Root stub: MmlMroot::setTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void MmlMroot::setChildInheritedAttributes(AttributeList attributes,
                                           Property display, Property level,
                                           Property prime) {
    // P23f Root stub: MmlMroot::setChildInheritedAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

}
