export module folia.math.core.mml_tree.mml_nodes.mroot;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMroot : public AbstractMmlNode {
  public:
    explicit MmlMroot(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    bool linebreakContainer() const override;
    MmlNode *setTeXclass(MmlNode *) override;

  protected:
    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;
};
} // namespace folia::math::mml
