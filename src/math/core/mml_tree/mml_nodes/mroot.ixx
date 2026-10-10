// Root-frozen MathJax 4.1.3 mroot.ts interface; Apache-2.0.
export module folia.math.core.mml_tree.mml_nodes.mroot;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMroot : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    void initializeClassFields() override;
    std::string kind() const override;
    double arity() const override;
    bool linebreakContainer() const override;
    MmlNode* setTeXclass(MmlNode*) override;
protected:
    void setChildInheritedAttributes(AttributeList, Property, Property, Property) override;
};
}
