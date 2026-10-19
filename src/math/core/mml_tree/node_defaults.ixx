export module folia.math.core.mml_tree.node_defaults;
export import folia.math.core.mml_tree.attributes;
export import folia.math.core.mml_tree.node_kind;
import std;

export namespace folia::math::mml {
using AttributeDefaultValue =
    std::variant<AttributeNull, bool, double, std::u16string_view>;

struct AttributeDefaultEntry final {
    std::u16string_view name;
    AttributeDefaultValue value;
};

using AttributeDefaultView = std::span<const AttributeDefaultEntry>;

AttributeDefaultView nodeDefaults(NodeKind kind) noexcept;
AttributeDefaultView globalDefaults() noexcept;
}
