// Root-frozen primitive operations for MathJax 4.1.3 multiscript semantics.
export module folia.math.core.mml_tree.multiscript_host;
export import folia.math.core.mml_tree.arithmetic_host;
import std;
export namespace folia::math::mml::host {
inline MmlNode& multiscriptNode(NodeValue value, std::string_view member) {
    return *value.requireNode(member);
}
inline Children multiscriptChildren(MmlNode& node) {
    auto children = node.childNodes;
    if (!children) typeError("Cannot read properties of null (reading 'length')");
    return children;
}
inline NodeValue multiscriptSlot(const Children& children, std::size_t index) {
    if (!children) typeError("Cannot read properties of null (reading '"+std::to_string(index)+"')");
    return children->get(index);
}
inline MmlNode& multiscriptChild(MmlNode& node, std::size_t index, std::string_view member) {
    return multiscriptNode(multiscriptSlot(node.childNodes,index),member);
}
inline void multiscriptInsert(const Children& children, std::size_t index, NodeValue value) {
    if (!children) typeError("Cannot read properties of null (reading 'splice')");
    if (index>static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max()))
        throw Error("RangeError",u"Native multiscript splice index exceeds int64_t");
    const std::array<NodeValue,1> values{value};
    children->splice(static_cast<std::int64_t>(index),0,values);
}
inline MmlNode* multiscriptCreate(Factory& factory, std::string_view kind) {
    // Retain a possibly null returned value until the source dereferences it.
    return factory.create(kind, {});
}
}
