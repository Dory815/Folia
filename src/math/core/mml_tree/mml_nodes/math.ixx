// Typed native family API based on MathJax 4.1.3 MmlNodes/math.ts.
export module folia.math.core.mml_tree.mml_nodes.math;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMath : public AbstractMmlLayoutNode {
  public:
    explicit MmlMath(MmlDocument &document);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    bool linebreakContainer() const override;
    std::string linebreakAlign() const override;
    void verifyTree(std::optional<VerifyOptions> const &options = std::nullopt) override;

  protected:
    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;
};
} // namespace folia::math::mml
