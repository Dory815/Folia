export module folia.math.core.mml_tree.mml_nodes.mtable;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMtable final : public AbstractMmlNode {
  public:
    explicit MmlMtable(MmlDocument &document) : AbstractMmlNode(document, NodeKind::mtable) {}

    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    bool linebreakContainer() const override;
    std::string linebreakAlign() const override;
    MmlNode *setTeXclass(MmlNode *) override;

  protected:
    void inheritAttributes(AttributeList const &, bool, double, bool) override;
    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;
    void verifyChildren(VerifyOptions const &) override;
};
} // namespace folia::math::mml
