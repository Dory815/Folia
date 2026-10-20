export module folia.math.input.tex.environment;
export import folia.math.util.regexp_host;
import std;

export namespace folia::math::tex {
struct TexEnvironment final {
    std::optional<bool> display;
    std::optional<bool> in_root;
    std::optional<bool> operator_letters;
    std::optional<regexp_host::RegExpRef> multi_letter_identifiers;
    std::optional<bool> no_auto_op;
    std::optional<bool> is_global;
    std::optional<bool> bold_symbol;
    std::optional<bool> vector_star;
    std::optional<double> script_level;
    std::optional<std::u16string> font;
    std::optional<std::u16string> italic_font;
    std::optional<std::u16string> style;
    std::optional<double> size;
    std::optional<std::u16string> color;
    std::optional<std::u16string> horizontal_size;
    std::optional<std::u16string> closing_environment;
    std::optional<std::u16string> cd_min_width;
    std::optional<std::u16string> cd_min_height;
    std::optional<std::u16string> vector_font;
    std::optional<std::u16string> math_variant;
    std::optional<std::u16string> math_size;
    std::optional<std::u16string> math_color;

    friend bool operator==(TexEnvironment const&, TexEnvironment const&) = default;
};

struct ParserEnvironmentSeed final {
    TexEnvironment environment;
    std::optional<bool> is_inner;
};

struct ResolvedParserEnvironmentSeed final {
    TexEnvironment environment;
    bool is_inner{true};

    friend bool operator==(ResolvedParserEnvironmentSeed const&,
                           ResolvedParserEnvironmentSeed const&) = default;
};

void overlayEnvironment(TexEnvironment& destination,
                       TexEnvironment const& source);
ResolvedParserEnvironmentSeed resolveParserEnvironmentSeed(
    ParserEnvironmentSeed seed);
}
