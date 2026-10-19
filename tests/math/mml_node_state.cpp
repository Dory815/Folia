#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_document;
using namespace boost::ut;
namespace mml=folia::math::mml;

suite native_mml_node_properties=[] {
    "constructor_intrinsic_state_and_family_defaults_match_source"_test=[] {
        mml::MmlDocument document;
        auto& factory = document.factory();
        for (const auto kind : mml::allNodeKinds) {
            std::optional<mml::TexClass> expected;
            switch (kind) {
                case mml::NodeKind::mi: case mml::NodeKind::mn: case mml::NodeKind::mtext:
                case mml::NodeKind::ms: case mml::NodeKind::msqrt: case mml::NodeKind::mroot:
                case mml::NodeKind::merror: case mml::NodeKind::mphantom: case mml::NodeKind::menclose:
                case mml::NodeKind::mtable: case mml::NodeKind::mglyph: case mml::NodeKind::tex_atom:
                    expected=mml::TexClass::ord; break;
                case mml::NodeKind::mspace: expected=mml::TexClass::none; break;
                case mml::NodeKind::mfenced: expected=mml::TexClass::inner; break;
                default: break;
            }
            auto& node=factory.make(kind);
            expect(node.state().working_tex_class==expected);
            if (kind==mml::NodeKind::tex_atom)
                expect(node.properties().tex.explicit_tex_class==mml::TexClass::ord);
            if (kind==mml::NodeKind::mtable) expect(node.properties().table.use_height==true);
            if (kind==mml::NodeKind::annotation) expect(node.properties().annotation.character_data==true);
        }
        auto& math=factory.make(mml::NodeKind::math);
        expect(!math.state().working_tex_class.has_value());
        expect(!math.properties().table.use_height.has_value());
        expect(!math.properties().annotation.character_data.has_value());
    };

    "all_typed_fields_copy_merge_and_absence_semantics_are_preserved"_test=[] {
        mml::NodeProperties source, destination;
        source.tex.auto_operator=false; source.tex.fix_bold=true; source.tex.fix_prescript=false;
        source.tex.fix_stretchy=true; source.tex.ignore_variant=false; source.tex.math_accent=true;
        source.tex.movable_limits=false; source.tex.move_sup_sub=true;
        source.tex.no_dictionary_definition=false; source.tex.pseudo_script=true;
        source.tex.relations_combined=false; source.tex.small_matrix=true;
        source.tex.sub_sup_allowed=false; source.tex.prime_style=true;
        source.tex.variant_form=false; source.tex.with_delimiters=true;
        source.tex.open=u""; source.tex.close=u")"; source.tex.left_delimiter=u"[";
        source.tex.right_delimiter=u"]"; source.tex.primes=u"′′"; source.tex.keep_attributes=u"class";
        source.tex.vertical_box=u"parbox:baseline"; source.tex.script_alignment=mml::HorizontalAlignment::center;
        source.tex.script_position=mml::ScriptPosition::superscript;
        source.tex.thickness=mml::Thickness{std::u16string{}};
        source.tex.explicit_tex_class=mml::TexClass::rel;
        source.proof.axiom=false; source.proof.proof=true; source.proof.sequent=false;
        source.proof.sequent_processed=true; source.proof.premise_count=0;
        source.proof.rule_direction=mml::ProofRuleDirection::down;
        source.proof.labelled_rule=mml::ProofLabels::both; source.proof.proof_label=mml::ProofSide::left;
        source.proof.sequent_adjust_left=0.0; source.proof.sequent_adjust_right=-0.25;
        source.proof.left_space=0.0; source.proof.right_space=0.5;
        source.layout.breakable=false; source.layout.force_break=true; source.layout.inline_breaks=false;
        source.layout.inline_forced=true; source.layout.inline_marked=false;
        source.layout.newline=true; source.layout.process_breaks=false;
        source.table.use_height=false;
        source.annotation.character_data=false;

        destination.tex.open=u"preserved"; destination.proof.axiom=true;
        mml::mergeNodeProperties(source,destination);
        expect(destination==source);
        expect(destination.tex.auto_operator==false); expect(destination.tex.open==u"");
        expect(destination.tex.vertical_box==u"parbox:baseline"); expect(destination.tex.thickness==source.tex.thickness);
        expect(destination.proof.premise_count==0u); expect(destination.proof.axiom==false);
        expect(destination.layout.process_breaks==false); expect(destination.table.use_height==false);
        expect(destination.annotation.character_data==false);
        mml::NodeProperties exact; mml::copyNodeProperties(source,exact);
        expect(exact==source);
        expect(exact.tex.auto_operator==source.tex.auto_operator);
        expect(exact.tex.explicit_tex_class==mml::TexClass::rel);
        expect(exact.proof.sequent_adjust_right==-0.25); expect(exact.layout.inline_marked==false);
        expect(exact.proof.right_space==0.5); expect(exact.table.use_height==false);
        mml::NodeProperties absent;
        mml::mergeNodeProperties(absent,destination);
        expect(destination==source);
        expect(destination.tex.open==u""); expect(destination.proof.premise_count==0u);
        expect(destination.layout.process_breaks==false);
    };

    "proof_move_transfers_only_three_present_fields_and_clears_source"_test=[] {
        mml::ProofNodeProperties from,to;
        from.premise_count=2; from.proof=false; from.labelled_rule=mml::ProofLabels::right;
        from.axiom=true; from.rule_direction=mml::ProofRuleDirection::up;
        to.premise_count=0; to.proof=true; to.labelled_rule=mml::ProofLabels::left;
        mml::moveProofProperties(from,to);
        expect(to.premise_count==2u); expect(to.proof==false);
        expect(to.labelled_rule==mml::ProofLabels::right);
        expect(!from.premise_count); expect(!from.proof); expect(!from.labelled_rule);
        expect(from.axiom==true); expect(from.rule_direction==mml::ProofRuleDirection::up);
        mml::ProofNodeProperties empty;
        mml::moveProofProperties(empty,to);
        expect(to.premise_count==2u); expect(to.proof==false);
    };

    "clone_copies_complete_typed_state_independently"_test=[] {
        mml::MmlDocument source_document,target_document;
        auto& factory = source_document.factory();
        auto& root=factory.make(mml::NodeKind::mrow);
        auto& child=factory.make(mml::NodeKind::mi);
        child.properties().tex.primes=u"′"; child.properties().proof.premise_count=3;
        child.state().working_tex_class=mml::TexClass::bin;
        child.state().previous_tex_class=mml::TexClass::rel;
        child.state().previous_script_level=-2.375;
        source_document.append(root,child);
        auto& clone=target_document.cloneSubtree(root);
        auto& clone_child=*clone.children()[0];
        expect(clone_child.properties()==child.properties());
        expect(clone_child.state()==child.state());
        expect(clone_child.properties().tex.primes==u"′");
        expect(clone_child.properties().proof.premise_count==3u);
        expect(clone_child.state().working_tex_class==mml::TexClass::bin);
        expect(clone_child.state().previous_tex_class==mml::TexClass::rel);
        expect(clone_child.state().previous_script_level==-2.375);
        clone_child.properties().tex.primes=u"changed"; clone_child.state().working_tex_class.reset();
        expect(child.properties().tex.primes==u"′");
        expect(child.state().working_tex_class==mml::TexClass::bin);
    };
};
