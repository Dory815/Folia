export module folia.math.input.tex.session;
export import folia.math.input.tex.color.util;
export import folia.math.input.tex.token;
import std;

export namespace folia::math::tex {
// Owned definition data only. ParseUtil/TexParser remains the sole owner of
// argument scanning, placeholder substitution and TeX buffer-limit behavior.
struct MacroDefinition final {
    std::u16string replacement_source;
    std::u16string parameter_prefix;
    std::vector<std::u16string> parameter_delimiters;
    double arity{};
    std::optional<std::u16string> default_first_argument;
    bool parameter_template{};    // Source MacroWithTemplate versus replacement Macro
    bool substitute_parameters{}; // An explicit [0] still processes defaults and # parameters.
    void validate() const;
    friend bool operator==(MacroDefinition const &, MacroDefinition const &) = default;
};

struct NullArgument final {
    friend bool operator==(NullArgument, NullArgument) = default;
};
using TexArgument = std::variant<NullArgument, bool, double, std::u16string>;

// Distinct indices into immutable compiled C++ function tables. Zero is absent.
// No runtime allocation, function name lookup, receiver or parser lives here.
struct MacroHandlerId final {
    std::uint32_t value{};
    friend bool operator==(MacroHandlerId, MacroHandlerId) = default;
};
struct TokenParserId final {
    std::uint32_t value{};
    friend bool operator==(TokenParserId, TokenParserId) = default;
};
struct NativeMacroAlias final {
    MacroHandlerId handler;
    std::vector<TexArgument> arguments;
};
struct CharacterMacroAlias final {
    TokenParserId parser;
    Token captured; // Source Let's captured CharacterMap token, including attributes
};
struct MacroSuppression final {};
struct CommandDefinition final {
    std::u16string source_token;
    std::variant<MacroDefinition, NativeMacroAlias, CharacterMacroAlias, MacroSuppression> body;
    void validate() const;
};
using DelimiterDefinition = Token;
struct EnvironmentDefinition final {
    std::u16string begin_source, end_source;
    double arity{};
    std::optional<std::u16string> default_first_argument;
    bool substitute_parameters{};
};
enum class DefinitionKind { command, delimiter, environment };
struct DefinitionKey final {
    DefinitionKind kind;
    std::u16string name;
};
struct DefinitionLayer final {
    std::int32_t priority{};
    std::unordered_map<std::u16string, CommandDefinition> commands;
    std::unordered_map<std::u16string, DelimiterDefinition> delimiters;
    std::unordered_map<std::u16string, EnvironmentDefinition> environments;
    bool erase(DefinitionKey const &);
};
// Only owned values persist between expressions. The original BegingroupStack
// service operates on these layers; lookup never creates another definition map.
struct TexDefinitionState final {
    static constexpr std::int32_t newcommand_priority = -100;
    DefinitionLayer global{newcommand_priority};
    std::vector<DefinitionLayer> groups; // outermost -> innermost
    std::int32_t current_priority{newcommand_priority};
    std::int32_t top_priority{newcommand_priority};
    std::int32_t base_priority{newcommand_priority};
};

// NewColumnType definitions survive equations and tag resets. Their parser
// service borrows these values; no callback or parser is retained here.
struct MacroColumnDefinition final {
    std::u16string replacement;
    double argument_count{};
    friend bool operator==(MacroColumnDefinition const &, MacroColumnDefinition const &) = default;
};

class ColumnMacroDefinitions final {
    std::unordered_map<char16_t, MacroColumnDefinition> definitions_;

