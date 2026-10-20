// The original NodeUtil owns the separation of MML attributes and properties.
module folia.math.input.tex.node_util;
import folia.math.core.mml_tree.mml_nodes.mo;
import std;

namespace folia::math::tex {
void NodeUtil::copyAttributes(mml::MmlNode const &source, mml::MmlNode &destination) {
    destination.attributes() = source.attributes();
    mml::mergeNodeProperties(source.properties(), destination.properties());
}
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
mml::AttributeValue const *NodeUtil::getMoAttribute(mml::MmlNode const &node,
                                                    std::u16string_view name) {
    if (!node.attributes().isSet(name))
        for (auto form : {u"infix", u"postfix", u"prefix"})
            if (auto op = getOp(node, form))
                for (auto const &[key, value] : op->properties)
                    if (key == name)
                        return &value;
    return node.attributes().lookup(name);
}
void NodeUtil::setProperties(mml::MmlNode &node, NodeDefinition definition) {
    auto &attributes = node.attributes();
    for (auto const &[name, value] : definition.attributes.ownEntries()) {
        auto &tex = node.properties().tex;
        if (name == u"texClass") {
            tex.explicit_tex_class = static_cast<mml::TexClass>(std::get<double>(value));
            node.setTexClass(tex.explicit_tex_class);
        } else if (name == u"movablelimits") {
            tex.movable_limits = std::get<bool>(value);
            if (node.kind() == mml::NodeKind::mo || node.kind() == mml::NodeKind::mstyle)
                attributes.set(name, value);
        } else if (name == u"inferred") {
            continue;
        } else if (name == u"autoOP")
            tex.auto_operator = std::get<bool>(value);
        else if (name == u"fnOP")
            tex.function_operator = std::get<bool>(value);
        else if (name == u"movesupsub")
            tex.move_sup_sub = std::get<bool>(value);
        else if (name == u"subsupOK")
            tex.sub_sup_allowed = std::get<bool>(value);
        else if (name == u"texprimestyle")
            tex.prime_style = std::get<bool>(value);
        else if (name == u"variantForm")
            tex.variant_form = std::get<bool>(value);
        else if (name == u"withDelims")
            tex.with_delimiters = std::get<bool>(value);
        else if (name == u"mathaccent")
            tex.math_accent = std::get<bool>(value);
        else if (name == u"open")
            tex.open = std::get<std::u16string>(value);
        else if (name == u"close")
            tex.close = std::get<std::u16string>(value);
        else if (name == u"useHeight")
            node.properties().table.use_height = std::get<bool>(value);
        else
            attributes.set(name, value);
    }
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
