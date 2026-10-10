// MathJax 4.1.3 MmlNodes/munderover.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.munderover;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMunderover:public AbstractMmlBaseNode {
public:
    using AbstractMmlBaseNode::AbstractMmlBaseNode;
    static PropertyList& defaults();
    static std::shared_ptr<std::vector<Property>>& accents();
    std::string kind()const override;
    double arity()const override;
    virtual std::size_t base()const;
    virtual std::size_t under()const;
    virtual std::size_t over()const;
    bool linebreakContainer()const override;
protected:
    void setChildInheritedAttributes(AttributeList,Property,Property,Property)override;
    virtual Property getScriptlevel(Property accent,bool force,Property level);
    virtual void setInheritedAccent(std::size_t n,Property accent,Property display,Property level,Property prime,bool force);
};
class MmlMunder:public MmlMunderover {
public:
    using MmlMunderover::MmlMunderover;
    static PropertyList& defaults();
    std::string kind()const override;
    double arity()const override;
};
class MmlMover:public MmlMunderover {
public:
    using MmlMunderover::MmlMunderover;
    static PropertyList& defaults();
    static std::shared_ptr<std::vector<Property>>& accents();
    std::string kind()const override;
    double arity()const override;
    std::size_t over()const override;
    std::size_t under()const override;
};
}
