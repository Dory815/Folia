export module folia.math.core.mml_tree.mml_nodes.mpadded;
export import folia.math.core.mml_tree.mml_document;

export namespace folia::math::mml {
class MmlMpadded : public AbstractMmlLayoutNode {
  public:
    explicit MmlMpadded(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    bool linebreakContainer() const override;
    MmlNode *setTeXclass(MmlNode *) override;
};
} // namespace folia::math::mml
