/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/mo.ts class methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mo_host;
import folia.math.core.mml_tree.operator_class;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.munderover;
import std;

namespace folia::math::mml {

MmlNode* MmlMo::coreParent() {
    // P23f Root stub: MmlMo::coreParent.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

std::u16string MmlMo::coreText(NodeValue parent) {
    // P23f Root stub: MmlMo::coreText.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

Property MmlMo::isAccent() {
    // P23f Root stub: MmlMo::isAccent.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

NodeValue MmlMo::setTeXclass(NodeValue prev) {
    // P23f Root stub: MmlMo::setTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

NodeValue MmlMo::adjustTeXclass(NodeValue prev) {
    // P23f Root stub: MmlMo::adjustTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

}
