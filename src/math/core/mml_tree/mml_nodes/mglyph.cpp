/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mglyph.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mglyph;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
MmlMglyph::MmlMglyph(MmlDocument &document) : AbstractMmlTokenNode(document, NodeKind::mglyph) {}

AttributeSet MmlMglyph::defaults() {
    return nodeDefaultAttributes(NodeKind::mglyph);
}

NodeKind MmlMglyph::kind() const noexcept {
    return NodeKind::mglyph;
}

void MmlMglyph::verifyAttributes(VerifyOptions const &options) {
    const auto *srcValue = attributes().lookup(u"src");
    const auto *fontValue = attributes().lookup(u"fontfamily");
    const auto *indexValue = attributes().lookup(u"index");
    const auto isEmpty = [](const AttributeValue *value) {
        const auto *text = value ? std::get_if<std::u16string>(value) : nullptr;
        return !text || text->empty();
    };
    if (isEmpty(srcValue) && (isEmpty(fontValue) || isEmpty(indexValue))) {
        (void)mError(u"mglyph must have either src or fontfamily and index attributes", options,
                     true);
    } else {
        AbstractMmlTokenNode::verifyAttributes(options);
    }
}
} // namespace folia::math::mml
