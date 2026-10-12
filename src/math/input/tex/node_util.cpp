module folia.math.input.tex.node_util;
import folia.math.core.mml_tree.mml_node_host;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.util.string;
import folia.math.util.js_numeric_input;
import std;
namespace folia::math::tex {
namespace {
namespace host=mml::host;
std::shared_ptr<Attributes> requiredAttributes(mml::NodeValue node,std::string_view member) {
    const auto attributes=node.requireNode("attributes")->attributes();
    if(!attributes)host::typeError("Cannot read properties of null (reading '"+std::string(member)+"')");
    return attributes;
}
std::u16string nodeText(mml::NodeValue node,std::string_view reference) {
    auto* receiver=node.requireNode("getText");
    if(auto* text=dynamic_cast<mml::TextNode*>(receiver))return text->getText();
    if(auto* token=dynamic_cast<mml::AbstractMmlTokenNode*>(receiver))return token->getText();
    host::typeError(std::string(reference)+".getText is not a function");
}
mml::MmlMo& formsReceiver(mml::NodeValue node) {
    auto* receiver=node.requireNode("getForms");
    if(auto* mo=dynamic_cast<mml::MmlMo*>(receiver))return *mo;
    host::typeError("mo.getForms is not a function");
}
mml::MmlNode* canonicalNode(const std::any& value) {
    if(auto* slot=std::any_cast<mml::NodeValue>(&value))return slot->nodeOrNull();
    if(auto* pointer=std::any_cast<mml::MmlNode*>(&value))return *pointer;
    return nullptr;
}
Property propertyAt(const Property& receiver,std::string_view key) {
    if(host::nullish(receiver))host::typeError(
        std::string("Cannot read properties of ")+(receiver.missing()?"undefined":"null")+
        " (reading '"+std::string(key)+"')");
    return host::get(receiver,key);
}
}
std::unordered_set<std::string>& NodeUtil::attrs() {
    static std::unordered_set<std::string> names{
        "autoOP","fnOP","movesupsub","subsupOK","texprimestyle","useHeight",
        "variantForm","withDelims","mathaccent","open","close"};
    return names;
}
NodeUtil& NodeUtil::instance(){static NodeUtil service;return service;}
std::u16string NodeUtil::createEntity(std::u16string_view code) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    const std::array<double, 1> codePoint{
        js_input::parseInt(code, js_input::IntegerRadix::Hex)};
    return string_util::unicodeString(codePoint);
}
mml::Children NodeUtil::getChildren(mml::NodeValue node) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    return node.requireNode("childNodes")->childNodes;
}
std::u16string NodeUtil::getText(mml::NodeValue node) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    return nodeText(node, "node");
}
void NodeUtil::appendChildren(mml::NodeValue node,mml::Children children) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    if (!children) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for (std::size_t index = 0; index < children->size(); ++index) {
        const mml::NodeValue child = children->get(index);
        node.requireNode("appendChild")->appendChild(child);
    }
}
void NodeUtil::setAttribute(mml::NodeValue node,std::string_view attribute,Property value) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    requiredAttributes(node, "set")->set(attribute, std::move(value));
}
void NodeUtil::setProperty(mml::NodeValue node,std::string property,Property value) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    node.requireNode("setProperty")->setProperty(property, std::move(value));
}
void NodeUtil::setProperties(mml::NodeValue node,Property properties) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    const std::vector<std::string> names = host::ownKeys(properties);
    for (const std::string& name : names) {
        const Property value = host::get(properties, name);
        if (name == "texClass") {
            node.requireNodeForWrite("texClass")->setTexClass(value);
            node.requireNode("setProperty")->setProperty(name, value);
        } else if (name == "movablelimits") {
            node.requireNode("setProperty")->setProperty(name, value);
            if (node.requireNode("isKind")->isKind("mo") ||
                node.requireNode("isKind")->isKind("mstyle")) {
                requiredAttributes(node, "set")->set("movablelimits", value);
            }
        } else if (name == "inferred") {
            continue;
        } else if (NodeUtil::attrs().contains(name)) {
            node.requireNode("setProperty")->setProperty(name, value);
        } else {
            requiredAttributes(node, "set")->set(name, value);
        }
    }
}
Property NodeUtil::getProperty(mml::NodeValue node,std::string_view property) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    return node.requireNode("getProperty")->getProperty(property);
}
Property NodeUtil::getAttribute(mml::NodeValue node,std::string_view attribute) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    return requiredAttributes(node, "get")->get(attribute);
}
void NodeUtil::removeAttribute(mml::NodeValue node,std::string_view attribute) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    requiredAttributes(node, "unset")->unset(attribute);
}
void NodeUtil::removeProperties(mml::NodeValue node,std::span<const std::string> properties) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    node.requireNode("removeProperty")->removeProperty(properties);
}
void NodeUtil::copyAttributes(mml::NodeValue oldNode,mml::NodeValue newNode) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    const auto attributes = oldNode.requireNode("attributes")->attributes();
    newNode.requireNodeForWrite("attributes")->setAttributes(attributes);

    const Property properties(oldNode.requireNode("getAllProperties")->getAllProperties());
    const std::vector<std::string> names = host::ownKeys(properties);
    std::vector<std::pair<std::string, Property>> entries;
    entries.reserve(names.size());
    for (const std::string& name : names) {
        entries.emplace_back(name, host::get(properties, name));
    }
    for (const auto& [name, value] : entries) {
        newNode.requireNode("setProperty")->setProperty(name, value);
    }
}
bool NodeUtil::isType(mml::NodeValue node,std::string_view kind) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    return node.requireNode("isKind")->isKind(kind);
}
bool NodeUtil::isEmbellished(mml::NodeValue node) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    return node.requireNode("isEmbellished")->isEmbellished();
}
Property NodeUtil::getTexClass(mml::NodeValue node) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    return node.requireNode("texClass")->texClass();
}
mml::NodeValue NodeUtil::getCoreMO(mml::NodeValue node) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    return node.requireNode("coreMO")->coreMO();
}
bool NodeUtil::isNode(const std::any& value) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    auto* const node = canonicalNode(value);
    return node && (dynamic_cast<mml::AbstractMmlNode*>(node) ||
                    dynamic_cast<mml::AbstractMmlEmptyNode*>(node));
}
bool NodeUtil::isInferred(mml::NodeValue node) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    return node.requireNode("isInferred")->isInferred();
}
Property NodeUtil::getForm(mml::NodeValue node) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    if (!node.requireNode("isKind")->isKind("mo")) return Property(PropertyNull{});
    const PropertyArray forms = formsReceiver(node).getForms();
    for (std::size_t index = 0; index < forms.size(); ++index) {
        const Property definition = this->getOp(node, forms.get(index));
        if (host::truthy(definition)) return definition;
    }
    return Property(PropertyNull{});
}
Property NodeUtil::getOp(mml::NodeValue node,Property form) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    if (form.missing()) form = Property("infix");
    const Property operatorTable = mml::MmlMo::operatorTable();
    const Property formTable = propertyAt(operatorTable, host::debugText(host::string(form)));
    const std::u16string text = nodeText(node, "mo");
    const Property definition = propertyAt(formTable, host::debugText(text));
    return host::truthy(definition) ? definition : Property(PropertyNull{});
}
Property NodeUtil::getMoAttribute(mml::NodeValue node,std::string_view attribute) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    if (!requiredAttributes(node, "isSet")->isSet(attribute)) {
        for (const std::string_view form : {"infix", "postfix", "prefix"}) {
            const Property definition = this->getOp(node, Property(std::string(form)));
            if (host::nullish(definition)) continue;
            const Property tuple = host::get(definition, "3");
            if (host::nullish(tuple)) continue;
            const Property value = host::get(tuple, attribute);
            if (!value.missing()) return value;
        }
    }
    return requiredAttributes(node, "get")->get(attribute);
}
mml::NodeValue NodeUtil::getChildAt(mml::NodeValue node,std::size_t position) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    auto* receiver = node.requireNode("childNodes");
    const auto children = receiver->childNodes;
    if (!children) {
        const std::string message = "Cannot read properties of null (reading '" +
                                   std::to_string(position) + "')";
        throw mml::Error("TypeError", std::u16string(message.begin(), message.end()));
    }
    return children->get(position);
}
void NodeUtil::setChild(mml::NodeValue node,std::size_t position,mml::NodeValue child) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    auto* receiver = node.requireNode("childNodes");
    const auto children = receiver->childNodes;
    if (!children) {
        const std::string message = "Cannot set properties of null (setting '" +
                                   std::to_string(position) + "')";
        throw mml::Error("TypeError", std::u16string(message.begin(), message.end()));
    }
    children->set(position, child);
    if (child.isNode()) {
        child.requireNodeForWrite("parent")->setParent(node);
    }
}
void NodeUtil::copyChildren(mml::NodeValue oldNode,mml::NodeValue newNode) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    auto* source = oldNode.requireNode("childNodes");
    const auto children = source->childNodes;
    if (!children) {
        throw mml::Error("TypeError",u"Cannot read properties of null (reading 'length')");
    }
    for (std::size_t index = 0; index < children->size(); ++index) {
        const mml::NodeValue child = children->get(index);
        this->setChild(newNode, index, child);
    }
}
}
