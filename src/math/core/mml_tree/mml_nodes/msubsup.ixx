// MathJax 4.1.3 MmlNodes/msubsup.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.msubsup;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMsubsup : public AbstractMmlBaseNode {
  protected:
    MmlMsubsup(MmlDocument &, NodeKind);
    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;

  public:
    explicit MmlMsubsup(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    virtual std::size_t base() const;
    virtual std::size_t sub() const;
    virtual std::size_t sup() const;
};

class MmlMsub : public MmlMsubsup {
  public:
    explicit MmlMsub(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
};

class MmlMsup : public MmlMsubsup {
  public:
    explicit MmlMsup(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    std::size_t sup() const override;
    std::size_t sub() const override;
};
} // namespace folia::math::mml
