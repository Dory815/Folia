// Root bindings for source own-property spread and required child access.
export module folia.math.core.mml_tree.arithmetic_host;
export import folia.math.core.mml_tree.leaf_host;
import std;
export namespace folia::math::mml::host {
inline PropertyList arithmeticOwnCopy(const PropertyList& input) {
    PropertyList copy;
    for (const auto& key : input.keys()) copy.setOwn(key, input.getOwn(key));
    return copy;
}
inline MmlNode& arithmeticChild(MmlNode& node, std::size_t index) {
    const auto children = node.childNodes;
    if (!children) typeError("Cannot read null arithmetic child array");
    if (index >= children->size() || !(*children)[index])
        typeError("Cannot read required arithmetic child");
    return *(*children)[index];
}
inline std::shared_ptr<Attributes> arithmeticAttributes(MmlNode& node) {
    auto attributes = node.attributes();
    if (!attributes) typeError("Cannot read null arithmetic attributes");
    return attributes;
}
}
