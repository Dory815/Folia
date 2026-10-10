// Root-frozen MathJax 4.1.3 mfrac.ts interface; Apache-2.0.
export module folia.math.core.mml_tree.mml_nodes.mfrac;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMfrac : public AbstractMmlBaseNode {
public:
    using AbstractMmlBaseNode::AbstractMmlBaseNode;
    static PropertyList& defaults();
    std::string kind() const override;
    double arity() const override;
    bool linebreakContainer() const override;
    std::string linebreakAlign() const override;
    MmlNode* setTeXclass(MmlNode*) override;
protected:
    void setChildInheritedAttributes(AttributeList, Property, Property, Property) override;
};
}
