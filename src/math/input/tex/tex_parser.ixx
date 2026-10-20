// MathJax 4.1.3 input/tex/TexParser.ts; Apache-2.0.
// Native parser interface. Shared dispatch aliases belong to :types.
export module folia.math.input.tex.parser:tex_parser;
export import :types;
export import :stack;
export import :token_map;
import std;

export namespace folia::math::tex {
class TexParser {
    // The order is significant: every Source field used by pushParser/start
    // construction is ready before the parser scope enters and Stack is built.
    TexScanner scanner_; // sole source, position (Source i), and currentCS
  public:
    ParseOptions &configuration;
    std::size_t macroCount{};

  private:
    std::size_t saveI{};

    class ParserScope final {
        ParseOptions &configuration_;
        TexParser &parser_;
        bool active_{true};

      public:
        ParserScope(ParseOptions &, TexParser &); // enters immediately
        ~ParserScope() noexcept;                  // removes this exact parser
        ParserScope(ParserScope const &) = delete;
        ParserScope &operator=(ParserScope const &) = delete;
        ParserScope(ParserScope &&) = delete;
        ParserScope &operator=(ParserScope &&) = delete;
        void release(); // normal Source popParser point, idempotent thereafter
    } scope_;

  public:
    Stack stack; // constructed after scope_; owns activation handles, not MML
  private:
    void updateResult(std::u16string_view input, std::size_t old);
    void composeLatex(mml::MmlNode &, std::u16string_view comp, std::size_t pos1, std::size_t pos2);
    void composeBraces(mml::MmlNode &);
    std::u16string composeBracedContent(mml::MmlNode &);
    mml::MmlNode &finishCreatedNode(mml::MmlNode &); // create's ignore-variant rule

  protected:
    // Actual TextParser needs its Parse/Push overrides during initialization.
    // It calls initializeTexParser in its constructor body before assigning level.
    struct DeferParsing final {};
    TexParser(std::u16string source, ParserEnvironmentSeed, ParseOptions &, DeferParsing);
    void initializeTexParser(); // Parse; Push(stop); restore initial env scope
    void finishParserScope();   // mml(), including TextParser::mml(), releases here
    virtual std::u16string trimTex(std::u16string);

  private:
    TexParser(std::u16string source, ResolvedParserEnvironmentSeed, ParseOptions &, DeferParsing);

  public:
    TexParser(std::u16string source, ParserEnvironmentSeed, ParseOptions &);
    virtual ~TexParser();
    TexParser(TexParser const &) = delete;
    TexParser &operator=(TexParser const &) = delete;
    TexParser(TexParser &&) = delete;
    TexParser &operator=(TexParser &&) = delete;

    virtual TexOptionValues &options() const noexcept;
    virtual StackItemFactory &itemFactory() const noexcept;
    virtual Tags &tags() const;
    virtual void setString(std::u16string);
    virtual std::u16string_view string() const noexcept;
    virtual std::u16string currentCS() const;
    virtual void setCurrentCS(std::u16string);
    std::size_t position() const noexcept;
    void setPosition(std::size_t) noexcept;
    SourceSnapshot sourceSnapshot() const noexcept;

    // ParseInput belongs to :types: {TexParser& parser; u16string token;}.
    // Preserve its receiver; Source parse may dispatch a parser other than *this.
    virtual DispatchOutcome parse(HandlerKind, ParseInput const &);
    virtual TokenLookup lookup(HandlerKind, std::u16string_view);
    virtual bool contains(HandlerKind, std::u16string_view);
    virtual std::u16string toString() const;
    virtual void Parse();
    virtual void Push(StackEntry);
    virtual void PushAll(std::span<StackEntry const>);
    virtual void PushAll(std::span<mml::MmlNode *const>);
    virtual mml::MmlNode *mml(); // nullable borrow from configuration.document()
    virtual std::optional<std::u16string> convertDelimiter(std::u16string_view);
    virtual std::u16string getCodePoint();
    virtual bool nextIsSpace();
    virtual std::u16string GetNext();
    virtual std::u16string GetCS();
    virtual std::optional<std::u16string> GetArgument(std::u16string_view name,
                                                      bool noneOK = false);
    virtual std::optional<std::u16string>
    GetBrackets(std::u16string_view name, std::optional<std::u16string> fallback = std::nullopt,
                bool matchBrackets = false);
    virtual std::optional<std::u16string> GetDelimiter(std::u16string_view name,
                                                       bool braceOK = false);
    virtual std::u16string GetDimen(std::u16string_view name);
    virtual std::u16string GetUpTo(std::u16string_view name, std::u16string_view token);
    virtual mml::MmlNode *ParseArg(std::u16string_view name);
    virtual mml::MmlNode *ParseUpTo(std::u16string_view name, std::u16string_view token);
    virtual std::optional<std::u16string> GetDelimiterArg(std::u16string_view name);
    virtual bool GetStar();

    // Original create(kind,...rest), expressed by its four actual creator domains.
    virtual mml::MmlNode &create(mml::NodeKind, std::span<mml::MmlNode *const> children = {},
                                 NodeDefinition = {});
    virtual mml::MmlNode &createToken(mml::NodeKind, std::u16string_view text,
                                      NodeDefinition = {});
    virtual mml::MmlNode &createText(std::u16string_view text);
    virtual mml::MmlNode &createError(std::u16string_view message);
};
} // namespace folia::math::tex
