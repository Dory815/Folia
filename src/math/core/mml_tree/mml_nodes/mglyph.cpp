/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/mglyph.ts methods.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mglyph;
import folia.math.core.mml_tree.leaf_host;
import std;

namespace folia::math::mml {

void MmlMglyph::verifyAttributes(PropertyList options) {
    const auto currentAttributes = attributes();
    if (!currentAttributes)
        host::typeError("Cannot read properties of null (reading 'getList')");

    const auto values = currentAttributes->getList({"src", "fontfamily", "index"});
    const Property src = values.get("src");
    const Property fontfamily = values.get("fontfamily");
    const Property index = values.get("index");
    const Property empty{std::string{}};

    if (src == empty && (fontfamily == empty || index == empty)) {
        (void)mError(
            "mglyph must have either src or fontfamily and index attributes",
            ErrorOptions(options), true);
    } else {
        AbstractMmlTokenNode::verifyAttributes(options);
    }
}

}
