export module folia.math.core.mml_tree.mml_nodes.mtd;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMtd final : public AbstractMmlBaseNode {
  public:
    explicit MmlMtd(MmlDocument &document) : AbstractMmlBaseNode(document, NodeKind::mtd) {}

    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    bool linebreakContainer() const override;
    std::string linebreakAlign() const override;
    MmlNode *setTeXclass(MmlNode *) override;

  protected:
    void verifyChildren(VerifyOptions const &) override;
};
} // namespace folia::math::mml
