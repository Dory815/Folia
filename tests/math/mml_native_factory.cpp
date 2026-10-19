#include "support/folia_test.hpp"
import folia.math.input.tex.node_factory;
using namespace boost::ut;
namespace mml = folia::math::mml;
namespace tex = folia::math::tex;

suite math_native_mml_factory = [] {
    "typed_factory_owns_attributes_and_borrows_same_document_children"_test = [] {
        mml::MmlDocument document;
        mml::MmlFactory factory(document);
        auto& child = document.make(mml::NodeKind::mi);
        child.setText(u"x");

        mml::MmlNode* children[]{&child};
        mml::AttributeSet attributes;
        attributes.set(u"displaystyle", true);
        attributes.set(u"width", 2.5);
        auto& row = factory.make(mml::NodeKind::mrow, std::move(attributes), children);

        expect(row.kind() == mml::NodeKind::mrow);
        expect(row.children().size() == 1u);
        expect(row.children()[0] == &child);
        expect(child.parent() == &row);
        expect(std::get<bool>(*row.attributes().own(u"displaystyle")));
        expect(std::get<double>(*row.attributes().own(u"width")) == 2.5);
    };

    "negative_arity_inserts_one_inferred_row_for_supplied_children"_test = [] {
        mml::MmlDocument document;
        mml::MmlFactory factory(document);
        auto& text = document.make(mml::NodeKind::mtext);
        auto& explicitRow = document.make(mml::NodeKind::inferred_mrow);
        document.append(explicitRow, text);
        mml::MmlNode* supplied[]{&explicitRow};

        auto& error = factory.make(mml::NodeKind::merror, {}, supplied);
        expect(error.children().size() == 1u);
        auto* inferred = error.children()[0];
        expect(inferred->kind() == mml::NodeKind::inferred_mrow);
        expect(inferred != &explicitRow);
        expect(inferred->children().size() == 1u);
        expect(inferred->children()[0] == &text);
        expect(explicitRow.parent() == nullptr);
        expect(explicitRow.children().empty());
        expect(text.parent() == inferred);
    };

    "all_registered_kind_arities_match_pinned_source_classes"_test = [] {
        struct Expected final {
            mml::NodeKind kind;
            mml::NodeArityKind shape;
            std::uint8_t count;
        };
        const Expected expected[]{
            {mml::NodeKind::math, mml::NodeArityKind::inferred_row, 0},
            {mml::NodeKind::mi, mml::NodeArityKind::any, 0},
            {mml::NodeKind::mn, mml::NodeArityKind::any, 0},
            {mml::NodeKind::mo, mml::NodeArityKind::any, 0},
            {mml::NodeKind::mtext, mml::NodeArityKind::any, 0},
            {mml::NodeKind::mspace, mml::NodeArityKind::exact, 0},
            {mml::NodeKind::ms, mml::NodeArityKind::any, 0},
            {mml::NodeKind::mrow, mml::NodeArityKind::any, 0},
            {mml::NodeKind::inferred_mrow, mml::NodeArityKind::any, 0},
            {mml::NodeKind::mfrac, mml::NodeArityKind::exact, 2},
            {mml::NodeKind::msqrt, mml::NodeArityKind::inferred_row, 0},
            {mml::NodeKind::mroot, mml::NodeArityKind::exact, 2},
            {mml::NodeKind::mstyle, mml::NodeArityKind::inferred_row, 0},
            {mml::NodeKind::merror, mml::NodeArityKind::inferred_row, 0},
            {mml::NodeKind::mpadded, mml::NodeArityKind::inferred_row, 0},
            {mml::NodeKind::mphantom, mml::NodeArityKind::inferred_row, 0},
            {mml::NodeKind::mfenced, mml::NodeArityKind::any, 0},
            {mml::NodeKind::menclose, mml::NodeArityKind::inferred_row, 0},
            {mml::NodeKind::maction, mml::NodeArityKind::at_least_one, 1},
            {mml::NodeKind::msub, mml::NodeArityKind::exact, 2},
            {mml::NodeKind::msup, mml::NodeArityKind::exact, 2},
            {mml::NodeKind::msubsup, mml::NodeArityKind::exact, 3},
            {mml::NodeKind::munder, mml::NodeArityKind::exact, 2},
            {mml::NodeKind::mover, mml::NodeArityKind::exact, 2},
            {mml::NodeKind::munderover, mml::NodeArityKind::exact, 3},
            {mml::NodeKind::mmultiscripts, mml::NodeArityKind::at_least_one, 1},
            {mml::NodeKind::mprescripts, mml::NodeArityKind::exact, 0},
            {mml::NodeKind::none, mml::NodeArityKind::exact, 0},
            {mml::NodeKind::mtable, mml::NodeArityKind::any, 0},
            {mml::NodeKind::mlabeledtr, mml::NodeArityKind::at_least_one, 1},
            {mml::NodeKind::mtr, mml::NodeArityKind::any, 0},
            {mml::NodeKind::mtd, mml::NodeArityKind::inferred_row, 0},
            {mml::NodeKind::maligngroup, mml::NodeArityKind::inferred_row, 0},
            {mml::NodeKind::malignmark, mml::NodeArityKind::exact, 0},
            {mml::NodeKind::mglyph, mml::NodeArityKind::any, 0},
            {mml::NodeKind::semantics, mml::NodeArityKind::at_least_one, 1},
            {mml::NodeKind::annotation, mml::NodeArityKind::any, 0},
            {mml::NodeKind::annotation_xml, mml::NodeArityKind::any, 0},
            {mml::NodeKind::tex_atom, mml::NodeArityKind::inferred_row, 0},
            {mml::NodeKind::math_choice, mml::NodeArityKind::exact, 4},
            {mml::NodeKind::text, mml::NodeArityKind::exact, 0},
            {mml::NodeKind::xml, mml::NodeArityKind::exact, 0},
            {mml::NodeKind::html, mml::NodeArityKind::exact, 0},
        };
        expect(sizeof(expected) / sizeof(expected[0]) == 43u);
        for (const auto& entry : expected) {
            expect(mml::nodeTraits(entry.kind).arityKind == entry.shape);
            expect(mml::nodeTraits(entry.kind).exactArity == entry.count);
            expect(mml::nodeKindFromName(mml::nodeKindName(entry.kind)).has_value());
            expect(*mml::nodeKindFromName(mml::nodeKindName(entry.kind)) == entry.kind);
        }
    };

    "infinite_arity_parent_flattens_inferred_child"_test = [] {
        mml::MmlDocument document;
        mml::MmlFactory factory(document);
        auto& text = document.make(mml::NodeKind::text);
        auto& inferred = document.make(mml::NodeKind::inferred_mrow);
        document.append(inferred, text);
        mml::MmlNode* children[]{&inferred};

        auto& row = factory.make(mml::NodeKind::mrow, {}, children);
        expect(row.children().size() == 1u);
        expect(row.children()[0] == &text);
        expect(text.parent() == &row);
        expect(inferred.children().empty());
        expect(inferred.parent() == nullptr);
    };

    "finite_arity_parent_converts_inferred_child_to_explicit_row"_test = [] {
        mml::MmlDocument document;
        mml::MmlFactory factory(document);
        auto& text = document.make(mml::NodeKind::text);
        auto& inferred = document.make(mml::NodeKind::inferred_mrow);
        inferred.attributes().set(u"mathvariant", std::u16string(u"bold"));
        document.append(inferred, text);
        auto& denominator = document.make(mml::NodeKind::mi);
        mml::MmlNode* children[]{&inferred, &denominator};

        auto& fraction = factory.make(mml::NodeKind::mfrac, {}, children);
        expect(fraction.children().size() == 2u);
        auto* explicitRow = fraction.children()[0];
        expect(explicitRow->kind() == mml::NodeKind::mrow);
        expect(explicitRow->children().size() == 1u);
        expect(explicitRow->children()[0] == &text);
        expect(explicitRow->parent() == &fraction);
        expect(text.parent() == explicitRow);
        expect(inferred.children().empty());
        expect(std::get<std::u16string>(*explicitRow->attributes().own(u"mathvariant")) == u"bold");
        expect(fraction.children()[1] == &denominator);
    };

    "invalid_factory_arguments_leave_existing_tree_unchanged"_test = [] {
        mml::MmlDocument first;
        mml::MmlDocument second;
        mml::MmlFactory factory(first);
        auto& parent = first.make(mml::NodeKind::mrow);
        auto& child = first.make(mml::NodeKind::mi);
        auto& grandchild = first.make(mml::NodeKind::text);
        auto& foreign = second.make(mml::NodeKind::mi);
        first.append(parent, child);
        first.append(child, grandchild);
        first.setRoot(parent);
        const auto sizeBefore = first.size();

        bool foreignRejected = false;
        bool nullRejected = false;
        bool duplicateRejected = false;
        bool overlapRejected = false;
        bool badKindRejected = false;
        mml::MmlNode* foreignInput[]{&foreign};
        mml::MmlNode* nullInput[]{nullptr};
        mml::MmlNode* duplicateInput[]{&child, &child};
        mml::MmlNode* overlapInput[]{&child, &grandchild};
        try { factory.make(mml::NodeKind::mrow, {}, foreignInput); }
        catch (...) { foreignRejected = true; }
        try { factory.make(mml::NodeKind::mrow, {}, nullInput); }
        catch (...) { nullRejected = true; }
        try { factory.make(mml::NodeKind::mrow, {}, duplicateInput); }
        catch (...) { duplicateRejected = true; }
        try { factory.make(mml::NodeKind::mrow, {}, overlapInput); }
        catch (...) { overlapRejected = true; }
        try { factory.make(static_cast<mml::NodeKind>(255)); }
        catch (...) { badKindRejected = true; }

        expect(foreignRejected);
        expect(nullRejected);
        expect(duplicateRejected);
        expect(overlapRejected);
        expect(badKindRejected);
        expect(first.size() == sizeBefore);
        expect(first.root() == &parent);
        expect(parent.children().size() == 1u);
        expect(parent.children()[0] == &child);
        expect(child.parent() == &parent);
        expect(child.children().size() == 1u);
        expect(child.children()[0] == &grandchild);
        expect(grandchild.parent() == &child);
        expect(foreign.parent() == nullptr);
    };
};

