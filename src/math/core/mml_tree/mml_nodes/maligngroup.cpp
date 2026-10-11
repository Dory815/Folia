/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/maligngroup.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * TS SHA256: 4d8ecdae93aaff85887ec4eb09c8821811866a9e9b56e037ac61d0b6d81bf2b1
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.maligngroup;
import folia.math.core.mml_tree.mml_node_host;
import std;

namespace folia::math::mml {

void MmlMaligngroup::setChildInheritedAttributes(
    AttributeList attributes, Property display, Property level, Property prime) {
    auto currentAttributes = this->attributes();
    if (!currentAttributes) {
        host::typeError("Cannot read properties of null (reading 'getAllAttributes')");
    }
    PropertyList allAttributes = currentAttributes->getAllAttributes();
    AttributeList inherited = addInheritedAttributes(attributes, allAttributes);
    AbstractMmlLayoutNode::setChildInheritedAttributes(
        inherited, display, level, prime);
}

} // namespace folia::math::mml
