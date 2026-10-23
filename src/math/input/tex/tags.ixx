// MathJax 4.1.3 Tags.ts class interfaces and typed creator catalog. Apache-2.0.
export module folia.math.input.tex.parser:tags;
export import :parse_options;
import std;
import folia.math.core.math_item;
import folia.math.input.tex.environment;

export namespace folia::math::tex {

using TagLabel = LabelRecord;

// A named tag form can decline to format, preserving the selected parent Tags.
class TagForm {
  public:
    virtual ~TagForm() = default;
    virtual std::optional<TagFormatValue> formatTag(std::u16string_view) const = 0;
};

class Tags {
  public:
    virtual ~Tags() = default;
    virtual std::string_view env() const = 0;
    virtual std::u16string_view label() const = 0;
    virtual void setLabel(std::u16string value) = 0;
    virtual void setFormatters(TexOptionValues const &) = 0;
    virtual void setTagForm(std::unique_ptr<TagForm>) = 0;
    virtual TagForm *tagForm() noexcept = 0; // borrowed, owned by this Tags product
    virtual TagFormatValue formatTag(std::u16string_view tag) = 0;
    virtual TagFormatValue formatRef(std::u16string_view tag) = 0;
    virtual std::u16string formatUrl(std::u16string_view id, std::u16string_view base) = 0;
    virtual void autoTag() = 0;
    virtual mml::MmlNode *getTag(bool force = false) = 0; // borrowed node or null
    virtual void clearTag() = 0;
    virtual void resetTag() = 0;
    virtual void reset(std::int64_t offset = 0) = 0;
    virtual void startEquation(core::MathItem const &) = 0;
    virtual void finishEquation(core::MathItem &) = 0;
    virtual mml::MmlNode &finalize(mml::MmlNode &, TexEnvironment const &) = 0;
    virtual void start(std::string_view environment, bool taggable, bool default_tags) = 0;
    virtual void end() = 0;
    virtual void tag(std::u16string_view value, bool no_format) = 0;
    virtual void notag() = 0;
    virtual mml::MmlNode &enTag(mml::MmlNode &, mml::MmlNode &) = 0;
};

class AbstractTags : public Tags {
  public:
    explicit AbstractTags(ParseOptions &) noexcept;
    AbstractTags(AbstractTags const &) = delete;
    AbstractTags &operator=(AbstractTags const &) = delete;
    ~AbstractTags() override;

    void setFormatters(TexOptionValues const &) final;
    void setTagForm(std::unique_ptr<TagForm>) final;
    TagForm *tagForm() noexcept final;
    TagFormatValue formatTag(std::u16string_view) final;
    TagFormatValue formatRef(std::u16string_view) final;
    std::u16string formatUrl(std::u16string_view, std::u16string_view) final;
    void autoTag() override;
    mml::MmlNode *getTag(bool force = false) override;
    void clearTag() override;
    void resetTag() override;
    void reset(std::int64_t offset = 0) override;
    void startEquation(core::MathItem const &) override;
    void finishEquation(core::MathItem &) override;
    mml::MmlNode &finalize(mml::MmlNode &, TexEnvironment const &) override;
    void start(std::string_view environment, bool taggable, bool default_tags) override;
    void end() override;
    void tag(std::u16string_view value, bool no_format) override;
    void notag() override;
    mml::MmlNode &enTag(mml::MmlNode &, mml::MmlNode &) override;
    std::string_view env() const override;
    std::u16string_view label() const override;
    void setLabel(std::u16string value) override;

  protected:
    bool noTag() const;
    std::u16string formatId(std::u16string_view id);
    std::u16string formatNumber(std::int64_t number,
                                std::optional<std::int64_t> subnumber = std::nullopt);
    virtual TagFormatValue formatTagDefault(std::u16string_view);
    virtual TagFormatValue formatRefDefault(std::u16string_view);
    virtual std::u16string formatUrlDefault(std::u16string_view, std::u16string_view);
    virtual std::u16string formatIdDefault(std::u16string_view);
    virtual std::u16string formatNumberDefault(std::int64_t,
                                               std::optional<std::int64_t> = std::nullopt);
    ParseOptions &configuration() const noexcept;

  private:
    void makeId();
    mml::MmlNode &makeTag();
    ParseOptions &configuration_; // sole borrowed access to document/session/request state
    // Borrow the stable input-jax record, never an optional payload address.
    TexOptionValues const *formatterOptions_{};
    std::unique_ptr<TagForm> tagForm_;
    bool tagFormFirst_{}; // the last installed formatTag layer has precedence
    TagFormatOptions const &formatters() const;
};

class NoTags : public AbstractTags {
  public:
    using AbstractTags::AbstractTags;
    void autoTag() override;
    mml::MmlNode *getTag(bool force = false) override;
};

class AllTags : public AbstractTags {
  public:
    using AbstractTags::AbstractTags;
    mml::MmlNode &finalize(mml::MmlNode &, TexEnvironment const &) override;
};

using TagsCreator = std::function<std::unique_ptr<Tags>(ParseOptions &)>;
struct TagsCreatorEntry final {
    std::u16string name;
    TagsCreator create;
};

class TagsFactory final {
  public:
    static TexOptionValues defaults();
    static TagFormatOptions formatters();
    static void add(ParserConfiguration &, TagsCreatorEntry);
    static void addTags(ParserConfiguration &, std::span<TagsCreatorEntry const> entries);
    static void setDefault(ParserConfiguration &, std::u16string_view name);
    static std::unique_ptr<Tags> getDefault(ParseOptions &, std::u16string_view default_name,
                                            std::span<TagsCreatorEntry const> creators);
    static std::unique_ptr<Tags> create(ParseOptions &, std::u16string_view requested_name,
                                        std::u16string_view default_name,
                                        std::span<TagsCreatorEntry const> creators);
    static TagsCreator creator(std::u16string_view requested_name, std::u16string_view default_name,
                               std::span<TagsCreatorEntry const> creators);
};

} // namespace folia::math::tex
