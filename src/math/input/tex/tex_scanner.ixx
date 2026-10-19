// MathJax 4.1.3 TexParser lexical methods; Apache-2.0 Source semantics.
export module folia.math.input.tex.tex_scanner;
import std;
import folia.math.input.tex.tex_error;
export namespace folia::math::tex {
class TexScanner {
    std::u16string source_;
    std::size_t position_ = 0;
    std::u16string currentControlSequence_;
    [[noreturn]] void fail(std::string id, std::u16string format,
                           std::span<std::u16string const> arguments = {}) const;
public:
    explicit TexScanner(std::u16string source = {});
    std::u16string_view source() const noexcept;
    std::size_t position() const noexcept;
    void setPosition(std::size_t position) noexcept;
    void replaceSource(std::u16string source);
    std::u16string_view currentControlSequence() const noexcept;
    void setCurrentControlSequence(std::u16string value);

    std::u16string getCodePoint() const;
    bool nextIsSpace() const noexcept;
    std::u16string getNext();
    std::u16string getCS();
    std::optional<std::u16string> getArgument(std::u16string_view name = {}, bool noneOK = false);
    std::optional<std::u16string> getBrackets(
        std::u16string_view name = {},
        std::optional<std::u16string> fallback = std::nullopt,
        bool matchBrackets = false);
    std::u16string getUpTo(std::u16string_view name, std::u16string_view token);
    bool getStar();
};
}
