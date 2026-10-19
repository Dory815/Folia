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
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    const auto children = this->childArrayOwner();
    if (!children) {
        host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    }
    for (std::size_t index = 0; index < children->size(); ++index) {
        children->get(index).requireNode("setTeXclass")->setTeXclass(PropertyNull{});
    }
    return this;
}

void MmlMfrac::setChildInheritedAttributes(AttributeList attributes, Property display,
                                            Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;

    if (!host::truthy(display) || host::number(level) > 0.0) {
        level = Property(host::number(level) + 1.0);
    }

    const Property numalign = host::arithmeticAttributes(*this)->get("numalign");
    const Property denalign = host::arithmeticAttributes(*this)->get("denomalign");

    PropertyList numeratorAdditions;
    numeratorAdditions.setOwn("numalign", numalign);
    numeratorAdditions.setOwn("indentshift", "0");
    numeratorAdditions.setOwn("indentalignfirst", numalign);
    numeratorAdditions.setOwn("indentshiftfirst", "0");
    numeratorAdditions.setOwn("indentalignlast", "indentalign");
    numeratorAdditions.setOwn("indentshiftlast", "indentshift");
    const AttributeList numeratorAttributes = this->addInheritedAttributes(
        host::arithmeticCopy(attributes), Property(numeratorAdditions));

    PropertyList denominatorAdditions;
    denominatorAdditions.setOwn("denalign", denalign);
    denominatorAdditions.setOwn("indentshift", "0");
    denominatorAdditions.setOwn("indentalignfirst", denalign);
    denominatorAdditions.setOwn("indentshiftfirst", "0");
    denominatorAdditions.setOwn("indentalignlast", "indentalign");
    denominatorAdditions.setOwn("indentshiftlast", "indentshift");
    const AttributeList denominatorAttributes = this->addInheritedAttributes(
        host::arithmeticCopy(attributes), Property(denominatorAdditions));

    host::arithmeticChild(*this, 0, "setInheritedAttributes").setInheritedAttributes(
        numeratorAttributes, false, level, prime);
    host::arithmeticChild(*this, 1, "setInheritedAttributes").setInheritedAttributes(
        denominatorAttributes, false, level, true);
}

} // namespace folia::math::mml
