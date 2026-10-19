// MathJax 4.1.3 MmlNodes/mtext.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mtext;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMtext : public AbstractMmlTokenNode {
  public:
    explicit MmlMtext(MmlDocument &);
    static AttributeSet defaults();
    static std::span<const std::u16string_view> NONSPACELIKE();
    NodeKind kind() const noexcept override;
    bool isSpacelike() const override;
};
} // namespace folia::math::mml
