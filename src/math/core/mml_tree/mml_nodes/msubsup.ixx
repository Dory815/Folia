// MathJax 4.1.3 MmlNodes/msubsup.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.msubsup;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMsubsup:public AbstractMmlBaseNode {
public:
    using AbstractMmlBaseNode::AbstractMmlBaseNode;
    static PropertyList& defaults();
    std::string kind()const override;
    double arity()const override;
    virtual std::size_t base()const;
    virtual std::size_t sub()const;
    virtual std::size_t sup()const;
protected:
    void setChildInheritedAttributes(AttributeList,Property,Property,Property)override;
};
class MmlMsub:public MmlMsubsup {
public:
    using MmlMsubsup::MmlMsubsup;
    static PropertyList& defaults();
    std::string kind()const override;
    double arity()const override;
};
class MmlMsup:public MmlMsubsup {
public:
    using MmlMsubsup::MmlMsubsup;
    static PropertyList& defaults();
    std::string kind()const override;
    double arity()const override;
    std::size_t sup()const override;
    std::size_t sub()const override;
};
}
