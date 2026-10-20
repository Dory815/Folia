// MathJax 4.1.3 input/tex/braket/BraketItems.ts; Apache-2.0.
export module folia.math.input.tex.braket.items;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class BraketItem final : public BaseItem {
  public:
    std::vector<mml::MmlNode *> barNodes; // borrows from the active MmlDocument
    std::size_t bar_count{};
    double bar_max{};
    std::u16string open, close;
    bool stretchy{}, single{}, space{};

    explicit BraketItem(StackItemFactory &);
    StackKind kind() const override;
    bool isOpen() const noexcept override;
    CheckResult checkItem(FrameHandle const &) override;
    mml::MmlNode &toMml(bool inferred = true, bool forceRow = false) override;
};
} // namespace folia::math::tex
