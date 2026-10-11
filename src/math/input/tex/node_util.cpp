module folia.math.input.tex.node_util;
import std;
namespace folia::math::tex {
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
