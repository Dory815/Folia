// Typed native family API based on MathJax 4.1.3 MmlNodes/mathchoice.ts.
export module folia.math.core.mml_tree.mml_nodes.mathchoice;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MathChoice : public AbstractMmlBaseNode {
  public:
    explicit MathChoice(MmlDocument &document);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    bool notParent() const override;
    void setInheritedAttributes(AttributeList const &attributes = {}, bool display = false,
                                double level = 0.0, bool prime = false) override;
};
} // namespace folia::math::mml
