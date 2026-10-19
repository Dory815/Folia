export module folia.math.input.tex.session;
import std;

export namespace folia::math::tex {
// Owned definition data only. ParseUtil/TexParser remains the sole owner of
// argument scanning, placeholder substitution and TeX buffer-limit behavior.
struct MacroDefinition final {
    std::u16string replacement_source;
    std::u16string parameter_prefix;
    std::vector<std::u16string> parameter_delimiters;
    std::uint8_t arity{};
    std::optional<std::u16string> default_first_argument;
    void validate() const;
    friend bool operator==(MacroDefinition const&, MacroDefinition const&) = default;
};

struct LabelRecord final {
    std::u16string tag;
    std::u16string id;
    friend bool operator==(LabelRecord const&, LabelRecord const&) = default;
};
using LabelMap = std::unordered_map<std::u16string, LabelRecord>;
using IdSet = std::unordered_set<std::u16string>;
struct TagCounterState final {
    std::int64_t equation{};
    friend bool operator==(TagCounterState const&, TagCounterState const&) = default;
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
public:
    using MacroMap = std::unordered_map<std::string, MacroDefinition>;
private:
    MacroMap macros_;
    LabelMap all_labels_;
    IdSet all_ids_;
    TagCounterState all_counter_;
public:
    TexSessionState() = default;
    TexSessionState(TexSessionState const&) = default;
    TexSessionState& operator=(TexSessionState const&) = default;
    TexSessionState(TexSessionState&&) noexcept = default;
    TexSessionState& operator=(TexSessionState&&) noexcept = default;

    MacroDefinition const* findMacro(std::string_view ascii_name) const;
    void defineMacro(std::string ascii_name, MacroDefinition definition);
    bool eraseMacro(std::string_view ascii_name);
    MacroMap const& macros() const noexcept { return macros_; }
    void clearMacros() noexcept { macros_.clear(); }

    LabelRecord const* findLabel(std::u16string_view name) const;
    LabelMap const& labels() const noexcept { return all_labels_; }
    bool containsId(std::u16string_view id) const;
    IdSet const& ids() const noexcept { return all_ids_; }
    TagCounterState counters() const noexcept { return all_counter_; }

    TexEquationState beginEquation(
        bool ref_update = false,
        std::optional<std::int64_t> recompile_counter = std::nullopt) const;
    EquationFinishResult finishEquation(TexEquationState const& equation);
    void resetTags(std::int64_t offset = 0) noexcept;
};
}
