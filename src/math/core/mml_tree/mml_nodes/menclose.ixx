export module folia.math.core.mml_tree.mml_nodes.menclose;
export import folia.math.core.mml_tree.mml_document;

export namespace folia::math::mml {
class MmlMenclose : public AbstractMmlNode {
  public:
    explicit MmlMenclose(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    double arity() const override;
    bool linebreakContainer() const override;
    MmlNode *setTeXclass(MmlNode *) override;
};
} // namespace folia::math::mml
