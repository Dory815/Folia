// MathJax 4.1.3 MmlNodes/semantics.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.semantics;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlSemantics : public AbstractMmlBaseNode {
public:
    using AbstractMmlBaseNode::AbstractMmlBaseNode;
    static PropertyList& defaults();
    std::string kind()const override;
    double arity()const override;
    Property notParent()const override;
};
class MmlAnnotationXML : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    std::string kind()const override;
protected:
    void setChildInheritedAttributes(AttributeList,Property,Property,Property)override;
};
class MmlAnnotation : public MmlAnnotationXML {
public:
    using MmlAnnotationXML::MmlAnnotationXML;
    static PropertyList& defaults();
    void initializeClassFields()override;
    std::string kind()const override;
};
}
