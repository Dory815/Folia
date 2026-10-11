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
inline MmlNode& arithmeticChild(MmlNode&, std::size_t, std::string_view) {
    // Root frozen member-at-dereference binding; actual MML red follows.
    throw Error("Unimplemented",u"Root-frozen MML required child binding");
}
inline std::shared_ptr<Attributes> arithmeticAttributes(const MmlNode& node) {
    auto attributes = node.attributes();
    if (!attributes) typeError("Cannot read null arithmetic attributes");
    return attributes;
}
}
