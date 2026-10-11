// Root-frozen MathJax 4.1.3 mtr.ts interfaces; Apache-2.0.
export module folia.math.core.mml_tree.mml_nodes.mtr;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMtr : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    std::string kind() const override;
    bool linebreakContainer() const override;
    std::string linebreakAlign() const override;
    MmlNode* setTeXclass(MmlNode*) override;
protected:
    void setChildInheritedAttributes(AttributeList, Property, Property, Property) override;
    void verifyChildren(PropertyList) override;
};
class MmlMlabeledtr : public MmlMtr {
public:
    using MmlMtr::MmlMtr;
    std::string kind() const override;
    double arity() const override;
};
}
