// Root bindings for pinned CJS __assign lowering and required child access.
export module folia.math.core.mml_tree.arithmetic_host;
export import folia.math.core.mml_tree.leaf_host;
import std;
export namespace folia::math::mml::host {
inline PropertyList arithmeticCopy(const Property& input) {
    // The locked MathJax distribution lowers {...input} to __assign({}, input),
    // which uses Object.assign, including its inherited __proto__ setter.
    return objectAssign(input);
}
inline MmlNode& arithmeticChild(MmlNode& node, std::size_t index, std::string_view member) {
    const auto children = node.childArrayOwner();
    if (!children) {
        const std::string message = "Cannot read properties of null (reading '" +
                                   std::to_string(index) + "')";
        typeError(message);
    }
    return *children->get(index).requireNode(member);
}
inline std::shared_ptr<Attributes> arithmeticAttributes(const MmlNode& node) {
    auto attributes = node.attributes();
    if (!attributes) typeError("Cannot read properties of null (reading 'get')");
    return attributes;
}
}
