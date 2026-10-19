// Typed native family API based on MathJax 4.1.3 MmlNodes/maction.ts.
export module folia.math.core.mml_tree.mml_nodes.maction;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMaction : public AbstractMmlNode {
  public:
    explicit MmlMaction(MmlDocument &document);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    virtual MmlNode &selected() const;
    bool isEmbellished() const override;
    bool isSpacelike() const override;
    MmlNode *core() override;
    MmlNode *coreMO() override;
    MmlNode *setTeXclass(MmlNode *previous) override;
    void nextToggleSelection();

  protected:
    void verifyAttributes(VerifyOptions const &) override;
    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;
};
} // namespace folia::math::mml
