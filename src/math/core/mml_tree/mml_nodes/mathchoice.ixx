// Root MathJax 4.1.3 MathChoice declarations; Apache-2.0.
export module folia.math.core.mml_tree.mml_nodes.mathchoice;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MathChoice:public AbstractMmlBaseNode {
public:
    using AbstractMmlBaseNode::AbstractMmlBaseNode;
    static PropertyList& defaults();
    std::string kind()const override;
    double arity()const override;
    Property notParent()const override;
protected:
    void inheritAttributes(AttributeList,Property,Property,Property)override;
};
}
