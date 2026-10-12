/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mfenced.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Upstream source SHA256: 64b3f4d89d47765eb83034b480b843b2769d8de12e31d7571c50e5c4bb2bd397
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mfenced;
import folia.math.core.mml_tree.mml_node_host;
import std;

namespace folia::math::mml {
namespace {

NodeValue childAt(const Children& children, std::size_t index) {
    if (!children) host::typeError("Cannot read properties of null (reading '"+
                                  std::to_string(index)+"')");
    return children->get(index);
}

TextNode& requiredText(MmlNode* product) {
    auto* node = NodeValue(product).requireNode("setText");
    if (auto* text = dynamic_cast<TextNode*>(node)) return *text;
    host::typeError("this.factory.create(...).setText is not a function");
}

std::u16string nativeStringReplaceTarget(const Property& value) {
    const auto* bytes = std::get_if<std::string>(&value.value);
    if (!bytes) host::typeError("value.replace is not a function");
    return host::text(*bytes);
}

void removeSourceWhitespace(std::u16string& text) {
    std::erase_if(text, [](char16_t c) {
        return c == u' ' || c == u'\t' || c == u'\n' || c == u'\r';
    });
}

} // namespace

NodeValue MmlMfenced::setTeXclass(NodeValue previous) {
    // P23f Root stub: MmlMfenced::setTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void MmlMfenced::setChildInheritedAttributes(AttributeList attributes, Property display,
                                             Property level, Property prime) {
    // P23f Root stub: MmlMfenced::setChildInheritedAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void MmlMfenced::addFakeNodes() {
    // P23f Root stub: MmlMfenced::addFakeNodes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

MmlNode* MmlMfenced::fakeNode(std::u16string character, Property properties, Property texClass) {
    // P23f Root stub: MmlMfenced::fakeNode.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

} // namespace folia::math::mml
