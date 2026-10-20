// MathJax 4.1.3 input/tex/ams/AmsItems.ts; Apache-2.0.
export module folia.math.input.tex.ams.items;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class MultlineItem : public ArrayItem {
  public:
    MultlineItem(StackItemFactory &, bool numbered);
    StackKind kind() const override;
    void EndEntry() override;
    void EndRow() override;
    void EndTable() override;
};
class FlalignItem : public EqnArrayItem {
  public:
    bool padded{}, center{}, zeroWidthLabel{};
    std::optional<double> xalignat;
    FlalignItem(StackItemFactory &, std::u16string name, bool numbered, bool padded, bool center);
    StackKind kind() const override;
    void EndEntry() override;
    void EndRow() override;
    void EndTable() override;
};
} // namespace folia::math::tex
