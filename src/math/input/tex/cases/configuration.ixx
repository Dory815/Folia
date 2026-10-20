// MathJax 4.1.3 input/tex/cases/CasesConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.cases.configuration;
export import folia.math.input.tex.ams.configuration;
import std;
export namespace folia::math::tex {
class CasesBeginItem : public BeginItem {
  public:
    using BeginItem::BeginItem;
    bool end{};
    StackKind kind() const override;
    CheckResult checkItem(FrameHandle const &) override;
};
class CasesTags : public AmsTags {
    std::int64_t subcounter_{};

  public:
    using AmsTags::AmsTags;
    void start(std::string_view, bool taggable, bool defaultTags) override;
    void autoTag() override;

  protected:
    std::u16string formatNumberDefault(std::int64_t,
                                       std::optional<std::int64_t> = std::nullopt) override;
};
struct CasesMethods {
    static StackEntry NumCases(TexParser &, FrameHandle const &, std::span<TexArgument const>);
    static DispatchOutcome Entry(TexParser &, MacroInvocation const &);
    static DispatchOutcome environment(TexParser &, std::u16string_view, EnvironmentBodyHandler,
                                       std::span<TexArgument const>);
};
struct CasesConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
