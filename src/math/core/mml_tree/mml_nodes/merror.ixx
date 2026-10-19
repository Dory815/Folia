export module folia.math.core.mml_tree.mml_nodes.merror;
export import folia.math.core.mml_tree.mml_document;

export namespace folia::math::mml {
class MmlMerror : public AbstractMmlNode {
  public:
    explicit MmlMerror(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    bool linebreakContainer() const override;
};
} // namespace folia::math::mml
