#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mn;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mtext;
import folia.math.core.mml_tree.mml_nodes.mspace;
import folia.math.core.mml_tree.mml_nodes.ms;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.mfrac;
import folia.math.core.mml_tree.mml_nodes.msqrt;
import folia.math.core.mml_tree.mml_nodes.mroot;
import folia.math.core.mml_tree.mml_nodes.mstyle;
import folia.math.core.mml_tree.mml_nodes.merror;
import folia.math.core.mml_tree.mml_nodes.mpadded;
import folia.math.core.mml_tree.mml_nodes.mphantom;
import folia.math.core.mml_tree.mml_nodes.mfenced;
import folia.math.core.mml_tree.mml_nodes.menclose;
import folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.core.mml_tree.mml_nodes.mmultiscripts;
import folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.core.mml_tree.mml_nodes.mtr;
import folia.math.core.mml_tree.mml_nodes.mtd;
import folia.math.core.mml_tree.mml_nodes.maligngroup;
import folia.math.core.mml_tree.mml_nodes.malignmark;
import folia.math.core.mml_tree.mml_nodes.mglyph;
import folia.math.core.mml_tree.mml_nodes.semantics;
import folia.math.core.mml_tree.mml_nodes.tex_atom;
import folia.math.core.mml_tree.mml_nodes.mathchoice;
import folia.math.core.mml_tree.mml_nodes.html_node;
using namespace boost::ut;
namespace mml = folia::math::mml;

namespace {
template <class Family> void expect_raw_family(mml::MmlDocument &document, mml::NodeKind kind) {
    auto &raw = document.make(kind);
    expect(dynamic_cast<Family *>(&raw) != nullptr);
    expect(raw.kind() == kind);
    expect(raw.isKind(kind));
    expect(&raw.document() == &document);
    expect(raw.attributes().ownEntries().empty());
    expect(raw.attributes().defaultEntries().empty());
    expect(raw.attributes().globalDefaultEntries().empty());
    expect(raw.children().empty());
}
} // namespace

