// MathJax 4.1.3 MmlNodes/mmultiscripts.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mmultiscripts;
export import folia.math.core.mml_tree.mml_nodes.msubsup;
import std;
export namespace folia::math::mml {
class MmlMmultiscripts : public MmlMsubsup {
public:
    using MmlMsubsup::MmlMsubsup;
    static PropertyList& defaults();
    std::string kind()const override;
    double arity()const override;
protected:
    void setChildInheritedAttributes(AttributeList,Property,Property,Property)override;
    void verifyChildren(Property)override;
};
class MmlMprescripts : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    std::string kind()const override;
    double arity()const override;
    void verifyTree(Property options=Missing{})override;
};
class MmlNone : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    std::string kind()const override;
    double arity()const override;
    void verifyTree(Property options=Missing{})override;
};
}
