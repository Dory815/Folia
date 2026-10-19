export module folia.math.core.mml_tree.mml_nodes.maligngroup;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMaligngroup final : public AbstractMmlLayoutNode {
  public:
    explicit MmlMaligngroup(MmlDocument &document)
        : AbstractMmlLayoutNode(document, NodeKind::maligngroup) {}

    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    bool isSpacelike() const override;

  protected:
    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;
};
} // namespace folia::math::mml
