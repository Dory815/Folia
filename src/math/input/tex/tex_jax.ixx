// MathJax 4.1.3 input/tex.ts; Apache-2.0.
export module folia.math.input.tex.parser:tex_jax;
export import :configuration;
export import :parse_options;
export import :tex_parser;
import std;

export namespace folia::math::tex {
class TeXRequest;
class TeX : public core::AbstractInputJax<TexOptionValues, ParseOptions> {
    friend class TeXRequest;
    TexSessionState session_;
    std::unique_ptr<ParserConfiguration> configuration_;
    std::unique_ptr<ParseOptions> parseOptions_;
    std::unique_ptr<TextMacrosData> textmacros_;
  public:
    // Configuration and session outlive requests; expression state is caller-owned.
    TeX(ConfigurationHandler &, MapHandler &, MacroCallbacks const &, WarningSink &,
        TexOptionValues supplied = {});
    ~TeX() override;
    TeX(TeX const &) = delete;
    TeX &operator=(TeX const &) = delete;
    static TexOptionValues defaults();
    std::string_view name() const noexcept override;
    ParseOptions &parseOptions() const noexcept;
    ParserConfiguration &configuration() const noexcept;
    TexSessionState &session() noexcept;
    void installTextMacros(std::unique_ptr<TextMacrosData>);
    void reset(std::int64_t offset = 0);
    mml::MmlNode &compile(core::MathItem &, mml::MmlDocument &,
                          core::CompileServices = {}) override;
    virtual mml::MmlNode &formatError(TexError const &);
};

class TeXRequest final {
  public:
    ParseRequest main;
    std::optional<ParseRequest> text;

    TeXRequest(TeX &, mml::MmlFactory &);
    TeXRequest(TeXRequest const &) = delete;
    TeXRequest &operator=(TeXRequest const &) = delete;
    TeXRequest(TeXRequest &&) = delete;
    TeXRequest &operator=(TeXRequest &&) = delete;
};
} // namespace folia::math::tex
