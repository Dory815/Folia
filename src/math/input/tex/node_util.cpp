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
}
std::unordered_set<std::string>& NodeUtil::attrs() {
    static std::unordered_set<std::string> names{
        "autoOP","fnOP","movesupsub","subsupOK","texprimestyle","useHeight",
        "variantForm","withDelims","mathaccent","open","close"};
    return names;
}
NodeUtil& NodeUtil::instance(){static NodeUtil service;return service;}
std::u16string NodeUtil::createEntity(std::u16string_view) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
mml::Children NodeUtil::getChildren(mml::NodeValue) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
std::u16string NodeUtil::getText(mml::NodeValue) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
void NodeUtil::appendChildren(mml::NodeValue,mml::Children) {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
void NodeUtil::setAttribute(mml::NodeValue,std::string_view,Property) {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
void NodeUtil::setProperty(mml::NodeValue,std::string,Property) {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
void NodeUtil::setProperties(mml::NodeValue,Property) {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
Property NodeUtil::getProperty(mml::NodeValue,std::string_view) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
Property NodeUtil::getAttribute(mml::NodeValue,std::string_view) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
void NodeUtil::removeAttribute(mml::NodeValue,std::string_view) {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
void NodeUtil::removeProperties(mml::NodeValue,std::span<const std::string>) {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
void NodeUtil::copyAttributes(mml::NodeValue,mml::NodeValue) {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
bool NodeUtil::isType(mml::NodeValue,std::string_view) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
bool NodeUtil::isEmbellished(mml::NodeValue) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
Property NodeUtil::getTexClass(mml::NodeValue) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
mml::NodeValue NodeUtil::getCoreMO(mml::NodeValue) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
bool NodeUtil::isNode(const std::any&) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
bool NodeUtil::isInferred(mml::NodeValue) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
Property NodeUtil::getForm(mml::NodeValue) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
Property NodeUtil::getOp(mml::NodeValue,Property) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
Property NodeUtil::getMoAttribute(mml::NodeValue,std::string_view) const {throw mml::Error("Unimplemented",u"Root-frozen NodeUtil operation");}
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
