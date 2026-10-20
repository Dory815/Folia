module folia.math.core.mml_tree.node_properties;
import std;

namespace folia::math::mml {
void ProofNodeProperties::remember(ProofAttribute key) noexcept {
    auto keys = orderedKeys();
    if (std::ranges::find(keys, key) == keys.end())
        order_[order_size_++] = key;
}
std::span<ProofAttribute const> ProofNodeProperties::orderedKeys() const noexcept {
    return {order_.data(), order_size_};
}
std::optional<bool> ProofNodeProperties::axiom() const noexcept {
    return axiom_;
}
std::optional<bool> ProofNodeProperties::proof() const noexcept {
    return proof_;
}
std::optional<bool> ProofNodeProperties::sequent() const noexcept {
    return sequent_;
}
std::optional<bool> ProofNodeProperties::sequent_processed() const noexcept {
    return sequent_processed_;
}
std::optional<std::size_t> ProofNodeProperties::premise_count() const noexcept {
    return premise_count_;
}
std::optional<ProofRuleDirection> ProofNodeProperties::rule_direction() const noexcept {
    return rule_direction_;
}
std::optional<ProofLabels> ProofNodeProperties::labelled_rule() const noexcept {
    return labelled_rule_;
}
std::optional<ProofSide> ProofNodeProperties::proof_label() const noexcept {
    return proof_label_;
}
std::optional<double> ProofNodeProperties::sequent_adjust_left() const noexcept {
    return sequent_adjust_left_;
}
std::optional<double> ProofNodeProperties::sequent_adjust_right() const noexcept {
    return sequent_adjust_right_;
}
void ProofNodeProperties::setAxiom(bool value) noexcept {
    remember(ProofAttribute::axiom);
    axiom_ = value;
}
void ProofNodeProperties::setProof(bool value) noexcept {
    remember(ProofAttribute::proof);
    proof_ = value;
}
void ProofNodeProperties::setSequent(bool value) noexcept {
    remember(ProofAttribute::sequent);
    sequent_ = value;
}
void ProofNodeProperties::setSequentProcessed(bool value) noexcept {
    remember(ProofAttribute::sequent_processed);
    sequent_processed_ = value;
}
void ProofNodeProperties::setPremiseCount(std::size_t value) noexcept {
    remember(ProofAttribute::premise_count);
    premise_count_ = value;
}
void ProofNodeProperties::setRuleDirection(ProofRuleDirection value) noexcept {
    remember(ProofAttribute::rule_direction);
    rule_direction_ = value;
}
void ProofNodeProperties::setLabelledRule(ProofLabels value) noexcept {
    remember(ProofAttribute::labelled_rule);
    labelled_rule_ = value;
}
void ProofNodeProperties::setProofLabel(ProofSide value) noexcept {
    remember(ProofAttribute::proof_label);
    proof_label_ = value;
}
void ProofNodeProperties::setSequentAdjustLeft(double value) noexcept {
    remember(ProofAttribute::sequent_adjust_left);
    sequent_adjust_left_ = value;
}
void ProofNodeProperties::setSequentAdjustRight(double value) noexcept {
    remember(ProofAttribute::sequent_adjust_right);
    sequent_adjust_right_ = value;
}
void ProofNodeProperties::erase(ProofAttribute key) noexcept {
    switch (key) {
    case ProofAttribute::axiom:
        axiom_.reset();
        break;
    case ProofAttribute::proof:
        proof_.reset();
        break;
    case ProofAttribute::sequent:
        sequent_.reset();
        break;
    case ProofAttribute::sequent_processed:
        sequent_processed_.reset();
        break;
    case ProofAttribute::premise_count:
        premise_count_.reset();
        break;
    case ProofAttribute::rule_direction:
        rule_direction_.reset();
        break;
    case ProofAttribute::labelled_rule:
        labelled_rule_.reset();
        break;
    case ProofAttribute::proof_label:
        proof_label_.reset();
        break;
    case ProofAttribute::sequent_adjust_left:
        sequent_adjust_left_.reset();
        break;
    case ProofAttribute::sequent_adjust_right:
        sequent_adjust_right_.reset();
        break;
    }
    auto keys = orderedKeys();
    auto found = std::ranges::find(keys, key);
    if (found == keys.end())
        return;
    auto index = static_cast<std::size_t>(found - keys.begin());
    for (; index + 1 < order_size_; ++index)
        order_[index] = order_[index + 1];
    order_[--order_size_] = {};
}
void ProofNodeProperties::mergeFrom(ProofNodeProperties const &source) noexcept {
    for (auto key : source.orderedKeys()) {
        switch (key) {
        case ProofAttribute::axiom:
            setAxiom(*source.axiom());
            break;
        case ProofAttribute::proof:
            setProof(*source.proof());
            break;
        case ProofAttribute::sequent:
            setSequent(*source.sequent());
            break;
        case ProofAttribute::sequent_processed:
            setSequentProcessed(*source.sequent_processed());
            break;
        case ProofAttribute::premise_count:
            setPremiseCount(*source.premise_count());
            break;
        case ProofAttribute::rule_direction:
            setRuleDirection(*source.rule_direction());
            break;
        case ProofAttribute::labelled_rule:
            setLabelledRule(*source.labelled_rule());
            break;
        case ProofAttribute::proof_label:
            setProofLabel(*source.proof_label());
            break;
        case ProofAttribute::sequent_adjust_left:
            setSequentAdjustLeft(*source.sequent_adjust_left());
            break;
        case ProofAttribute::sequent_adjust_right:
            setSequentAdjustRight(*source.sequent_adjust_right());
            break;
        }
    }
    if (source.left_space)
        left_space = source.left_space;
    if (source.right_space)
        right_space = source.right_space;
}
void copyNodeProperties(const NodeProperties &source, NodeProperties &destination) {
    destination = source;
}

void mergeNodeProperties(const NodeProperties &source, NodeProperties &destination) {
#define FOLIA_MERGE_FIELD(group, field)                                                            \
    if (source.group.field)                                                                        \
    destination.group.field = source.group.field
    FOLIA_MERGE_FIELD(tex, in_lists);
    FOLIA_MERGE_FIELD(tex, auto_operator);
    FOLIA_MERGE_FIELD(tex, fix_bold);
    FOLIA_MERGE_FIELD(tex, function_operator);
    FOLIA_MERGE_FIELD(tex, fix_prescript);
    FOLIA_MERGE_FIELD(tex, fix_stretchy);
    FOLIA_MERGE_FIELD(tex, ignore_variant);
    FOLIA_MERGE_FIELD(tex, math_accent);
    FOLIA_MERGE_FIELD(tex, movable_limits);
    FOLIA_MERGE_FIELD(tex, move_sup_sub);
    FOLIA_MERGE_FIELD(tex, no_dictionary_definition);
    FOLIA_MERGE_FIELD(tex, pseudo_script);
    FOLIA_MERGE_FIELD(tex, relations_combined);
    FOLIA_MERGE_FIELD(tex, small_matrix);
    FOLIA_MERGE_FIELD(tex, sub_sup_allowed);
    FOLIA_MERGE_FIELD(tex, prime_style);
    FOLIA_MERGE_FIELD(tex, variant_form);
    FOLIA_MERGE_FIELD(tex, with_delimiters);
    FOLIA_MERGE_FIELD(tex, open);
    FOLIA_MERGE_FIELD(tex, close);
    FOLIA_MERGE_FIELD(tex, left_delimiter);
    FOLIA_MERGE_FIELD(tex, right_delimiter);
    FOLIA_MERGE_FIELD(tex, primes);
    FOLIA_MERGE_FIELD(tex, keep_attributes);
    FOLIA_MERGE_FIELD(tex, vertical_box);
    FOLIA_MERGE_FIELD(tex, script_alignment);
    FOLIA_MERGE_FIELD(tex, script_position);
    FOLIA_MERGE_FIELD(tex, thickness);
    FOLIA_MERGE_FIELD(tex, explicit_tex_class);
    destination.proof.mergeFrom(source.proof);
    FOLIA_MERGE_FIELD(layout, breakable);
    FOLIA_MERGE_FIELD(layout, force_break);
    FOLIA_MERGE_FIELD(layout, inline_breaks);
    FOLIA_MERGE_FIELD(layout, inline_forced);
    FOLIA_MERGE_FIELD(layout, inline_marked);
    FOLIA_MERGE_FIELD(layout, newline);
    FOLIA_MERGE_FIELD(layout, process_breaks);
    FOLIA_MERGE_FIELD(table, use_height);
    FOLIA_MERGE_FIELD(annotation, character_data);
#undef FOLIA_MERGE_FIELD
}

void moveProofProperties(ProofNodeProperties &source, ProofNodeProperties &destination) noexcept {
    if (auto value = source.premise_count()) {
        destination.setPremiseCount(*value);
        source.erase(ProofAttribute::premise_count);
    }
    if (auto value = source.proof()) {
        destination.setProof(*value);
        source.erase(ProofAttribute::proof);
    }
    if (auto value = source.labelled_rule()) {
        destination.setLabelledRule(*value);
        source.erase(ProofAttribute::labelled_rule);
    }
}

MmlIntrinsicState initialIntrinsicState(NodeKind kind) noexcept {
    switch (kind) {
    case NodeKind::mi:
    case NodeKind::mn:
    case NodeKind::mtext:
    case NodeKind::ms:
    case NodeKind::msqrt:
    case NodeKind::mroot:
    case NodeKind::merror:
    case NodeKind::mphantom:
    case NodeKind::menclose:
    case NodeKind::mtable:
    case NodeKind::mglyph:
    case NodeKind::tex_atom:
        return {TexClass::ord, {}, {}};
    case NodeKind::mspace:
        return {TexClass::none, {}, {}};
    case NodeKind::mfenced:
        return {TexClass::inner, {}, {}};
    default:
        return {};
    }
}

void initializeNodeProperties(NodeKind kind, NodeProperties &properties) noexcept {
    if (kind == NodeKind::mtable)
        properties.table.use_height = true;
    if (kind == NodeKind::annotation)
        properties.annotation.character_data = true;
    // TeXAtom's Source constructor mirrors its intrinsic ORD class to the
    // explicit texClass property for serialization; other intrinsic classes
    // are direct class state only.
    if (kind == NodeKind::tex_atom)
        properties.tex.explicit_tex_class = TexClass::ord;
}
} // namespace folia::math::mml
