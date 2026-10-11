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

MmlNode* childAt(const Children& children, std::size_t index, std::string_view message) {
    if (!children) host::typeError(message);
    return index < children->size() ? (*children)[index] : nullptr;
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

MmlNode* MmlMfenced::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);

    if (open) previous = open->setTeXclass(previous);

    // Each indexed read below uses the current array handle, as the source
    // method accesses this.childNodes afresh around virtual callbacks.
    if (MmlNode* const first = childAt(childNodes, 0, "Cannot read properties of null (reading '0')"))
        previous = first->setTeXclass(previous);

    const Children countedChildren = childNodes;
    if (!countedChildren) host::typeError("Cannot read properties of null (reading 'length')");
    const std::size_t count = countedChildren->size();
    for (std::size_t index = 1; index < count; ++index) {
        if (!separators) host::typeError("Cannot read properties of null (reading '0')");
        MmlNode* const separator = index - 1 < separators->size()
            ? (*separators)[index - 1] : nullptr;
        if (separator) previous = separator->setTeXclass(previous);

        MmlNode* const child = childAt(childNodes, index, "Cannot read properties of null (reading '1')");
        if (child) previous = child->setTeXclass(previous);
    }

    if (close) previous = close->setTeXclass(previous);
    if (!open || !close) {
        MmlNode* first = nullptr;
        if (open) first = open;
        else first = childAt(childNodes, 0, "Cannot read properties of null (reading '0')");
        if (!first) first = close;
        updateTeXclass(first);
    }
    return previous;
}

void MmlMfenced::setChildInheritedAttributes(AttributeList attributes, Property display,
                                             Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    addFakeNodes();

    // JavaScript's array literal and concat produce a snapshot before the
    // first inherited-attribute callback runs.
    std::vector<MmlNode*> fakeChildren{open, close};
    const Children separatorSnapshot = separators;
    if (separatorSnapshot) {
        fakeChildren.insert(fakeChildren.end(), separatorSnapshot->begin(), separatorSnapshot->end());
    } else {
        fakeChildren.push_back(nullptr); // concat(null) appends one null value.
    }
    for (MmlNode* child : fakeChildren) {
        if (child) child->setInheritedAttributes(attributes, display, level, prime);
    }
    AbstractMmlNode::setChildInheritedAttributes(
        std::move(attributes), std::move(display), std::move(level), std::move(prime));
}

void MmlMfenced::addFakeNodes() {
    tree::TreeCallGuard<MmlNode> guard;
    const auto currentAttributes = attributes();
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'getList')");
    const PropertyList values = currentAttributes->getList({"open", "close", "separators"});

    // All three native String.replace calls happen before the first factory
    // call, and only the four exact ASCII whitespace code units are removed.
    std::u16string openText = nativeStringReplaceTarget(values.get("open"));
    std::u16string closeText = nativeStringReplaceTarget(values.get("close"));
    std::u16string separatorText = nativeStringReplaceTarget(values.get("separators"));
    removeSourceWhitespace(openText);
    removeSourceWhitespace(closeText);
    removeSourceWhitespace(separatorText);

    if (!openText.empty()) {
        open = fakeNode(std::move(openText), Property(PropertyList{{"fence", true}, {"form", "prefix"}}),
                        Property(static_cast<double>(TEXCLASS.OPEN)));
    }

    if (!separatorText.empty()) {
        // The source reevaluates this.childNodes.length for every while test.
        for (;;) {
            const Children currentChildren = childNodes;
            if (!currentChildren)
                host::typeError("Cannot read properties of null (reading 'length')");
            const std::size_t required = currentChildren->empty() ? 0 : currentChildren->size() - 1;
            if (separatorText.size() >= required) break;
            separatorText.push_back(separatorText.back());
        }

        const Children currentChildren = childNodes;
        if (!currentChildren) host::typeError("Cannot read properties of null (reading 'slice')");
        std::vector<MmlNode*> childrenAfterFirst;
        if (currentChildren->size() > 1)
            childrenAfterFirst.assign(currentChildren->begin() + 1, currentChildren->end());

        std::size_t separatorIndex = 0;
        for (MmlNode* child : childrenAfterFirst) {
            if (!child) continue;
            const Children pushReceiver = separators;
            if (!pushReceiver) host::typeError("Cannot read properties of null (reading 'push')");
            const std::u16string separatorTextValue = separatorIndex < separatorText.size()
                ? std::u16string(1, separatorText[separatorIndex]) : std::u16string{};
            ++separatorIndex;
            MmlNode* const fake = fakeNode(separatorTextValue, Missing{}, Missing{});
            pushReceiver->push_back(fake);
        }
    }

    if (!closeText.empty()) {
        close = fakeNode(std::move(closeText), Property(PropertyList{{"fence", true}, {"form", "postfix"}}),
                         Property(static_cast<double>(TEXCLASS.CLOSE)));
    }
}

MmlNode* MmlMfenced::fakeNode(std::u16string character, Property properties, Property texClass) {
    tree::TreeCallGuard<MmlNode> guard;
    if (properties.missing()) properties = PropertyList{};
    if (texClass.missing()) texClass = PropertyNull{};

    const std::array<std::any, 0> noArguments{};
    MmlNode* const textProduct = factory.create("text", noArguments);
    auto* const textNode = dynamic_cast<TextNode*>(textProduct);
    if (!textNode) host::typeError("Factory product has no TextNode setText method");
    TextNode* const textResult = textNode->setText(std::move(character));

    std::any nodeProperties;
    if (auto record = PropertyList::record(properties)) nodeProperties = *record;
    else nodeProperties = properties;
    auto textChildren = std::make_shared<std::vector<MmlNode*>>();
    textChildren->push_back(textResult);
    const std::array<std::any, 2> arguments{
        std::move(nodeProperties), std::any(Children(textChildren))};
    MmlNode* const node = factory.create("mo", tree::FactoryArgs(arguments));
    if (!node) host::typeError("Cannot set properties of null (setting 'texClass')");
    node->setTexClass(std::move(texClass));
    node->parent = this;
    return node;
}

} // namespace folia::math::mml
