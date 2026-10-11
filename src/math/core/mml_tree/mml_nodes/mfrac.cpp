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

MmlNode* MmlMfrac::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);

    // A JavaScript for-of loop keeps this array object but observes its live
    // length and indexed values on each iteration.
    const Children children = childNodes;
    if (!children) host::typeError("this.childNodes is not iterable");
    for (std::size_t index = 0; index < children->size(); ++index) {
        MmlNode* const child = (*children)[index];
        if (!child) host::typeError("Cannot read properties of null (reading 'setTeXclass')");
        child->setTeXclass(nullptr);
    }
    return this;
}

void MmlMfrac::setChildInheritedAttributes(AttributeList attributes, Property display,
                                            Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;

    // Preserve the source `if (!display || level > 0) level++` and JavaScript's
    // numeric conversion for both the relational comparison and increment.
    if (!host::truthy(display) || host::number(level) > 0) {
        level = Property(host::number(level) + 1.0);
    }

    const Property numalign = host::arithmeticAttributes(*this)->get("numalign");
    const Property denalign = host::arithmeticAttributes(*this)->get("denomalign");

    // Both new records are completed before either child callback can mutate
    // the fraction or its inherited-attribute input.
    AttributeList numAttributes = addInheritedAttributes(
        host::arithmeticCopy(attributes),
        PropertyList{{"numalign", numalign},
                     {"indentshift", "0"},
                     {"indentalignfirst", numalign},
                     {"indentshiftfirst", "0"},
                     {"indentalignlast", "indentalign"},
                     {"indentshiftlast", "indentshift"}});
    AttributeList denAttributes = addInheritedAttributes(
        host::arithmeticCopy(attributes),
        PropertyList{{"denalign", denalign},
                     {"indentshift", "0"},
                     {"indentalignfirst", denalign},
                     {"indentshiftfirst", "0"},
                     {"indentalignlast", "indentalign"},
                     {"indentshiftlast", "indentshift"}});

    host::arithmeticChild(*this, 0).setInheritedAttributes(
        std::move(numAttributes), Property(false), level, prime);
    host::arithmeticChild(*this, 1).setInheritedAttributes(
        std::move(denAttributes), Property(false), level, Property(true));
}

} // namespace folia::math::mml
