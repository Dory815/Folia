// Root bindings for pinned CJS __assign lowering and required child access.
export module folia.math.core.mml_tree.arithmetic_host;
export import folia.math.core.mml_tree.leaf_host;
import std;
export namespace folia::math::mml::host {
inline PropertyList arithmeticCopy(const PropertyList& input) {
    PropertyList copy;
    // The locked MathJax distribution lowers {...input} to __assign({}, input),
    // which uses Object.assign, including its inherited __proto__ setter.
    copy.assign(input);
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
