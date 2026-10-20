// The original NodeUtil owns the separation of MML attributes and properties.
module folia.math.input.tex.node_util;
import folia.math.core.mml_tree.mml_nodes.mo;
import std;

namespace folia::math::tex {
void NodeUtil::setChild(mml::MmlNode &parent, std::size_t index, mml::MmlNode *child) {
    parent.document().setChildSlot(parent, index, child);
}
std::u16string NodeUtil::getText(mml::MmlNode const &node) {
    if (auto text = dynamic_cast<mml::TextNode const *>(&node))
        return text->getText();
    if (auto token = dynamic_cast<mml::AbstractMmlTokenNode const *>(&node))
        return token->getText();
    throw std::invalid_argument("MML node does not provide token text");
}
mml::operator_dict::OperatorDef const *NodeUtil::getOp(mml::MmlNode const &node,
                                                       std::u16string_view form) {
    return mml::operator_dict::findOperatorDefinition(form, getText(node));
}
mml::operator_dict::OperatorDef const *NodeUtil::getForm(mml::MmlNode &node) {
    if (auto mo = dynamic_cast<mml::MmlMo *>(&node))
        for (auto const &form : mo->getForms())
            if (auto def = getOp(node, form))
                return def;
    return nullptr;
}
void NodeUtil::setProperties(mml::MmlNode &node, NodeDefinition definition) {
    auto &attributes = node.attributes();
    for (auto const &[name, value] : definition.attributes.ownEntries())
        attributes.set(name, value);
    for (auto const &[name, value] : definition.attributes.inheritedEntries())
        attributes.setInherited(name, value);
    for (auto const &[name, value] : definition.attributes.defaultEntries())
        attributes.setDefault(name, value);
    for (auto const &[name, value] : definition.attributes.globalDefaultEntries())
        attributes.setGlobalDefault(name, value);
    mml::mergeNodeProperties(definition.properties, node.properties());
    if (definition.properties.tex.explicit_tex_class)
        node.setTexClass(definition.properties.tex.explicit_tex_class);
    if (definition.properties.tex.movable_limits &&
        (node.kind() == mml::NodeKind::mo || node.kind() == mml::NodeKind::mstyle))
        attributes.set(u"movablelimits", *definition.properties.tex.movable_limits);
}
} // namespace folia::math::tex
