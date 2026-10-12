// Root-frozen primitive operations for MathJax 4.1.3 multiscript semantics.
export module folia.math.core.mml_tree.multiscript_host;
export import folia.math.core.mml_tree.arithmetic_host;
import std;
export namespace folia::math::mml::host {
inline MmlNode& multiscriptNode(NodeValue, std::string_view) {
    throw Error("Unimplemented",u"Root-frozen MML multiscript receiver binding");
}
inline Children multiscriptChildren(MmlNode& node) {
    auto children = node.childNodes;
    if (!children) typeError("Cannot read null multiscript child array");
    return children;
}
inline NodeValue multiscriptSlot(const Children&, std::size_t) {
    throw Error("Unimplemented",u"Root-frozen MML multiscript indexed Get");
}
inline MmlNode& multiscriptChild(MmlNode&, std::size_t, std::string_view) {
    throw Error("Unimplemented",u"Root-frozen MML multiscript child member");
}
inline void multiscriptInsert(const Children&, std::size_t, NodeValue) {
    throw Error("Unimplemented",u"Root-frozen MML multiscript splice binding");
}
inline MmlNode* multiscriptCreate(Factory& factory, std::string_view kind) {
    // Retain a possibly null returned value until the source dereferences it.
    return factory.create(kind, {});
}
}
