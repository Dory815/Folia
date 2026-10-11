// MathJax 4.1.3 MmlNodes/TeXAtom.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.tex_atom;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlTeXAtom : public AbstractMmlBaseNode {
public:
    using AbstractMmlBaseNode::AbstractMmlBaseNode;
    static PropertyList& defaults();
    void initializeClassFields()override;
    std::string kind()const override;
    double arity()const override;
    bool notParent()const override;
    MmlNode* setTeXclass(MmlNode*)override;
    virtual MmlNode* adjustTeXclass(MmlNode*);
};
}
