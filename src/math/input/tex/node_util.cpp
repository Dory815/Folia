// The original NodeUtil owns the separation of MML attributes and properties.
module folia.math.input.tex.node_util;
import std;

namespace folia::math::tex {
void NodeUtil::setProperties(mml::MmlNode& node, NodeDefinition definition) {
    auto& attributes = node.attributes();
    for (auto const& [name, value] : definition.attributes.ownEntries()) attributes.set(name, value);
    for (auto const& [name, value] : definition.attributes.inheritedEntries()) attributes.setInherited(name, value);
    for (auto const& [name, value] : definition.attributes.defaultEntries()) attributes.setDefault(name, value);
    for (auto const& [name, value] : definition.attributes.globalDefaultEntries()) attributes.setGlobalDefault(name, value);
    mml::mergeNodeProperties(definition.properties, node.properties());
    if (definition.properties.tex.explicit_tex_class)
        node.setTexClass(definition.properties.tex.explicit_tex_class);
    if (definition.properties.tex.movable_limits &&
        (node.kind() == mml::NodeKind::mo || node.kind() == mml::NodeKind::mstyle))
        attributes.set(u"movablelimits", *definition.properties.tex.movable_limits);
}
}
