// MathJax 4.1.3 input/tex.ts; Apache-2.0.
export module folia.math.input.tex.parser:tex_jax;
export import :configuration;
export import :parse_options;
export import :tex_parser;
import std;

export namespace folia::math::tex {
class TeX : public core::AbstractInputJax<TexOptionValues, ParseOptions> {
    TexSessionState session_;
    std::unique_ptr<ParserConfiguration> configuration_;
    std::unique_ptr<ParseOptions> parseOptions_;
    std::unique_ptr<TextMacrosData> textmacros_;
    std::u16string latex_;
    mml::MmlNode *mathNode_{}; // borrowed from the caller's current MmlDocument
  public:
    // The compiled package registry, map registry, callback table and factory
    // outlive this jax. Per-expression MML is always owned by the supplied document.
    TeX(ConfigurationHandler &, MapHandler &, MacroCallbacks const &, WarningSink &,
        mml::MmlFactory &, TexOptionValues supplied = {});
    ~TeX() override;
    TeX(TeX const &) = delete;
    TeX &operator=(TeX const &) = delete;
    static TexOptionValues defaults();
    std::string_view name() const noexcept override;
    ParseOptions &parseOptions() const noexcept;
    ParserConfiguration &configuration() const noexcept;
    TexSessionState &session() noexcept;
    void installTextMacros(std::unique_ptr<TextMacrosData>);
    void setMmlFactory(mml::MmlFactory &) override;
    void reset(std::int64_t offset = 0);
    mml::MmlNode &compile(core::MathItem &, mml::MmlDocument &) override;
    virtual mml::MmlNode &formatError(TexError const &);
};
} // namespace folia::math::tex
