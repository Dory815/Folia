// MathJax 4.1.3 MmlNodes/ms.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.ms;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMs : public AbstractMmlTokenNode {
  public:
    explicit MmlMs(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
};
} // namespace folia::math::mml
