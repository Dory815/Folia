// MathJax 4.1.3 input/tex/newcommand/NewcommandItems.ts; Apache-2.0.
export module folia.math.input.tex.newcommand.items;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class BeginEnvItem : public BaseItem {
  public:
    std::optional<std::u16string> name;
    explicit BeginEnvItem(StackItemFactory &);
    StackKind kind() const override;
    bool isOpen() const noexcept override;
    std::optional<std::u16string_view> getName() const noexcept override;
    CheckResult checkItem(FrameHandle const &) override;
};
} // namespace folia::math::tex
