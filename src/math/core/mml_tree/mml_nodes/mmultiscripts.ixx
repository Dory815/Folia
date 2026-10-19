// MathJax 4.1.3 MmlNodes/mmultiscripts.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mmultiscripts;
export import folia.math.core.mml_tree.mml_nodes.msubsup;
import std;

export namespace folia::math::mml {
class MmlMmultiscripts : public MmlMsubsup {
  protected:
    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;
    void verifyChildren(VerifyOptions const &) override;

  public:
    explicit MmlMmultiscripts(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
};

class MmlMprescripts : public AbstractMmlNode {
  public:
    explicit MmlMprescripts(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    void verifyTree(std::optional<VerifyOptions> const &options = std::nullopt) override;
};

class MmlNone : public AbstractMmlNode {
  public:
    explicit MmlNone(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    void verifyTree(std::optional<VerifyOptions> const &options = std::nullopt) override;
};
} // namespace folia::math::mml
