// MathJax 4.1.3 MmlNodes/maction.ts; root-frozen declarations. Apache-2.0.
export module folia.math.core.mml_tree.mml_nodes.maction;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMaction:public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    std::string kind()const override;
    double arity()const override;
    virtual MmlNode* selected()const;
    bool isEmbellished()const override;
    bool isSpacelike()const override;
    NodeValue core()override;
    NodeValue coreMO()override;
    NodeValue setTeXclass(NodeValue)override;
    void nextToggleSelection();
protected:
    void verifyAttributes(Property)override;
    void setChildInheritedAttributes(AttributeList,Property,Property,Property)override;
};
}
