// MathJax 4.1.3 MmlNodes/munderover.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.munderover;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMunderover : public AbstractMmlBaseNode {
  protected:
    MmlMunderover(MmlDocument &, NodeKind);
    virtual std::u16string_view accentAttribute(std::size_t index) const noexcept;
    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;
    double getScriptlevel(std::u16string_view accent, bool force, double level) const;
    void setInheritedAccent(std::size_t index, std::u16string_view accent, bool display,
                            double level, bool prime, bool force);

  public:
    explicit MmlMunderover(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    virtual std::size_t base() const;
    virtual std::size_t under() const;
    virtual std::size_t over() const;
    bool linebreakContainer() const override;
};

class MmlMunder : public MmlMunderover {
  public:
    explicit MmlMunder(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
};

class MmlMover : public MmlMunderover {
  protected:
    std::u16string_view accentAttribute(std::size_t index) const noexcept override;

  public:
    explicit MmlMover(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    std::size_t over() const override;
    std::size_t under() const override;
};
} // namespace folia::math::mml
