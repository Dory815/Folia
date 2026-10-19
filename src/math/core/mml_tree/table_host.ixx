// Root-frozen primitive bindings for MathJax 4.1.3 table semantic methods.
export module folia.math.core.mml_tree.table_host;
export import folia.math.core.mml_tree.arithmetic_host;
import folia.math.util.string;
import std;
export namespace folia::math::mml::host {
inline MmlNode& tableNode(NodeValue value, std::string_view member) {
    return *value.requireNode(member);
}
inline Children tableChildren(MmlNode& node, std::string_view member="Symbol(Symbol.iterator)") {
    const auto children = node.childArrayOwner();
    if (!children) {
        if (member=="Symbol(Symbol.iterator)")
            typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
        typeError("Cannot read properties of null (reading '"+std::string(member)+"')");
    }
    return children;
}
inline NodeValue tableSlot(const Children& children, std::size_t index) {
    if (!children) typeError("Cannot read properties of null (reading '"+std::to_string(index)+"')");
    return children->get(index);
}
inline MmlNode& tableChild(const Children& children, std::size_t index, std::string_view member) {
    return tableNode(tableSlot(children,index),member);
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
inline NodeValue tableCreate(Factory& factory, std::string_view kind) {
    return factory.create(kind, {});
}
inline NodeValue tableCreate(Factory& factory, std::string_view kind, NodeValue child) {
    auto children=std::make_shared<tree::ChildArray<MmlNode>>();
    children->push_back(child);
    const std::array<std::any,2> args{std::any(PropertyList{}),std::any(children)};
    return factory.create(kind, tree::FactoryArgs(args));
}
}
