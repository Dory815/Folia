// Root-frozen primitive bindings for MathJax 4.1.3 table semantic methods.
export module folia.math.core.mml_tree.table_host;
export import folia.math.core.mml_tree.arithmetic_host;
import folia.math.util.string;
import std;
export namespace folia::math::mml::host {
inline MmlNode& tableNode(NodeValue, std::string_view) {
    throw Error("Unimplemented",u"Root-frozen MML table receiver binding");
}
inline Children tableChildren(MmlNode& node) {
    const auto children = node.childNodes;
    if (!children) typeError("Cannot read null table child array");
    return children;
}
inline NodeValue tableSlot(const Children&, std::size_t) {
    throw Error("Unimplemented",u"Root-frozen MML table indexed Get");
}
inline MmlNode& tableChild(const Children&, std::size_t, std::string_view) {
    throw Error("Unimplemented",u"Root-frozen MML table child member");
}
inline std::deque<Property> tableSplit(const Property& value) {
    const auto* raw = std::get_if<std::string>(&value.value);
    if (!raw) typeError("Table alignment trim is not callable");
    std::deque<Property> result;
    for (const auto& item : string_util::split(text(*raw)))
        result.emplace_back(debugText(item));
    return result;
}
inline Property tableShift(std::deque<Property>& values) {
    if (values.empty()) return Missing{};
    Property first = values.front();
    values.pop_front();
    return first;
}
inline void tableSetIndex(const Property& receiver, std::string_view key, Property value) {
    auto object = PropertyList::record(receiver);
    if (!object) typeError("Cannot assign table tuple property");
    object->set(key, std::move(value));
}
inline MmlNode& tableCreate(Factory&, std::string_view) {
    throw Error("Unimplemented",u"Root-frozen MML table factory receiver");
}
inline MmlNode& tableCreate(Factory&, std::string_view, NodeValue) {
    throw Error("Unimplemented",u"Root-frozen MML table factory child");
}
}
