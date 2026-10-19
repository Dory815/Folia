// MathJax 4.1.3 MmlNodes/mn.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mn;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::mml {
class MmlMn : public AbstractMmlTokenNode {
  public:
    explicit MmlMn(MmlDocument &);
    static AttributeSet defaults();
    NodeKind kind() const noexcept override;
};
} // namespace folia::math::mml
