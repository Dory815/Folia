// Typed native semantics/annotation APIs from MathJax 4.1.3 semantics.ts.
export module folia.math.core.mml_tree.mml_nodes.semantics;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlSemantics : public AbstractMmlBaseNode {
  public:
    explicit MmlSemantics(MmlDocument &document);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    bool notParent() const override;
};

class MmlAnnotationXML : public AbstractMmlNode {
  protected:
    MmlAnnotationXML(MmlDocument &document, NodeKind kind);

  public:
    explicit MmlAnnotationXML(MmlDocument &document);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;

  protected:
    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;
};

class MmlAnnotation : public MmlAnnotationXML {
  public:
    explicit MmlAnnotation(MmlDocument &document);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
};
} // namespace folia::math::mml
