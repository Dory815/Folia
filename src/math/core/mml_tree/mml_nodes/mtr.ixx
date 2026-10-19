export module folia.math.core.mml_tree.mml_nodes.mtr;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMtr : public AbstractMmlNode {
  protected:
    MmlMtr(MmlDocument &document, NodeKind kind) : AbstractMmlNode(document, kind) {}

    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;
    void verifyChildren(VerifyOptions const &) override;

  public:
    explicit MmlMtr(MmlDocument &document) : MmlMtr(document, NodeKind::mtr) {}

    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    bool linebreakContainer() const override;
    std::string linebreakAlign() const override;
    MmlNode *setTeXclass(MmlNode *) override;
};

class MmlMlabeledtr final : public MmlMtr {
  public:
    explicit MmlMlabeledtr(MmlDocument &document) : MmlMtr(document, NodeKind::mlabeledtr) {}

    NodeKind kind() const noexcept override;
    double arity() const override;
};
} // namespace folia::math::mml
