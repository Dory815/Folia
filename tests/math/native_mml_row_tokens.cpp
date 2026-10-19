#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mn;
import folia.math.core.mml_tree.mml_nodes.mtext;
import folia.math.core.mml_tree.mml_nodes.ms;
import folia.math.core.mml_tree.mml_nodes.mspace;
import folia.math.core.mml_tree.mml_nodes.mglyph;

namespace {
using namespace boost::ut;
namespace mml = folia::math::mml;

mml::MmlNode &makeToken(mml::MmlFactory &factory, mml::NodeKind kind, std::u16string_view value) {
    auto &text = factory.make(mml::NodeKind::text);
    text.setText(std::u16string(value));
    mml::MmlNode *children[]{&text};
    return factory.make(kind, {}, children);
}

suite native_mml_row_tokens = [] {
    "row_scans_content_and_preserves_inferred_identity"_test = [] {
        mml::MmlDocument document;
        auto &factory = document.factory();
        auto &row = dynamic_cast<mml::MmlMrow &>(factory.make(mml::NodeKind::mrow));
        auto &leading_space = dynamic_cast<mml::MmlMspace &>(factory.make(mml::NodeKind::mspace));
        auto &operator_token =
            dynamic_cast<mml::MmlMi &>(makeToken(factory, mml::NodeKind::mi, u"x"));
        auto &trailing_space = dynamic_cast<mml::MmlMspace &>(factory.make(mml::NodeKind::mspace));
        document.append(row, leading_space);
        document.append(row, operator_token);
        document.append(row, trailing_space);

        expect(!row.isEmbellished());
        expect(row.core() == &row);
        expect(row.coreMO() == &row);
        expect(row.nonSpaceLength() == 1u);
        expect(row.firstNonSpace() == &operator_token);
        expect(row.lastNonSpace() == &operator_token);
        expect(!row.isSpacelike());

        auto &inferred =
            dynamic_cast<mml::MmlInferredMrow &>(factory.make(mml::NodeKind::inferred_mrow));
        expect(inferred.isInferred());
        expect(inferred.notParent());
        expect(inferred.kind() == mml::NodeKind::inferred_mrow);
    };

    "mi_keeps_unicode_single_character_and_auto_operator_rules"_test = [] {
        expect(mml::MmlMi::singleCharacter(u"x"));
        expect(mml::MmlMi::singleCharacter(u"x\u0301"));
        expect(mml::MmlMi::singleCharacter(u"\U0001D465"));
        expect(!mml::MmlMi::singleCharacter(u"xy"));
        expect(!mml::MmlMi::singleCharacter(u"x\n"));
        expect(!mml::MmlMi::singleCharacter(u"x\r\n"));
        expect(mml::MmlMi::operatorName(u"sin2"));
        expect(!mml::MmlMi::operatorName(u"sin_2"));
        expect(!mml::MmlMi::operatorName(u"sin\n"));
        expect(!mml::MmlMi::operatorName(u"sin\r\n"));

        mml::MmlDocument document;
        auto &factory = document.factory();
        auto &token = dynamic_cast<mml::MmlMi &>(makeToken(factory, mml::NodeKind::mi, u"sin"));
        auto &symbol = dynamic_cast<mml::MmlMi &>(makeToken(factory, mml::NodeKind::mi, u"x"));
        symbol.setInheritedAttributes();
        expect(std::get<std::u16string>(*symbol.attributes().lookup(u"mathvariant")) == u"italic");
        auto &multi = dynamic_cast<mml::MmlMi &>(makeToken(factory, mml::NodeKind::mi, u"xy"));
        multi.setInheritedAttributes();
        expect(std::get<std::u16string>(*multi.attributes().lookup(u"mathvariant")) == u"normal");
        auto &inherited_false =
            dynamic_cast<mml::MmlMi &>(makeToken(factory, mml::NodeKind::mi, u"x"));
        const mml::AttributeList false_variant{{u"mathvariant", mml::NodeKind::math, false}};
        inherited_false.setInheritedAttributes(false_variant);
        expect(std::get<bool>(*inherited_false.attributes().lookup(u"mathvariant")) == false);
        token.attributes().set(u"mathvariant", std::u16string(u"normal"));
        expect(token.setTeXclass(nullptr) == &token);
        expect(token.state().working_tex_class == mml::TexClass::op);
        expect(token.properties().tex.auto_operator == true);

        auto &false_auto_operator =
            dynamic_cast<mml::MmlMi &>(makeToken(factory, mml::NodeKind::mi, u"sin"));
        false_auto_operator.attributes().set(u"mathvariant", std::u16string(u"normal"));
        false_auto_operator.properties().tex.auto_operator = false;
        (void)false_auto_operator.setTeXclass(nullptr);
        expect(false_auto_operator.state().working_tex_class != mml::TexClass::op);

        auto &true_auto_operator =
            dynamic_cast<mml::MmlMi &>(makeToken(factory, mml::NodeKind::mi, u"sin"));
        true_auto_operator.attributes().set(u"mathvariant", std::u16string(u"normal"));
        true_auto_operator.properties().tex.auto_operator = true;
        (void)true_auto_operator.setTeXclass(nullptr);
        expect(true_auto_operator.state().working_tex_class != mml::TexClass::op);

        auto &explicit_class =
            dynamic_cast<mml::MmlMi &>(makeToken(factory, mml::NodeKind::mi, u"sin"));
        explicit_class.attributes().set(u"mathvariant", std::u16string(u"normal"));
        explicit_class.properties().tex.explicit_tex_class = mml::TexClass::ord;
        (void)explicit_class.setTeXclass(nullptr);
        expect(explicit_class.state().working_tex_class != mml::TexClass::op);
    };

    "mtext_and_mspace_keep_their_spacing_and_break_exceptions"_test = [] {
        mml::MmlDocument document;
        auto &factory = document.factory();
        auto &text =
            dynamic_cast<mml::MmlMtext &>(makeToken(factory, mml::NodeKind::mtext, u"\u2003\t"));
        expect(text.isSpacelike());
        text.attributes().set(u"style", u"color:red");
        expect(!text.isSpacelike());

        auto &space = dynamic_cast<mml::MmlMspace &>(factory.make(mml::NodeKind::mspace));
        expect(space.arity() == 0.0);
        expect(space.canBreak());
        space.attributes().set(u"width", u" -0.2em");
        expect(!space.canBreak());
        space.attributes().set(u"width", u"0.2em");
        space.attributes().set(u"linebreak", u"newline");
        expect(space.hasNewline());
        expect(!space.isSpacelike());
    };

    "token_kinds_and_attribute_verification_are_family_owned"_test = [] {
        const auto spaceDefaults = mml::MmlMspace::defaults();
        const auto glyphDefaults = mml::MmlMglyph::defaults();
        expect(spaceDefaults.defaultValue(u"width") != nullptr);
        expect(std::get<std::u16string>(*spaceDefaults.defaultValue(u"width")) == u"0em");
        expect(glyphDefaults.defaultValue(u"src") != nullptr);
        expect(std::get<std::u16string>(*glyphDefaults.defaultValue(u"src")).empty());

        mml::MmlDocument document;
        auto &factory = document.factory();
        auto &mi = dynamic_cast<mml::MmlMi &>(factory.make(mml::NodeKind::mi));
        auto &mn = dynamic_cast<mml::MmlMn &>(factory.make(mml::NodeKind::mn));
        auto &mtext = dynamic_cast<mml::MmlMtext &>(factory.make(mml::NodeKind::mtext));
        auto &ms = dynamic_cast<mml::MmlMs &>(factory.make(mml::NodeKind::ms));
        auto &mspace = dynamic_cast<mml::MmlMspace &>(factory.make(mml::NodeKind::mspace));
        auto &mglyph = dynamic_cast<mml::MmlMglyph &>(factory.make(mml::NodeKind::mglyph));
        expect(mi.kind() == mml::NodeKind::mi);
        expect(mn.kind() == mml::NodeKind::mn);
        expect(mtext.kind() == mml::NodeKind::mtext);
        expect(ms.kind() == mml::NodeKind::ms);
        expect(mspace.kind() == mml::NodeKind::mspace);
        expect(mglyph.kind() == mml::NodeKind::mglyph);

        mglyph.attributes().set(u"src", u"icon.svg");
        mglyph.verifyTree();
    };
};
} // namespace
