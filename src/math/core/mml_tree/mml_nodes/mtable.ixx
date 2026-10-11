// Root-frozen MathJax 4.1.3 mtable.ts interface; Apache-2.0.
export module folia.math.core.mml_tree.mml_nodes.mtable;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMtable : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    void initializeClassFields() override;
    std::string kind() const override;
    bool linebreakContainer() const override;
    std::string linebreakAlign() const override;
    void setInheritedAttributes(AttributeList, Property, Property, Property) override;
    MmlNode* setTeXclass(MmlNode*) override;
protected:
    void setChildInheritedAttributes(AttributeList, Property, Property, Property) override;
    void verifyChildren(PropertyList) override;
};
}
