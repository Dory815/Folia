export module folia.math.core.mml_tree.node_properties;
export import folia.math.core.mml_tree.node_kind;
import std;

export namespace folia::math::mml {
enum class TexClass : std::int8_t {
    none = -1,
    ord = 0,
    op = 1,
    bin = 2,
    rel = 3,
    open = 4,
    close = 5,
    punct = 6,
    inner = 7
};
enum class HorizontalAlignment : std::uint8_t { left, center, right };
enum class ScriptPosition : std::uint8_t { subscript, superscript };
enum class ProofRuleDirection : std::uint8_t { up, down };
enum class ProofLabels : std::uint8_t { left, right, both };
enum class ProofSide : std::uint8_t { left, right };
enum class ProofAttribute : std::uint8_t {
    axiom,
    proof,
    sequent,
    sequent_processed,
    premise_count,
    rule_direction,
    labelled_rule,
    proof_label,
    sequent_adjust_left,
    sequent_adjust_right
};
using Thickness = std::variant<double, std::u16string>;

struct TexNodeProperties {
    std::optional<std::vector<std::u16string>> in_lists;
    std::optional<bool> auto_operator, function_operator, fix_bold, fix_prescript, fix_stretchy,
        ignore_variant, math_accent, movable_limits, move_sup_sub, no_dictionary_definition,
        pseudo_script, relations_combined, small_matrix, sub_sup_allowed, prime_style, variant_form,
        with_delimiters, unicode;
    std::optional<std::u16string> open, close, left_delimiter, right_delimiter, primes,
        keep_attributes, vertical_box;
    std::optional<HorizontalAlignment> script_alignment;
    std::optional<ScriptPosition> script_position;
    std::optional<Thickness> thickness;
    std::optional<TexClass> explicit_tex_class;
    friend bool operator==(TexNodeProperties const &, TexNodeProperties const &) = default;
};
struct ProofNodeProperties {
  private:
    std::optional<bool> axiom_, proof_, sequent_, sequent_processed_;
    std::optional<std::size_t> premise_count_;
    std::optional<ProofRuleDirection> rule_direction_;
    std::optional<ProofLabels> labelled_rule_;
    std::optional<ProofSide> proof_label_;
    std::optional<double> sequent_adjust_left_, sequent_adjust_right_;
    std::array<ProofAttribute, 10> order_{};
    std::size_t order_size_{};
    void remember(ProofAttribute) noexcept;

  public:
    std::optional<double> left_space, right_space;
    std::optional<bool> axiom() const noexcept;
    std::optional<bool> proof() const noexcept;
    std::optional<bool> sequent() const noexcept;
    std::optional<bool> sequent_processed() const noexcept;
    std::optional<std::size_t> premise_count() const noexcept;
    std::optional<ProofRuleDirection> rule_direction() const noexcept;
    std::optional<ProofLabels> labelled_rule() const noexcept;
    std::optional<ProofSide> proof_label() const noexcept;
    std::optional<double> sequent_adjust_left() const noexcept;
    std::optional<double> sequent_adjust_right() const noexcept;
    void setAxiom(bool) noexcept;
    void setProof(bool) noexcept;
    void setSequent(bool) noexcept;
    void setSequentProcessed(bool) noexcept;
    void setPremiseCount(std::size_t) noexcept;
    void setRuleDirection(ProofRuleDirection) noexcept;
    void setLabelledRule(ProofLabels) noexcept;
    void setProofLabel(ProofSide) noexcept;
    void setSequentAdjustLeft(double) noexcept;
    void setSequentAdjustRight(double) noexcept;
    void erase(ProofAttribute) noexcept;
    std::span<ProofAttribute const> orderedKeys() const noexcept;
    void mergeFrom(ProofNodeProperties const &) noexcept;
    friend bool operator==(ProofNodeProperties const &, ProofNodeProperties const &) = default;
};
struct LayoutNodeProperties {
    std::optional<bool> breakable, force_break, inline_breaks, inline_forced, inline_marked,
        newline, process_breaks;
    friend bool operator==(LayoutNodeProperties const &, LayoutNodeProperties const &) = default;
};
struct TableNodeProperties {
    std::optional<bool> use_height;
    friend bool operator==(TableNodeProperties const &, TableNodeProperties const &) = default;
};
struct AnnotationNodeProperties {
    std::optional<bool> character_data;
    friend bool operator==(AnnotationNodeProperties const &,
                           AnnotationNodeProperties const &) = default;
};
struct NodeProperties {
    TexNodeProperties tex;
    ProofNodeProperties proof;
    LayoutNodeProperties layout;
    TableNodeProperties table;
    AnnotationNodeProperties annotation;
    friend bool operator==(NodeProperties const &, NodeProperties const &) = default;
};
struct MmlIntrinsicState {
    std::optional<TexClass> working_tex_class;
    std::optional<TexClass> previous_tex_class;
    std::optional<double> previous_script_level;
    friend bool operator==(MmlIntrinsicState const &, MmlIntrinsicState const &) = default;
};

void copyNodeProperties(const NodeProperties &, NodeProperties &);
void mergeNodeProperties(const NodeProperties &, NodeProperties &);
void moveProofProperties(ProofNodeProperties &, ProofNodeProperties &) noexcept;
MmlIntrinsicState initialIntrinsicState(NodeKind) noexcept;
void initializeNodeProperties(NodeKind, NodeProperties &) noexcept;
} // namespace folia::math::mml
