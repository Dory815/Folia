// MathJax 4.1.3 input/tex/mathtools/MathtoolsItems.ts; Apache-2.0.
export module folia.math.input.tex.mathtools.items;
export import folia.math.input.tex.ams.items;
import std;
export namespace folia::math::tex {
class MultlinedItem final : public MultlineItem {
  public:
    explicit MultlinedItem(StackItemFactory &);
    StackKind kind() const override;
    void EndTable() override;
};
} // namespace folia::math::tex
