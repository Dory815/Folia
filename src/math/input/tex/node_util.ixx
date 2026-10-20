// MathJax 4.1.3 input/tex/NodeUtil.ts; typed MML property application.
export module folia.math.input.tex.node_util;
export import folia.math.core.mml_tree.mml_document;
export import folia.math.core.mml_tree.operator_dictionary;
import std;

export namespace folia::math::tex {
struct NodeDefinition {
    mml::AttributeSet attributes;
    mml::NodeProperties properties;
    NodeDefinition(mml::AttributeSet value = {}) : attributes(std::move(value)) {}
};
class NodeUtil final {
  public:
    static void setProperties(mml::MmlNode &, NodeDefinition);
    static void setChild(mml::MmlNode &, std::size_t, mml::MmlNode *);
    static void setChild(mml::MmlNode &parent, std::size_t index, mml::MmlNode &child) {
        setChild(parent, index, &child);
    }
    static std::u16string getText(mml::MmlNode const &);
    static mml::operator_dict::OperatorDef const *getForm(mml::MmlNode &);
    static mml::operator_dict::OperatorDef const *getOp(mml::MmlNode const &,
                                                        std::u16string_view form = u"infix");
    static mml::AttributeValue const *getMoAttribute(mml::MmlNode const &, std::u16string_view);
};
} // namespace folia::math::tex
