// MathJax 4.1.3 input/tex/textmacros/TextParser.ts; Apache-2.0.
export module folia.math.input.tex.parser:text_parser;
export import :tex_parser;
export import :base_items;
export import :parse_options;
import std;
export namespace folia::math::tex {
class TextParser : public TexParser {
    std::u16string text_;
    std::optional<ScriptLevel> level_;
    std::vector<mml::MmlNode *> nodes_; // borrows from the active MmlDocument
  public:
    std::vector<EnvironmentScope> envStack;
    TextParser(std::u16string, ParserEnvironmentSeed, ParseOptions &,
               std::optional<ScriptLevel> level = {});
    std::u16string &text() noexcept {
        return text_;
    }
    std::u16string const &text() const noexcept {
        return text_;
    }
    std::optional<ScriptLevel> const &level() const noexcept {
        return level_;
    }
    std::span<mml::MmlNode *const> nodes() const noexcept {
        return nodes_;
    }
    virtual TexParser &texParser() const;
    Tags &tags() const override;
    mml::MmlNode *mml() override;
    void copyLists();
    void Parse() override;
    void saveText();
    void Push(StackEntry) override;
    void PushMath(mml::MmlNode &);
    void addAttributes(mml::MmlNode &);
    mml::MmlNode *ParseTextArg(std::u16string_view, TexEnvironment);
    mml::MmlNode *ParseArg(std::u16string_view) override;
    [[noreturn]] void Error(std::string, std::u16string, std::span<std::u16string const> = {});
};
} // namespace folia::math::tex
