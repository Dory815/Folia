// Root-frozen MathJax 4.1.3 msqrt.ts interface; Apache-2.0.
export module folia.math.core.mml_tree.mml_nodes.msqrt;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMsqrt : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    void initializeClassFields() override;
    std::string kind() const override;
    double arity() const override;
    bool linebreakContainer() const override;
    NodeValue setTeXclass(NodeValue) override;
protected:
    void setChildInheritedAttributes(AttributeList, Property, Property, Property) override;
};
}
