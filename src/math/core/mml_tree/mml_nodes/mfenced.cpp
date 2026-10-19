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

std::u16string nativeStringReplaceTarget(const Property& value, std::string_view name) {
    const auto* bytes = std::get_if<std::string>(&value.value);
    if (!bytes) {
        if (value.missing()) host::typeError("Cannot read properties of undefined (reading 'replace')");
        if (std::holds_alternative<PropertyNull>(value.value))
            host::typeError("Cannot read properties of null (reading 'replace')");
        host::typeError(std::string(name)+".replace is not a function");
    }
    return host::text(*bytes);
}

void removeSourceWhitespace(std::u16string& text) {
    std::erase_if(text, [](char16_t c) {
        return c == u' ' || c == u'\t' || c == u'\n' || c == u'\r';
    });
}

} // namespace

NodeValue MmlMfenced::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    if (open) previous = open->setTeXclass(previous);
    const NodeValue first = childAt(childNodes, 0);
    if (first.isNode()) previous = first.requireNode("setTeXclass")->setTeXclass(previous);

    const Children loopChildren = childNodes;
    if (!loopChildren) host::typeError("Cannot read properties of null (reading 'length')");
    const std::size_t count = loopChildren->size();
    for (std::size_t i = 1; i < count; ++i) {
        const NodeValue separator = childAt(separators, i - 1);
        if (separator.isNode()) previous = separator.requireNode("setTeXclass")->setTeXclass(previous);
        const NodeValue child = childAt(childNodes, i);
        if (child.isNode()) previous = child.requireNode("setTeXclass")->setTeXclass(previous);
    }
    if (close) previous = close->setTeXclass(previous);

    if (!open || !close) {
        NodeValue updateTarget;
        if (open) {
            updateTarget = NodeValue(open);
        } else {
            const NodeValue currentFirst = childAt(childNodes, 0);
            updateTarget = currentFirst.isNode() ? currentFirst : NodeValue(close);
        }
        updateTeXclass(updateTarget);
    }
    return previous;
}

void MmlMfenced::setChildInheritedAttributes(AttributeList attributes, Property display,
                                             Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    addFakeNodes();

    auto fakeSnapshot = std::make_shared<tree::ChildArray<MmlNode>>();
    fakeSnapshot->resize(2);
    fakeSnapshot->set(0, NodeValue(open));
    fakeSnapshot->set(1, NodeValue(close));
    const Children separatorSnapshot = separators;
    if (!separatorSnapshot) {
        fakeSnapshot->push_back(PropertyNull{});
    } else {
        const std::size_t length = separatorSnapshot->size();
        fakeSnapshot->resize(2 + length);
        for (std::size_t i = 0; i < length; ++i) {
            if (separatorSnapshot->has(i)) fakeSnapshot->set(2 + i, separatorSnapshot->get(i));
        }
    }
    for (std::size_t i = 0; i < fakeSnapshot->size(); ++i) {
        const NodeValue child = fakeSnapshot->get(i);
        if (child.isNode()) child.requireNode("setInheritedAttributes")->setInheritedAttributes(attributes, display, level, prime);
    }
    AbstractMmlNode::setChildInheritedAttributes(attributes, display, level, prime);
}

void MmlMfenced::addFakeNodes() {
    tree::TreeCallGuard<MmlNode> guard;
    const auto currentAttributes = attributes();
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'getList')");
    const auto values = currentAttributes->getList({"open", "close", "separators"});
    const Property rawOpen = values.get("open");
    const Property rawClose = values.get("close");
    const Property rawSeparators = values.get("separators");
    std::u16string openText = nativeStringReplaceTarget(rawOpen, "open");
    removeSourceWhitespace(openText);
    std::u16string closeText = nativeStringReplaceTarget(rawClose, "close");
    removeSourceWhitespace(closeText);
    std::u16string separatorText = nativeStringReplaceTarget(rawSeparators, "separators");
    removeSourceWhitespace(separatorText);

    if (!openText.empty()) {
        open = fakeNode(openText, PropertyList{{"fence", true}, {"form", "prefix"}}, TEXCLASS.OPEN);
    }

    if (!separatorText.empty()) {
        while (true) {
            const Children currentChildren = childNodes;
            if (!currentChildren) host::typeError("Cannot read properties of null (reading 'length')");
            const double requiredLength = static_cast<double>(currentChildren->size()) - 1.0;
            if (!(static_cast<double>(separatorText.size()) < requiredLength)) break;
            separatorText.push_back(separatorText.back());
        }
        const Children currentChildren = childNodes;
        if (!currentChildren) host::typeError("Cannot read properties of null (reading 'slice')");
        const Children slicedChildren = currentChildren->slice(1);
        std::size_t index = 0;
        for (std::size_t i = 0; i < slicedChildren->size(); ++i) {
            const NodeValue child = slicedChildren->get(i);
            if (!child.isNode()) continue;

            const Children pushReceiver = separators;
            if (!pushReceiver) host::typeError("Cannot read properties of null (reading 'push')");
            std::u16string separator;
            if (index < separatorText.size()) separator.push_back(separatorText[index]);
            ++index;
            MmlNode* const fake = fakeNode(separator);
            pushReceiver->push_back(NodeValue(fake));
        }
    }

    if (!closeText.empty()) {
        close = fakeNode(closeText, PropertyList{{"fence", true}, {"form", "postfix"}}, TEXCLASS.CLOSE);
    }
}

MmlNode* MmlMfenced::fakeNode(std::u16string character, Property properties, Property texClass) {
    tree::TreeCallGuard<MmlNode> guard;
    if (properties.missing()) properties = PropertyList{};
    if (texClass.missing()) texClass = PropertyNull{};

    TextNode* const text = requiredText(this->factoryOwner().create("text", {})).setText(std::move(character));
    auto children = std::make_shared<tree::ChildArray<MmlNode>>();
    children->push_back(NodeValue(text));
    std::any propertiesArgument;
    if (auto object = PropertyList::object(properties)) propertiesArgument = *object;
    else propertiesArgument = properties;
    const std::array<std::any, 2> arguments{
        std::move(propertiesArgument), std::any(Children(children))
    };
    MmlNode* const node = this->factoryOwner().create("mo", tree::FactoryArgs(arguments));
    NodeValue(node).requireNodeForWrite("texClass")->setTexClass(texClass);
    NodeValue(node).requireNodeForWrite("parent")->setParent(NodeValue(static_cast<MmlNode*>(this)));
    return node;
}

} // namespace folia::math::mml
