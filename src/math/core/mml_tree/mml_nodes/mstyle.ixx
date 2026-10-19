export module folia.math.core.mml_tree.mml_nodes.mstyle;
export import folia.math.core.mml_tree.mml_document;

export namespace folia::math::mml {
class MmlMstyle : public AbstractMmlLayoutNode {
  public:
    explicit MmlMstyle(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    bool notParent() const override;

  protected:
    void inheritAttributes(AttributeList const &, bool, double, bool) override;
    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;
};
} // namespace folia::math::mml
