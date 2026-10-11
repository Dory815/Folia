/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/mtr.ts methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mtr;
import folia.math.core.mml_tree.table_host;
import std;

namespace folia::math::mml {

void MmlMtr::setChildInheritedAttributes(AttributeList attributes, Property display,
                                         Property level, Property prime) {
    // P23f Root stub: MmlMtr::setChildInheritedAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void MmlMtr::verifyChildren(PropertyList options) {
    // P23f Root stub: MmlMtr::verifyChildren.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

NodeValue MmlMtr::setTeXclass(NodeValue previous) {
    // P23f Root stub: MmlMtr::setTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

}
