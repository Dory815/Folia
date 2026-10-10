// MathJax 4.1.3 core/MmlTree/OperatorDictionary.ts; Apache-2.0.
export module folia.math.core.mml_tree.operator_dictionary;
export import folia.math.core.tree.property;
import std;
export namespace folia::math::mml::operator_dict {
using OperatorDef=PropertyArray;
using OperatorList=PropertyList;
// Runtime ranges are Property handles; matched range identity is retained.
using RangeDef=Property;
OperatorDef OPDEF(Property left,Property right,Property texClass=Missing{},Property properties=Missing{});
RangeDef getRange(std::u16string_view text);
PropertyList& MO();
PropertyArray& RANGES();
PropertyArray& MMLSPACING();
PropertyList& OPTABLE();
}
