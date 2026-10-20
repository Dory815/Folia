module folia.math.core.mml_tree.node_properties;
import std;

namespace folia::math::mml {
void copyNodeProperties(const NodeProperties& source, NodeProperties& destination) {
    destination = source;
}

void mergeNodeProperties(const NodeProperties& source, NodeProperties& destination) {
#define FOLIA_MERGE_FIELD(group, field) if (source.group.field) destination.group.field = source.group.field
    FOLIA_MERGE_FIELD(tex,in_lists);
    FOLIA_MERGE_FIELD(tex,auto_operator); FOLIA_MERGE_FIELD(tex,fix_bold);
    FOLIA_MERGE_FIELD(tex,function_operator);
    FOLIA_MERGE_FIELD(tex,fix_prescript); FOLIA_MERGE_FIELD(tex,fix_stretchy);
    FOLIA_MERGE_FIELD(tex,ignore_variant); FOLIA_MERGE_FIELD(tex,math_accent);
    FOLIA_MERGE_FIELD(tex,movable_limits); FOLIA_MERGE_FIELD(tex,move_sup_sub);
    FOLIA_MERGE_FIELD(tex,no_dictionary_definition); FOLIA_MERGE_FIELD(tex,pseudo_script);
    FOLIA_MERGE_FIELD(tex,relations_combined); FOLIA_MERGE_FIELD(tex,small_matrix);
    FOLIA_MERGE_FIELD(tex,sub_sup_allowed); FOLIA_MERGE_FIELD(tex,prime_style);
    FOLIA_MERGE_FIELD(tex,variant_form); FOLIA_MERGE_FIELD(tex,with_delimiters);
    FOLIA_MERGE_FIELD(tex,open); FOLIA_MERGE_FIELD(tex,close);
    FOLIA_MERGE_FIELD(tex,left_delimiter); FOLIA_MERGE_FIELD(tex,right_delimiter);
    FOLIA_MERGE_FIELD(tex,primes); FOLIA_MERGE_FIELD(tex,keep_attributes);
    FOLIA_MERGE_FIELD(tex,vertical_box); FOLIA_MERGE_FIELD(tex,script_alignment);
    FOLIA_MERGE_FIELD(tex,script_position); FOLIA_MERGE_FIELD(tex,thickness);
    FOLIA_MERGE_FIELD(tex,explicit_tex_class);
    FOLIA_MERGE_FIELD(proof,axiom); FOLIA_MERGE_FIELD(proof,proof);
    FOLIA_MERGE_FIELD(proof,sequent); FOLIA_MERGE_FIELD(proof,sequent_processed);
    FOLIA_MERGE_FIELD(proof,premise_count); FOLIA_MERGE_FIELD(proof,rule_direction);
    FOLIA_MERGE_FIELD(proof,labelled_rule); FOLIA_MERGE_FIELD(proof,proof_label);
    FOLIA_MERGE_FIELD(proof,sequent_adjust_left); FOLIA_MERGE_FIELD(proof,sequent_adjust_right);
    FOLIA_MERGE_FIELD(proof,left_space); FOLIA_MERGE_FIELD(proof,right_space);
    FOLIA_MERGE_FIELD(layout,breakable); FOLIA_MERGE_FIELD(layout,force_break);
    FOLIA_MERGE_FIELD(layout,inline_breaks); FOLIA_MERGE_FIELD(layout,inline_forced);
    FOLIA_MERGE_FIELD(layout,inline_marked); FOLIA_MERGE_FIELD(layout,newline);
    FOLIA_MERGE_FIELD(layout,process_breaks); FOLIA_MERGE_FIELD(table,use_height);
    FOLIA_MERGE_FIELD(annotation,character_data);
#undef FOLIA_MERGE_FIELD
}

void moveProofProperties(ProofNodeProperties& source, ProofNodeProperties& destination) noexcept {
    if (source.premise_count) { destination.premise_count = std::move(source.premise_count); source.premise_count.reset(); }
    if (source.proof) { destination.proof = std::move(source.proof); source.proof.reset(); }
    if (source.labelled_rule) { destination.labelled_rule = std::move(source.labelled_rule); source.labelled_rule.reset(); }
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

void initializeNodeProperties(NodeKind kind, NodeProperties& properties) noexcept {
    if (kind == NodeKind::mtable) properties.table.use_height = true;
    if (kind == NodeKind::annotation) properties.annotation.character_data = true;
    // TeXAtom's Source constructor mirrors its intrinsic ORD class to the
    // explicit texClass property for serialization; other intrinsic classes
    // are direct class state only.
    if (kind == NodeKind::tex_atom) properties.tex.explicit_tex_class = TexClass::ord;
}
}
