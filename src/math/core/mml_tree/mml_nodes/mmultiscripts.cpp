/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MmlNodes/mmultiscripts.ts (MathJax 4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA-256: bb8d8d2180ef483b86b0b1570b919ba424ed54db1f30ef6e0e697cfab0bf5cf7
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mmultiscripts;
import folia.math.core.mml_tree.multiscript_host;
import std;

namespace folia::math::mml {

void MmlMmultiscripts::setChildInheritedAttributes(
    AttributeList attributes, Property display, Property level, Property prime) {
    // P23f Root stub: MmlMmultiscripts::setChildInheritedAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void MmlMmultiscripts::verifyChildren(PropertyList options) {
    // P23f Root stub: MmlMmultiscripts::verifyChildren.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void MmlMprescripts::verifyTree(std::optional<PropertyList> options) {
    // P23f Root stub: MmlMprescripts::verifyTree.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void MmlNone::verifyTree(std::optional<PropertyList> options) {
    // P23f Root stub: MmlNone::verifyTree.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

} // namespace folia::math::mml
