// Typed native family API based on MathJax 4.1.3 MmlNodes/mfenced.ts.
export module folia.math.core.mml_tree.mml_nodes.mfenced;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMfenced : public AbstractMmlNode {
  public:
    explicit MmlMfenced(MmlDocument &document);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
    std::vector<MmlNode *> separators;
    MmlNode *open{};
    MmlNode *close{};
    MmlNode *setTeXclass(MmlNode *previous) override;

  protected:
    void copyNativeStateTo(MmlNode &destination, MmlClone &clone) const override;
    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;
    virtual void addFakeNodes();
    virtual MmlNode &fakeNode(std::u16string character, AttributeSet attributes = {},
                              std::optional<TexClass> texClass = std::nullopt);
};
} // namespace folia::math::mml
