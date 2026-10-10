/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mi.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.leaf_host;
import std;

namespace folia::math::mml {
void MmlMi::initializeClassFields() {
    initializeTexClass(static_cast<double>(TEXCLASS.ORD));
}

std::string MmlMi::kind() const {
    return "mi";
}

void MmlMi::setInheritedAttributes(
    AttributeList attributes, Property display, Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    host::inheritanceDefaults(display, level, prime);
    AbstractMmlTokenNode::setInheritedAttributes(attributes, display, level, prime);
    const std::u16string text = getText();
    Pattern& pattern = singleCharacter();
    if (!pattern) host::typeError("MmlMi.singleCharacter has no native callback");
    const bool single = pattern(text);
    if (single && !host::truthy(attributes.get("mathvariant"))) {
        auto nodeAttributes = this->attributes();
        if (!nodeAttributes) host::typeError("Cannot read setInherited on null MML attributes");
        nodeAttributes->setInherited("mathvariant", std::string("italic"));
    }
}

MmlNode* MmlMi::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    const std::u16string name = getText();
    if (name.size() > 1) {
        Pattern& pattern = operatorName();
        if (!pattern) host::typeError("MmlMi.operatorName has no native callback");
        const bool operatorNameMatches = pattern(name);
        if (operatorNameMatches) {
            auto nodeAttributes = attributes();
            if (!nodeAttributes) host::typeError("Cannot read mathvariant on null MML attributes");
            const Property variant = nodeAttributes->get("mathvariant");
            if (variant == Property(std::string("normal")) &&
                getProperty("autoOP").missing() && getProperty("texClass").missing()) {
                setTexClass(static_cast<double>(TEXCLASS.OP));
                setProperty("autoOP", Property(true));
            }
        }
    }
    return this;
}
}
