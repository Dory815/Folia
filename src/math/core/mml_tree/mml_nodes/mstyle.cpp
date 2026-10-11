/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/Mstyle.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mstyle;
import folia.math.core.mml_tree.leaf_host;
import std;

namespace folia::math::mml {

std::string MmlMstyle::kind() const { return "mstyle"; }

Property MmlMstyle::notParent() const {
    // P23f Root stub: MmlMstyle::notParent.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void MmlMstyle::inheritAttributes(AttributeList attributes, Property display,
                                  Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    host::inheritanceDefaults(attributes, display, level, prime);
    auto currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot set inherited displaystyle on null attributes");
    currentAttributes->setInherited("displaystyle", display);
    currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot set inherited scriptlevel on null attributes");
    currentAttributes->setInherited("scriptlevel", level);
    AbstractMmlNode::inheritAttributes(attributes, display, level, prime);
}

void MmlMstyle::setChildInheritedAttributes(AttributeList attributes, Property display,
                                            Property level, Property prime) {
    // P23f Root stub: MmlMstyle::setChildInheritedAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

}