suite math_native_mml_families = [] {
    "all_43_registry_kinds_create_their_real_typed_family_without_defaults"_test = [] {
        mml::MmlDocument document;
        expect_raw_family<mml::MmlMath>(document, mml::NodeKind::math);
        expect_raw_family<mml::MmlMi>(document, mml::NodeKind::mi);
        expect_raw_family<mml::MmlMn>(document, mml::NodeKind::mn);
        expect_raw_family<mml::MmlMo>(document, mml::NodeKind::mo);
        expect_raw_family<mml::MmlMtext>(document, mml::NodeKind::mtext);
        expect_raw_family<mml::MmlMspace>(document, mml::NodeKind::mspace);
        expect_raw_family<mml::MmlMs>(document, mml::NodeKind::ms);
        expect_raw_family<mml::MmlMrow>(document, mml::NodeKind::mrow);
        expect_raw_family<mml::MmlInferredMrow>(document, mml::NodeKind::inferred_mrow);
        expect_raw_family<mml::MmlMfrac>(document, mml::NodeKind::mfrac);
        expect_raw_family<mml::MmlMsqrt>(document, mml::NodeKind::msqrt);
        expect_raw_family<mml::MmlMroot>(document, mml::NodeKind::mroot);
        expect_raw_family<mml::MmlMstyle>(document, mml::NodeKind::mstyle);
        expect_raw_family<mml::MmlMerror>(document, mml::NodeKind::merror);
        expect_raw_family<mml::MmlMpadded>(document, mml::NodeKind::mpadded);
        expect_raw_family<mml::MmlMphantom>(document, mml::NodeKind::mphantom);
        expect_raw_family<mml::MmlMfenced>(document, mml::NodeKind::mfenced);
        expect_raw_family<mml::MmlMenclose>(document, mml::NodeKind::menclose);
        expect_raw_family<mml::MmlMaction>(document, mml::NodeKind::maction);
        expect_raw_family<mml::MmlMsub>(document, mml::NodeKind::msub);
        expect_raw_family<mml::MmlMsup>(document, mml::NodeKind::msup);
        expect_raw_family<mml::MmlMsubsup>(document, mml::NodeKind::msubsup);
        expect_raw_family<mml::MmlMunder>(document, mml::NodeKind::munder);
        expect_raw_family<mml::MmlMover>(document, mml::NodeKind::mover);
        expect_raw_family<mml::MmlMunderover>(document, mml::NodeKind::munderover);
        expect_raw_family<mml::MmlMmultiscripts>(document, mml::NodeKind::mmultiscripts);
        expect_raw_family<mml::MmlMprescripts>(document, mml::NodeKind::mprescripts);
        expect_raw_family<mml::MmlNone>(document, mml::NodeKind::none);
        expect_raw_family<mml::MmlMtable>(document, mml::NodeKind::mtable);
        expect_raw_family<mml::MmlMlabeledtr>(document, mml::NodeKind::mlabeledtr);
        expect_raw_family<mml::MmlMtr>(document, mml::NodeKind::mtr);
        expect_raw_family<mml::MmlMtd>(document, mml::NodeKind::mtd);
        expect_raw_family<mml::MmlMaligngroup>(document, mml::NodeKind::maligngroup);
        expect_raw_family<mml::MmlMalignmark>(document, mml::NodeKind::malignmark);
        expect_raw_family<mml::MmlMglyph>(document, mml::NodeKind::mglyph);
        expect_raw_family<mml::MmlSemantics>(document, mml::NodeKind::semantics);
        expect_raw_family<mml::MmlAnnotation>(document, mml::NodeKind::annotation);
        expect_raw_family<mml::MmlAnnotationXML>(document, mml::NodeKind::annotation_xml);
        expect_raw_family<mml::MmlTeXAtom>(document, mml::NodeKind::tex_atom);
        expect_raw_family<mml::MathChoice>(document, mml::NodeKind::math_choice);
        expect_raw_family<mml::TextNode>(document, mml::NodeKind::text);
        expect_raw_family<mml::XMLNode>(document, mml::NodeKind::xml);
        expect_raw_family<mml::HtmlNode>(document, mml::NodeKind::html);
        expect(document.size() == mml::allNodeKinds.size());
    };

    "document_factory_is_the_single_owner_and_installs_factory_defaults"_test = [] {
        mml::MmlDocument document;
        auto &factory = document.factory();
        expect(&document.factory() == &factory);
        auto &raw = document.make(mml::NodeKind::mi);
        expect(raw.attributes().ownEntries().empty());
        expect(raw.attributes().lookup(u"mathvariant") == nullptr);
        auto &initialized = factory.make(mml::NodeKind::mi);
        expect(initialized.attributes().lookup(u"mathvariant") != nullptr);
        expect(std::get<std::u16string>(*initialized.attributes().lookup(u"mathvariant")) ==
               u"normal");
        expect(&document.factory() == &factory);
        expect(document.size() == 2u);
    };

    "source_virtual_copy_and_exact_cross_document_clone_remain_distinct"_test = [] {
        mml::MmlDocument source;
        auto &sourceNode = source.factory().make(mml::NodeKind::mi);
        sourceNode.attributes().set(u"id", std::u16string(u"source-id"));
        sourceNode.attributes().set(u"mathvariant", std::u16string(u"bold"));
        sourceNode.attributes().set(u"data-mark", std::u16string(u"source"));
        auto &sourceText = source.factory().make(mml::NodeKind::text);
        dynamic_cast<mml::TextNode &>(sourceText).setText(u"glyph text");
        sourceNode.appendChild(sourceText);
        sourceNode.setTexClass(mml::TexClass::rel);
        sourceNode.setPrevClass(mml::TexClass::op);
        sourceNode.setPrevLevel(2.0);
        sourceNode.properties().tex.explicit_tex_class = mml::TexClass::rel;

        auto &virtualCopy = sourceNode.copy();
        expect(dynamic_cast<mml::MmlMi *>(&virtualCopy) != nullptr);
        expect(&virtualCopy.document() == &source);
        expect(virtualCopy.children().size() == 1u);
        auto &copiedText = dynamic_cast<mml::TextNode &>(*virtualCopy.children().front());
        expect(copiedText.getText() == u"glyph text");
        expect(virtualCopy.state().working_tex_class == mml::TexClass::ord)
            << "MmlMi constructor class is restored; mutated source class is not copied";
        expect(!virtualCopy.state().previous_tex_class);
        expect(!virtualCopy.state().previous_script_level);
        expect(virtualCopy.properties() == sourceNode.properties());
        expect(virtualCopy.attributes().own(u"id") == nullptr);
        expect(std::get<std::u16string>(*virtualCopy.attributes().own(u"mathvariant")) == u"bold");
        expect(std::get<std::u16string>(*virtualCopy.attributes().own(u"data-mark")) == u"source");

        mml::MmlDocument cloneOwner;
        auto &exactClone = cloneOwner.cloneSubtree(sourceNode);
        expect(dynamic_cast<mml::MmlMi *>(&exactClone) != nullptr);
        expect(&exactClone.document() == &cloneOwner);
        expect(exactClone.properties() == sourceNode.properties());
        expect(exactClone.state() == sourceNode.state());
        expect(std::get<std::u16string>(*exactClone.attributes().own(u"id")) == u"source-id");
        expect(std::get<std::u16string>(*exactClone.attributes().own(u"mathvariant")) == u"bold");
        expect(std::get<std::u16string>(*exactClone.attributes().own(u"data-mark")) == u"source");
        expect(exactClone.children().size() == 1u);
        auto &exactText = dynamic_cast<mml::TextNode &>(*exactClone.children().front());
        expect(exactText.getText() == u"glyph text");

        exactClone.attributes().set(u"data-mark", std::u16string(u"clone-only"));
        exactClone.properties().tex.explicit_tex_class = mml::TexClass::bin;
        exactClone.setPrevLevel(7.0);
        exactText.setText(u"clone text");
        expect(std::get<std::u16string>(*sourceNode.attributes().own(u"data-mark")) == u"source");
        expect(sourceNode.properties().tex.explicit_tex_class == mml::TexClass::rel);
        expect(sourceNode.state().previous_script_level == 2.0);
        expect(dynamic_cast<mml::TextNode &>(*sourceNode.children().front()).getText() ==
               u"glyph text");
        expect(dynamic_cast<mml::TextNode &>(*virtualCopy.children().front()).getText() ==
               u"glyph text");
    };
};
