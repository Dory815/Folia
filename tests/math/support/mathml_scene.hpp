#pragma once

namespace mathml_scene_test {
enum class AttributeKind { text, boolean, number };

struct Attribute {
    std::u16string_view name;
    AttributeKind kind{};
    std::u16string_view text;
    double number{};
    bool boolean{};
};

struct Node {
    std::string_view kind;
    bool is_inferred{};
    std::size_t parent{};
    std::size_t attribute_first{};
    std::size_t attribute_count{};
    std::u16string_view text{};
    std::size_t xml_payload_index = std::numeric_limits<std::size_t>::max();
};

struct Metrics {
    double em{}, ex{}, container_width{};
    bool display{};
};

struct Case {
    std::string_view name;
    Metrics metrics;
    std::span<Node const> nodes;
    std::span<Attribute const> attributes;
    std::string_view source_svg;
};

using XmlInitializer = std::function<void(folia::math::mml::XMLNode &, std::size_t)>;

inline folia::math::mml::MmlNode &constructRoot(folia::math::mml::MmlDocument &document,
                                                std::span<Node const> nodes,
                                                std::span<Attribute const> attributes,
                                                XmlInitializer const &initialize_xml = {}) {
    using namespace folia::math::mml;
    constexpr auto no_parent = std::numeric_limits<std::size_t>::max();
    if (nodes.empty())
        throw std::invalid_argument("MathML fixture has no nodes");

    std::vector<NodeKind> kinds;
    kinds.reserve(nodes.size());
    std::size_t root_index = no_parent;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        auto const &node = nodes[i];
        if (node.parent == no_parent) {
            if (root_index != no_parent)
                throw std::invalid_argument("MathML fixture has multiple roots");
            root_index = i;
        } else if (node.parent >= i) {
            throw std::invalid_argument("MathML fixture must be parent-before-child");
        }
        if (node.attribute_first > attributes.size() ||
            node.attribute_count > attributes.size() - node.attribute_first)
            throw std::invalid_argument("MathML fixture attribute range is invalid");

        std::optional<NodeKind> kind;
        for (auto const candidate : allNodeKinds) {
            if (nodeKindName(candidate) == node.kind) {
                kind = candidate;
                break;
            }
        }
        if (!kind)
            throw std::invalid_argument("MathML fixture kind is not in the native catalog");
        const bool inferred_kind = *kind == NodeKind::inferred_mrow;
        if (node.is_inferred != inferred_kind)
            throw std::invalid_argument("MathML fixture inferred marker does not match its kind");
        if (*kind == NodeKind::html)
            throw std::invalid_argument("MathML fixture XML/HTML payload is unsupported");
        if (*kind == NodeKind::xml && !initialize_xml)
            throw std::invalid_argument(
                "MathML fixture XML payload requires an explicit initializer");
        if (*kind == NodeKind::xml &&
            node.xml_payload_index == std::numeric_limits<std::size_t>::max())
            throw std::invalid_argument("MathML fixture XML payload index is missing");
        if (*kind != NodeKind::text && !node.text.empty())
            throw std::invalid_argument("Only MathML text nodes may carry text content");
        kinds.push_back(*kind);

        for (auto const &attribute :
             attributes.subspan(node.attribute_first, node.attribute_count)) {
            if (attribute.name.empty())
                throw std::invalid_argument("MathML fixture attribute name is empty");
            if (attribute.kind != AttributeKind::text && attribute.kind != AttributeKind::boolean &&
                attribute.kind != AttributeKind::number)
                throw std::invalid_argument("MathML fixture attribute kind is invalid");
        }
    }
    if (root_index == no_parent)
        throw std::invalid_argument("MathML fixture has no root");

    std::vector<MmlNode *> built(nodes.size());
    for (std::size_t i = nodes.size(); i-- > 0;) {
        AttributeSet own_attributes;
        for (auto const &attribute :
             attributes.subspan(nodes[i].attribute_first, nodes[i].attribute_count)) {
            std::u16string name(attribute.name);
            switch (attribute.kind) {
            case AttributeKind::text:
                own_attributes.set(std::move(name), std::u16string(attribute.text));
                break;
            case AttributeKind::boolean:
                own_attributes.set(std::move(name), attribute.boolean);
                break;
            case AttributeKind::number:
                own_attributes.set(std::move(name), attribute.number);
                break;
            }
        }

        std::vector<MmlNode *> children;
        for (std::size_t child_index = i + 1; child_index < nodes.size(); ++child_index) {
            if (nodes[child_index].parent == i) {
                if (!built[child_index])
                    throw std::logic_error("MathML fixture child was not constructed");
                children.push_back(built[child_index]);
            }
        }
        auto &node = document.factory().make(kinds[i], std::move(own_attributes), children);
        if (kinds[i] == NodeKind::text)
            node.setText(std::u16string(nodes[i].text));
        if (kinds[i] == NodeKind::xml) {
            auto *xml = dynamic_cast<XMLNode *>(&node);
            if (!xml)
                throw std::logic_error("XML kind did not construct an XMLNode");
            initialize_xml(*xml, nodes[i].xml_payload_index);
        }
        built[i] = &node;
    }
    return *built[root_index];
}
} // namespace mathml_scene_test
