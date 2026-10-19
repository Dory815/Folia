export module folia.math.core.mml_tree.mml_nodes.malignmark;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMalignmark final : public AbstractMmlNode {
  public:
    explicit MmlMalignmark(MmlDocument &document)
        : AbstractMmlNode(document, NodeKind::malignmark) {}

    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    bool isSpacelike() const override;
};
} // namespace folia::math::mml
