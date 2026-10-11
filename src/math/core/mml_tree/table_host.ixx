// Root-frozen primitive bindings for MathJax 4.1.3 table semantic methods.
export module folia.math.core.mml_tree.table_host;
export import folia.math.core.mml_tree.arithmetic_host;
import folia.math.util.string;
import std;
export namespace folia::math::mml::host {
inline MmlNode& tableNode(MmlNode* node) {
    if (!node) typeError("Cannot read null table node");
    return *node;
}
inline Children tableChildren(MmlNode& node) {
    const auto children = node.childNodes;
    if (!children) typeError("Cannot read null table child array");
    return children;
}
inline MmlNode* tableSlot(const Children& children, std::size_t index) {
    if (!children) typeError("Cannot read null table child array");
    return index < children->size() ? (*children)[index] : nullptr;
}
inline MmlNode& tableChild(const Children& children, std::size_t index) {
    return tableNode(tableSlot(children, index));
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
inline MmlNode& tableCreate(Factory& factory, std::string_view kind) {
    return tableNode(factory.create(kind, {}));
}
inline MmlNode& tableCreate(Factory& factory, std::string_view kind, MmlNode* child) {
    const auto children = std::make_shared<std::vector<MmlNode*>>(
        std::initializer_list<MmlNode*>{child});
    const std::array<std::any, 2> args{PropertyList{}, children};
    return tableNode(factory.create(kind, args));
}
}
