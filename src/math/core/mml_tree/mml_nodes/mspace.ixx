// MathJax 4.1.3 MmlNodes/mspace.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mspace;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMspace : public AbstractMmlTokenNode {
  public:
    explicit MmlMspace(MmlDocument &);
    static AttributeSet defaults();
    static std::span<const std::u16string_view> NONSPACELIKE();
    NodeKind kind() const noexcept override;
    double arity() const override;
    bool isSpacelike() const override;
    bool hasNewline() const;
    bool canBreak() const;
    MmlNode *setTeXclass(MmlNode *) override;
};
} // namespace folia::math::mml
