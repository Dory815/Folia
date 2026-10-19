export module folia.math.core.mml_tree.mml_nodes.mphantom;
export import folia.math.core.mml_tree.mml_document;

export namespace folia::math::mml {
class MmlMphantom : public AbstractMmlLayoutNode {
  public:
    explicit MmlMphantom(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
};
} // namespace folia::math::mml
