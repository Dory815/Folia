/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MmlMfrac's arithmetic behavior from MathJax 4.1.3.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA256: 4ded5fc9922d462d02818523d676c30299d08cfe7794067bd4e20950c8fccfc7
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mfrac;
import folia.math.core.mml_tree.arithmetic_host;
import std;

namespace folia::math::mml {

NodeValue MmlMfrac::setTeXclass(NodeValue previous) {
    // P23f Root stub: MmlMfrac::setTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void MmlMfrac::setChildInheritedAttributes(AttributeList attributes, Property display,
                                            Property level, Property prime) {
    // P23f Root stub: MmlMfrac::setChildInheritedAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

} // namespace folia::math::mml
