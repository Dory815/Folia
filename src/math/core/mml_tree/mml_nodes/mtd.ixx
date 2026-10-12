// Root-frozen MathJax 4.1.3 mtd.ts interface; Apache-2.0.
export module folia.math.core.mml_tree.mml_nodes.mtd;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMtd : public AbstractMmlBaseNode {
public:
    using AbstractMmlBaseNode::AbstractMmlBaseNode;
    static PropertyList& defaults();
    std::string kind() const override;
    double arity() const override;
    bool linebreakContainer() const override;
    std::string linebreakAlign() const override;
    NodeValue setTeXclass(NodeValue) override;
protected:
    void verifyChildren(Property) override;
};
}
