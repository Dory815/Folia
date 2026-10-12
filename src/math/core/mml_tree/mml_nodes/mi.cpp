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

void MmlMi::inheritAttributes(
    AttributeList attributes, Property display, Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    host::inheritanceDefaults(attributes, display, level, prime);
    AbstractMmlTokenNode::inheritAttributes(attributes, display, level, prime);
    const std::u16string text = getText();
    Pattern& pattern = singleCharacter();
    if (!pattern) host::typeError("MmlMi.singleCharacter has no native callback");
    const bool single = pattern(text);
    if (single && !host::truthy(host::get(attributes, "mathvariant"))) {
        auto nodeAttributes = this->attributes();
        if (!nodeAttributes) host::typeError("Cannot read setInherited on null MML attributes");
        nodeAttributes->setInherited("mathvariant", std::string("italic"));
    }
}

NodeValue MmlMi::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    const std::u16string name = getText();
    if (name.size() > 1) {
        Pattern& pattern = operatorName();
        if (!pattern) host::typeError("MmlMi.operatorName has no native callback");
        if (pattern(name)) {
            const auto currentAttributes = attributes();
            if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'get')");
            if (currentAttributes->get("mathvariant") == Property(std::string("normal"))) {
                const Property autoOP = getProperty("autoOP");
                if (autoOP.missing()) {
                    const Property texClassProperty = getProperty("texClass");
                    if (texClassProperty.missing()) {
                        setTexClass(static_cast<double>(TEXCLASS.OP));
                        setProperty("autoOP", true);
                    }
                }
            }
        }
    }
    return this;
}
}
