// MathJax 4.1.3 input/tex/ParseOptions.ts and its finite package option schema.
// Apache-2.0. Sparse values distinguish an omitted field from false/zero/empty.
export module folia.math.input.tex.parser:parse_options;
export import :types;
export import folia.math.util.function_list;
export import folia.math.util.regexp_host;
import std;

export namespace folia::math::tex {
using TagFormatValue = std::variant<std::u16string, std::vector<std::u16string>>;
using TagNumberFormatter = std::u16string (*)(std::int64_t);
using TagTextFormatter = TagFormatValue (*)(std::u16string_view);
using TagIdFormatter = std::u16string (*)(std::u16string_view);
using TagUrlFormatter = std::u16string (*)(std::u16string_view, std::u16string_view);
using OptionInputValue = std::variant<bool, std::u16string, regexp_host::RegExpRef>;
using ScriptLevel = std::variant<double, std::u16string>;
using InternalMath = std::vector<mml::MmlNode*> (*)(TexParser&, std::u16string_view,
    std::optional<ScriptLevel>, std::optional<std::u16string_view>);
using FormatError = mml::MmlNode& (*)(TeX&, TexError const&);
using FilterList = FunctionList<TexFilterData&>;
struct PackageSelection {
    std::u16string name;
    std::optional<double> priority;
};

struct TextMacrosOptions {
    std::optional<std::vector<PackageSelection>> packages;
    void mergeDefaults(TextMacrosOptions const&);
};
struct PhysicsOptions {
    std::optional<bool> italicdiff, arrowdel;
    void mergeDefaults(PhysicsOptions const&);
};
struct UnitsOptions {
    std::optional<bool> loose, ugly;
    void mergeDefaults(UnitsOptions const&);
};
struct BboldxOptions {
    std::optional<bool> bfbb, light;
    void mergeDefaults(BboldxOptions const&);
};
struct BbmOptions {
    std::optional<bool> bold;
    void mergeDefaults(BbmOptions const&);
};
struct DsfontOptions {
    std::optional<bool> sans;
    void mergeDefaults(DsfontOptions const&);
};
struct NoUndefinedOptions {
    std::optional<std::u16string> color, background, size;
    void mergeDefaults(NoUndefinedOptions const&);
};
struct ColorOptions {
    std::optional<std::u16string> padding, borderWidth;
    void mergeDefaults(ColorOptions const&);
};
struct BegingroupOptions {
    std::optional<std::vector<std::u16string>> allowGlobal;
    void mergeDefaults(BegingroupOptions const&);
};
struct AmscdOptions {
    std::optional<std::u16string> colspace, rowspace, harrowsize, varrowsize;
    std::optional<bool> hideHorizontalLabels;
    void mergeDefaults(AmscdOptions const&);
};
struct AmsOptions {
    std::optional<regexp_host::RegExpRef> operatornamePattern;
    std::optional<std::u16string> multlineWidth, multlineIndent;
    void mergeDefaults(AmsOptions const&);
};
struct TagFormatOptions {
    std::optional<TagNumberFormatter> number;
    std::optional<TagTextFormatter> tag, ref;
    std::optional<TagIdFormatter> id;
    std::optional<TagUrlFormatter> url;
    void mergeDefaults(TagFormatOptions const&);
};
struct SetOptionsOptions {
    using Permission = std::variant<bool, std::unordered_map<std::u16string, bool>>;
    using PackageFilter = bool (*)(TexParser&, std::u16string_view);
    using OptionFilter = bool (*)(TexParser&, std::u16string_view, std::u16string_view);
    using ValueFilter = OptionInputValue (*)(TexParser&, std::u16string_view,
        std::u16string_view, OptionInputValue const&);
    std::optional<PackageFilter> filterPackage;
    std::optional<OptionFilter> filterOption;
    std::optional<ValueFilter> filterValue;
    std::optional<bool> allowPackageDefault, allowOptionsDefault;
    std::optional<std::unordered_map<std::u16string, Permission>> allowOptions;
    void mergeDefaults(SetOptionsOptions const&);
};
struct PairedDelimiterOptions {
    std::u16string left, right;
    std::u16string body{u"#1"};
    std::uint8_t arity{1};
    std::u16string pre, post;
};
struct MathtoolsOptions {
    std::optional<std::u16string> multlinedGap, multlinedPos, multlinedWidth;
    std::optional<std::u16string> firstlineAfterskip, lastlinePreskip, smallmatrixAlign;
    std::optional<std::u16string> shortvdotsAdjustAbove, shortvdotsAdjustBelow;
    std::optional<bool> centercolon;
    std::optional<std::u16string> centercolonOffset, thincolonDx, thincolonDw;
    std::optional<bool> useUnicode, legacycolonsymbols;
    std::optional<std::u16string> prescriptSubFormat, prescriptSupFormat, prescriptArgFormat;
    std::optional<bool> allowMathtoolsset;
    std::optional<std::unordered_map<std::u16string, PairedDelimiterOptions>> pairedDelimiters;
    std::optional<std::unordered_map<std::u16string, std::array<std::u16string, 3>>> tagforms;
    void mergeDefaults(MathtoolsOptions const&);
};

// One schema is used both by sparse package defaults and by the effective
// input-jax options. Later defaults overwrite earlier present values, as in
// Options.defaultOptions; this operation does not implement userOptions warnings.
struct TexOptionValues {
    std::optional<std::vector<FilterList::Item>> preFilters, postFilters;
    std::optional<std::shared_ptr<FindTeX>> findTeX;
    std::optional<std::vector<PackageSelection>> packages;
    std::optional<std::size_t> maxBuffer, maxTemplateSubtitutions, maxMacros;
    std::optional<std::u16string> mathStyle;
    std::optional<FormatError> formatError;
    std::optional<std::u16string> tags, tagSide, tagIndent, tagAlign;
    std::optional<bool> useLabelIds, ignoreDuplicateLabels;
    std::optional<TextMacrosOptions> textmacros;
    std::optional<SetOptionsOptions> setoptions;
    std::optional<NoUndefinedOptions> noundefined;
    std::optional<BboldxOptions> bboldx;
    std::optional<UnitsOptions> units;
    std::optional<bool> allowTexHTML;
    std::optional<TagFormatOptions> tagformat;
    std::optional<PhysicsOptions> physics;
    std::optional<std::vector<std::u16string>> protectedMacros;
    std::optional<MathtoolsOptions> mathtools;
    std::optional<DsfontOptions> dsfont;
    std::optional<std::unordered_map<std::u16string, MacroDefinition>> active, macros;
    std::optional<std::unordered_map<std::u16string, EnvironmentDefinition>> environments;
    std::optional<ColorOptions> color;
    std::optional<BegingroupOptions> begingroup;
    std::optional<BbmOptions> bbm;
    std::optional<AmscdOptions> amscd;
    std::optional<std::u16string> multlineWidth;
    std::optional<AmsOptions> ams;
    std::optional<std::variant<std::u16string, regexp_host::RegExpRef>> digits;
    std::optional<regexp_host::RegExpRef> numberPattern, initialDigit,
        identifierPattern, initialLetter;
    std::optional<std::u16string> baseURL;
    std::optional<InternalMath> internalMath;
    void mergeDefaults(TexOptionValues const&);
};

// The service interface remains on the original owner. Constructor/factory,
// Tags, ColumnParser and request-list implementations are still being ported;
// no Context implementation is used behind this declaration.
class ParseOptions : public NodeFactoryConfiguration {
public:
    using MathStyleResolver = std::u16string (*)(std::u16string_view, bool);
    static MathStyleResolver getVariant(std::u16string_view) noexcept;
    ParseOptions(ParserConfiguration&, TexOptionValues&, TexSessionState&,
                 mml::MmlFactory&, MathStyleResolver capturedMathStyle);
    ~ParseOptions() override;
    ParseOptions(ParseOptions const&) = delete;
    ParseOptions& operator=(ParseOptions const&) = delete;

    SubHandlers& handlers;
    TexOptionValues& options; // borrowed input-jax lifetime record, never per-expression defaults
    std::unique_ptr<StackItemFactory> itemFactory;
    NodeFactory nodeFactory;
    std::unique_ptr<Tags> tags;
    MathStyleResolver mathStyle; // captured by input-jax construction, independent of later options
    std::unique_ptr<ColumnParser> columnParser;
    TexSessionState& packageData;
    std::vector<TexParser*> parsers;
    core::MathItem* mathItem{};
    mml::MmlNode* root{};
    std::unordered_map<std::u16string, std::vector<mml::MmlNode*>> nodeLists;
    bool error{};
    void pushParser(TexParser&);
    void popParser();
    void removeParser(TexParser&) noexcept;
    TexParser* parser() const noexcept;
    void clear();
    TexEnvironment const& environment() const override;
    void addNode(mml::NodeKind, mml::MmlNode&) override;
    void addNode(std::u16string_view, mml::MmlNode&) override;
    std::span<mml::MmlNode* const> getList(std::u16string_view);
    void removeFromList(std::u16string_view, std::span<mml::MmlNode* const>);
private:
    bool inTree(mml::MmlNode const&) const noexcept;
};
}