suite math_native_tex_node_factory = [] {
    "text_nodes_preserve_owned_utf16_and_token_uses_text_node_child"_test = [] {
        mml::MmlDocument document;
        mml::MmlFactory mmlFactory(document);
        tex::NodeFactory factory(mmlFactory);
        auto& token = factory.makeToken(mml::NodeKind::mi, u"𝛂é");
        expect(token.kind() == mml::NodeKind::mi);
        expect(token.text().empty());
        expect(token.children().size() == 1u);
        expect(token.children()[0]->kind() == mml::NodeKind::text);
        expect(token.children()[0]->text() == u"𝛂é");
    };

    "tex_error_uses_generic_merror_arity_rule_once"_test = [] {
        mml::MmlDocument document;
        mml::MmlFactory mmlFactory(document);
        tex::NodeFactory factory(mmlFactory);
        auto& error = factory.makeError(u"bad 𝛂");

        expect(error.kind() == mml::NodeKind::merror);
        expect(error.children().size() == 1u);
        expect(error.children()[0]->kind() == mml::NodeKind::inferred_mrow);
        auto* inferred = error.children()[0];
        expect(inferred->children().size() == 1u);
        auto* mtext = inferred->children()[0];
        expect(mtext->kind() == mml::NodeKind::mtext);
        expect(mtext->children().size() == 1u);
        expect(mtext->children()[0]->kind() == mml::NodeKind::text);
        expect(mtext->children()[0]->text() == u"bad 𝛂");
        const auto* metadata = error.attributes().own(u"data-mjx-error");
        expect(metadata != nullptr);
        expect(std::get<std::u16string>(*metadata) == u"bad 𝛂");
    };
};
