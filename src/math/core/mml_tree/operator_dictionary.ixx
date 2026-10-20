// Typed immutable operator data from MathJax 4.1.3 OperatorDictionary.ts.
export module folia.math.core.mml_tree.operator_dictionary;
export import folia.math.core.mml_tree.attributes;
export import folia.math.core.mml_tree.node_properties;
import std;

export namespace folia::math::mml::operator_dict {
struct OperatorDef {
    double left;
    double right;
    TexClass texClass;
    std::vector<AttributeSet::Entry> properties;
};
struct RangeDef {
    char32_t first;
    char32_t last;
    TexClass texClass;
    std::u16string kind;
    std::optional<std::u16string> variant;
};
using NamedOperatorDef = std::pair<std::u16string, OperatorDef>;
struct OperatorEntry {
    std::u16string form;
    std::u16string text;
    OperatorDef definition;
};

OperatorDef OPDEF(double left, double right, TexClass texClass = TexClass::bin,
                  std::vector<AttributeSet::Entry> properties = {});
RangeDef getRange(std::u16string_view text);
const OperatorDef* findOperatorDefinition(std::u16string_view form,
                                         std::u16string_view text);
std::span<NamedOperatorDef const> MO();
std::span<RangeDef const> RANGES();
std::span<std::array<double, 2> const> MMLSPACING();
std::span<OperatorEntry const> OPTABLE();
}
