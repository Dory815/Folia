// Root-frozen primitive operations for MathJax 4.1.3 multiscript semantics.
export module folia.math.core.mml_tree.multiscript_host;
export import folia.math.core.mml_tree.arithmetic_host;
import std;
export namespace folia::math::mml::host {
inline MmlNode& multiscriptNode(MmlNode* node) {
    if (!node) typeError("Cannot read required multiscript node");
    return *node;
}
inline Children multiscriptChildren(MmlNode& node) {
    auto children = node.childNodes;
    if (!children) typeError("Cannot read null multiscript child array");
    return children;
}
inline MmlNode* multiscriptSlot(const Children& children, std::size_t index) {
    if (!children) typeError("Cannot read null multiscript child array");
    return index < children->size() ? (*children)[index] : nullptr;
}
inline MmlNode& multiscriptChild(MmlNode& node, std::size_t index) {
    return multiscriptNode(multiscriptSlot(multiscriptChildren(node), index));
}
inline void multiscriptInsert(const Children& children, std::size_t index, MmlNode* node) {
    if (!children) typeError("Cannot splice null multiscript child array");
    // Source splice(i, 0, node) clamps positive i to the current array length.
    // Parent assignment is a separate, later source operation.
    children->insert(children->begin() + std::min(index, children->size()), node);
}
inline MmlNode* multiscriptCreate(Factory& factory, std::string_view kind) {
    // Retain a possibly null returned value until the source dereferences it.
    return factory.create(kind, {});
}
inline ErrorOptions multiscriptErrorOptions(const std::optional<PropertyList>& options) {
    return options ? ErrorOptions(*options) : ErrorOptions(nullptr);
}
}
