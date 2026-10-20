// MathJax 4.1.3 input/tex/FindTeX.ts; Apache-2.0.
export module folia.math.input.tex.find_tex;
export import folia.math.core.find_math;
import folia.math.util.regexp_host;
import std;

export namespace folia::math::tex {
struct FindTeXOptions {
    std::vector<std::array<std::u16string, 2>> inlineMath{
        {u"\\(", u"\\)"}};
    std::vector<std::array<std::u16string, 2>> displayMath{
        {u"$$", u"$$"}, {u"\\[", u"\\]"}};
    bool processEscapes = true;
    bool processEnvironments = true;
    bool processRefs = true;
};

class FindTeX final : public core::AbstractFindMath<FindTeXOptions> {
    struct EndItem {
        std::u16string close;
        bool display = false;
        std::shared_ptr<regexp_host::RegExp> pattern;
    };

    std::shared_ptr<regexp_host::RegExp> start_;
    std::unordered_map<std::u16string, EndItem> end_;
    std::size_t environmentCapture_ = 0;
    std::size_t specialCapture_ = 0;
    bool hasPatterns_ = false;

    void getPatterns();
    void addPattern(std::vector<std::u16string>& starts,
                    std::array<std::u16string, 2> const& delimiters,
                    bool display);
    std::shared_ptr<regexp_host::RegExp> endPattern(
        std::u16string_view close,
        std::optional<std::u16string_view> overridePattern = {}) const;
    std::optional<core::ProtoItem> findEnd(
        std::u16string_view text, std::size_t n,
        regexp_host::Match const& start, EndItem const& end) const;
    void findMathInString(std::vector<core::ProtoItem>& math, std::size_t n,
                          std::u16string const& text);

public:
    explicit FindTeX(FindTeXOptions options = {});
    std::vector<core::ProtoItem> findMath(
        std::span<std::u16string const> strings) override;
};
}