  public:
    MacroColumnDefinition const *find(char16_t specifier) const noexcept;
    void set(char16_t specifier, MacroColumnDefinition definition);
    bool erase(char16_t specifier) noexcept;
    std::size_t size() const noexcept {
        return definitions_.size();
    }
    bool empty() const noexcept {
        return definitions_.empty();
    }
    friend bool operator==(ColumnMacroDefinitions const &,
                           ColumnMacroDefinitions const &) = default;
};

struct LabelRecord final {
    std::u16string tag;
    std::u16string id;
    friend bool operator==(LabelRecord const &, LabelRecord const &) = default;
};
using LabelMap = std::unordered_map<std::u16string, LabelRecord>;
using IdSet = std::unordered_set<std::u16string>;
struct TagCounterState final {
    std::int64_t equation{};
    friend bool operator==(TagCounterState const &, TagCounterState const &) = default;
};

struct TagInfoState final {
    std::string environment;
    bool taggable{};
    bool default_tags{};
    std::optional<std::u16string> tag;
    std::u16string tag_id;
    std::variant<std::u16string, std::vector<std::u16string>> tag_format;
    bool no_tag{};
    std::u16string label_id;
};

// One request/equation's mutable tag inputs and outputs. It is returned by
// value from beginEquation and must not be stored in TexSessionState.
struct TexEquationState final {
    TagInfoState current_tag;
    std::vector<TagInfoState> history;
    std::vector<TagInfoState> stack;
    LabelMap labels;
    IdSet ids;
    TagCounterState counter;
    std::vector<std::u16string> tag_parts;
    std::u16string pending_label;
    bool redo{};
    bool ref_update{};
};
struct EquationFinishResult final {
    // When present, Source requested a recompile using this pre-commit counter.
    std::optional<std::int64_t> recompile_counter;
};

// Renderer-lifetime data corresponding to Tags.allLabels/allIds/allCounter
// and the renderer's macro definitions. No equation/parser/request state lives
// here. Equation scratch is passed explicitly to finishEquation.
class TexSessionState final {
  private:
    TexDefinitionState definitions_;
    LabelMap all_labels_;
    IdSet all_ids_;
    TagCounterState all_counter_;
    // Source ColorConfiguration creates one ColorModel per TeX input jax.
    // It survives individual equations and tag resets without retaining nodes.
    std::optional<ColorModel> color_;
    ColumnMacroDefinitions column_definitions_;

  public:
    TexSessionState() = default;
    TexSessionState(TexSessionState const &) = default;
    TexSessionState &operator=(TexSessionState const &) = default;
    TexSessionState(TexSessionState &&) noexcept = default;
    TexSessionState &operator=(TexSessionState &&) noexcept = default;

    MacroDefinition const *findMacro(std::u16string_view name) const;
    void defineMacro(std::u16string name, MacroDefinition definition);
    bool eraseMacro(std::u16string_view name);
    void clearMacros() noexcept;

    CommandDefinition const *findCommand(std::u16string_view name) const;
    void defineCommand(std::u16string name, CommandDefinition);
    bool eraseCommand(std::u16string_view name);
    DelimiterDefinition const *findDelimiter(std::u16string_view name) const;
    void defineDelimiter(std::u16string name, DelimiterDefinition);
    bool eraseDelimiter(std::u16string_view name);
    EnvironmentDefinition const *findEnvironment(std::u16string_view name) const;
    void defineEnvironment(std::u16string name, EnvironmentDefinition);
    bool eraseEnvironment(std::u16string_view name);
    TexDefinitionState &definitions() noexcept {
        return definitions_;
    }
    TexDefinitionState const &definitions() const noexcept {
        return definitions_;
    }
    DefinitionLayer &currentDefinitionLayer() noexcept;

    ColorModel &enableColor();
    ColorModel *colorModel() noexcept;
    ColorModel const *colorModel() const noexcept;

    ColumnMacroDefinitions &columnDefinitions() noexcept {
        return column_definitions_;
    }
    ColumnMacroDefinitions const &columnDefinitions() const noexcept {
        return column_definitions_;
    }

    LabelRecord const *findLabel(std::u16string_view name) const;
    LabelMap const &labels() const noexcept {
        return all_labels_;
    }
    bool containsId(std::u16string_view id) const;
    IdSet const &ids() const noexcept {
        return all_ids_;
    }
    TagCounterState counters() const noexcept {
        return all_counter_;
    }

    TexEquationState
    beginEquation(bool ref_update = false,
                  std::optional<std::int64_t> recompile_counter = std::nullopt) const;
    EquationFinishResult finishEquation(TexEquationState const &equation);
    void resetTags(std::int64_t offset = 0) noexcept;
};
} // namespace folia::math::tex
