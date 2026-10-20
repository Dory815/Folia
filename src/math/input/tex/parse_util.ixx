// MathJax 4.1.3 input/tex/ParseUtil.ts; Apache-2.0.
export module folia.math.input.tex.parse_util;
export import folia.math.input.tex.parser;
import std;
export import folia.math.input.tex.tex_error;

export namespace folia::math::tex {
// Actual key-value validators from ParseUtil.ts. The only owned state for
// oneof is its allowed strings; callbacks are ordinary compiled C++ functions.
class KeyValueDef {
  public:
    using Verify = bool (*)(std::u16string_view);
    using Convert = mml::AttributeValue (*)(std::u16string_view);
    std::u16string name;
    KeyValueDef(std::u16string, Verify, Convert);
    static KeyValueDef oneof(std::span<std::u16string_view const>);
    bool verify(std::u16string_view) const;
    mml::AttributeValue convert(std::u16string_view) const;

  private:
    Verify verify_;
    Convert convert_;
    std::optional<std::vector<std::u16string>> choices_;
};
namespace KeyValueTypes {
extern KeyValueDef const boolean, number, integer, string, dimen;
}
struct KeyValueOption {
    std::u16string_view key;
    std::optional<KeyValueDef> type;
};
namespace parse_util {
struct ExpansionBudget final {
    std::size_t maxMacros;
    std::size_t maxBuffer;
    std::size_t macroCount = 0;
};

std::u16string addArgs(std::u16string_view left, std::u16string_view right, std::size_t maxBuffer);

std::u16string substituteArgs(std::span<std::u16string const> arguments,
                              std::u16string_view replacement, std::size_t maxBuffer);

void checkMaxMacros(ExpansionBudget &budget, bool isMacro = true);
std::u16string addArgs(TexParser &, std::u16string_view, std::u16string_view);
std::u16string substituteArgs(TexParser &, std::span<std::u16string const>, std::u16string_view);
void checkMaxMacros(TexParser &, bool isMacro = true);
mml::AttributeSet
keyvalOptions(std::u16string_view,
              std::optional<std::span<KeyValueOption const>> allowed = std::nullopt,
              bool error = false, bool l3keys = false);
void checkEqnEnv(TexParser &, bool nestable = true);
std::vector<mml::MmlNode *> internalMath(TexParser &, std::u16string_view,
                                         std::optional<ScriptLevel> level = std::nullopt,
                                         std::optional<std::u16string_view> font = std::nullopt);
mml::MmlNode &internalText(TexParser &, std::u16string_view, NodeDefinition);
mml::MmlNode &underOver(TexParser &, mml::MmlNode &, mml::MmlNode &, std::u16string_view position,
                        bool stack);
void checkMovableLimits(mml::MmlNode &);
ArrayItem &setArrayAlign(ArrayItem &, std::optional<std::u16string_view>, TexParser * = nullptr);
mml::MmlNode &copyNode(mml::MmlNode const &, TexParser &);
std::u16string mmlFilterAttribute(TexParser &, std::u16string_view, std::u16string_view);
NodeDefinition getFontDef(TexParser &);
bool isLatinOrGreekChar(std::u16string_view);
void fixInitialMO(ParseOptions &, std::vector<mml::MmlNode *> &);
mml::MmlNode &fenced(ParseOptions &, std::u16string_view open, mml::MmlNode &,
                     std::u16string_view close, std::u16string_view big = {},
                     std::u16string_view color = {});
mml::MmlNode &mathPalette(ParseOptions &, std::u16string_view fence, std::u16string_view side);
mml::MmlNode &fixedFence(ParseOptions &, std::u16string_view open, mml::MmlNode &,
                         std::u16string_view close);
} // namespace parse_util
} // namespace folia::math::tex
